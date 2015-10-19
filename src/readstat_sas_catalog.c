#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include "readstat_sas.h"
#include "readstat_iconv.h"
#include "readstat_convert.h"
#include "readstat_io.h"

#define SAS_CATALOG_FIRST_INDEX_PAGE 1
#define SAS_CATALOG_USELESS_PAGES    3

typedef struct sas_catalog_ctx_s {
    readstat_value_label_handler   value_label_handler;
    void         *user_ctx;
    int           u64;
    int           pad1;
    int           bswap;
    int           fd;
    int64_t       page_count;
    int64_t       page_size;
    int64_t       header_size;
    uint64_t     *block_pointers;
    int           block_pointers_used;
    int           block_pointers_capacity;
    iconv_t       converter;
} sas_catalog_ctx_t;

static void sas_catalog_ctx_free(sas_catalog_ctx_t *ctx) {
    if (ctx->converter)
        iconv_close(ctx->converter);
    if (ctx->fd != -1)
        readstat_close(ctx->fd);
    if (ctx->block_pointers)
        free(ctx->block_pointers);

    free(ctx);
}

static readstat_error_t sas_parse_value_labels(const char *value_start, size_t value_labels_len, 
        int label_count_used, int label_count_capacity, const char *name, sas_catalog_ctx_t *ctx) {
    readstat_error_t retval = READSTAT_OK;
    int i;
    const char *lbp1 = value_start;
    uint32_t *value_offset = calloc(label_count_used, sizeof(uint32_t));
    /* Doubles appear to be stored as big-endian, always */
    int bswap_doubles = machine_is_little_endian();
    int is_string = (name[0] == '$');

    /* Pass 1 -- find out the offset of the labels */
    for (i=0; i<label_count_capacity; i++) {
        if (&lbp1[2] - value_start > value_labels_len) {
            retval = READSTAT_ERROR_PARSE;
            goto cleanup;
        }
        if (i<label_count_used) {
            uint32_t label_pos = sas_read4(&lbp1[10+ctx->pad1], ctx->bswap);
            if (label_pos >= label_count_used) {
                retval = READSTAT_ERROR_PARSE;
                goto cleanup;
            }
            value_offset[label_pos] = lbp1 - value_start;
        }
        lbp1 += 6 + lbp1[2];
    }

    const char *lbp2 = lbp1;

    /* Pass 2 -- parse pairs of values & labels */
    for (i=0; i<label_count_used && i<label_count_capacity; i++) {
        lbp1 = value_start + value_offset[i];

        if (&lbp1[30] - value_start > value_labels_len ||
                &lbp2[10] - value_start > value_labels_len) {
            retval = READSTAT_ERROR_PARSE;
            goto cleanup;
        }
        size_t label_len = sas_read2(&lbp2[8], ctx->bswap);
        size_t value_entry_len = 6 + lbp1[2];
        const char *label = &lbp2[10];
        readstat_value_t value = { .type = is_string ? READSTAT_TYPE_STRING : READSTAT_TYPE_DOUBLE };
        if (is_string) {
            char val[4*16+1];
            retval = readstat_convert(val, sizeof(val), &lbp1[value_entry_len-16], 16, ctx->converter);
            if (retval != READSTAT_OK)
                goto cleanup;

            value.v.string_value = val;
        } else {
            uint64_t val = sas_read8(&lbp1[22], bswap_doubles);
            double dval = NAN;
            if ((val | 0xFF0000000000) == 0xFFFFFFFFFFFF) {
                value.tag = (val >> 40);
                value.is_system_missing = 1;
            } else {
                memcpy(&dval, &val, 8);
                dval *= -1.0;
            }

            value.v.double_value = dval;
        }
        if (ctx->value_label_handler) {
            ctx->value_label_handler(name, value, label, ctx->user_ctx);
        }

        lbp2 += 8 + 2 + label_len + 1;
    }

cleanup:
    if (value_offset)
        free(value_offset);

    return retval;
}

static readstat_error_t sas_catalog_parse_block(const char *data, size_t data_size, sas_catalog_ctx_t *ctx) {
    readstat_error_t retval = READSTAT_OK;

    size_t pad = (data[2] & 0x08) ? 4 : 0; // might be 0x10, not sure

    int label_count_capacity = sas_read4(&data[38+pad], ctx->bswap);
    int label_count_used = sas_read4(&data[42+pad], ctx->bswap);
    char name[4*32+1];

    if ((retval = readstat_convert(name, sizeof(name), &data[8], 8, ctx->converter)) != READSTAT_OK)
        goto cleanup;

    if (pad) {
        pad += 16;
    }

    if ((data[2] & 0x80)) { // has long name
        retval = readstat_convert(name, sizeof(name), &data[106+pad], 32, ctx->converter);
        if (retval != READSTAT_OK)
            goto cleanup;
        pad += 32;
    }

    if ((retval = sas_parse_value_labels(&data[106+pad], data_size - 106 - pad, 
                    label_count_used, label_count_capacity, name, ctx)) != READSTAT_OK)
        goto cleanup;

cleanup:
    return retval;
}

static void sas_catalog_augment_index(const char *index, size_t len, sas_catalog_ctx_t *ctx) {
    const char *xlsr = index;
    while (xlsr + 212 <= index + len) {
        if (memcmp(xlsr, "XLSR", 4) != 0) // some block pointers seem to have 8 bytes of extra padding
            xlsr += 8;
        if (memcmp(xlsr, "XLSR", 4) != 0)
            break;

        if (xlsr[50+ctx->pad1] == 'O')
            ctx->block_pointers[ctx->block_pointers_used++] = ((uint64_t)sas_read2(&xlsr[4], ctx->bswap) << 32) + sas_read2(&xlsr[8], ctx->bswap);

        if (ctx->block_pointers_used == ctx->block_pointers_capacity) {
            ctx->block_pointers = realloc(ctx->block_pointers, (ctx->block_pointers_capacity *= 2) * sizeof(uint64_t));
        }

        xlsr += 212 + ctx->pad1;
    }
}

static int sas_catalog_block_size(int start_page, int start_page_pos, sas_catalog_ctx_t *ctx, readstat_error_t *outError) {
    readstat_error_t retval = READSTAT_OK;
    int next_page = start_page;
    int next_page_pos = start_page_pos;

    int buffer_len = 0;
    int block_len = 0;

    char *page = malloc(16);

    // calculate buffer size needed
    while (next_page > 0 && next_page_pos > 0) {
        if (readstat_lseek(ctx->fd, ctx->header_size+(next_page-1)*ctx->page_size+next_page_pos, SEEK_SET) == -1) {
            retval = READSTAT_ERROR_SEEK;
            goto cleanup;
        }
        if (read(ctx->fd, page, 16) < 16) {
            retval = READSTAT_ERROR_READ;
            goto cleanup;
        }
        next_page = sas_read4(&page[0], ctx->bswap);
        next_page_pos = sas_read2(&page[4], ctx->bswap);
        block_len = sas_read2(&page[6], ctx->bswap);

        buffer_len += block_len;
    }

cleanup:
    if (outError)
        *outError = retval;
    if (page)
        free(page);

    return retval == READSTAT_OK ? buffer_len : -1;
}

static readstat_error_t sas_catalog_read_block(char *buffer, size_t buffer_len, int start_page, int start_page_pos, sas_catalog_ctx_t *ctx) {
    readstat_error_t retval = READSTAT_OK;
    int next_page = start_page;
    int next_page_pos = start_page_pos;

    int block_len = 0;
    int buffer_offset = 0;

    char *page = malloc(16);

    while (next_page > 0 && next_page_pos > 0) {
        if (readstat_lseek(ctx->fd, ctx->header_size+(next_page-1)*ctx->page_size+next_page_pos, SEEK_SET) == -1) {
            retval = READSTAT_ERROR_SEEK;
            goto cleanup;
        }
        if (read(ctx->fd, page, 16) < 16) {
            retval = READSTAT_ERROR_READ;
            goto cleanup;
        }
        next_page = sas_read4(&page[0], ctx->bswap);
        next_page_pos = sas_read2(&page[4], ctx->bswap);
        block_len = sas_read2(&page[6], ctx->bswap);
        if (read(ctx->fd, buffer + buffer_offset, block_len) < block_len) {
            retval = READSTAT_ERROR_READ;
            goto cleanup;
        }
        buffer_offset += block_len;
    }
cleanup:
    if (page)
        free(page);

    return retval;
}

readstat_error_t readstat_parse_sas7bcat(readstat_parser_t *parser, const char *filename, void *user_ctx) {
    readstat_error_t retval = READSTAT_OK;
    int64_t i;
    char *page = NULL;
    char *buffer = NULL;

    sas_catalog_ctx_t *ctx = calloc(1, sizeof(sas_catalog_ctx_t));
    sas_header_info_t *hinfo = calloc(1, sizeof(sas_header_info_t));

    ctx->block_pointers = malloc((ctx->block_pointers_capacity = 200) * sizeof(uint64_t));

    ctx->value_label_handler = parser->value_label_handler;
    ctx->user_ctx = user_ctx;

    if ((ctx->fd = readstat_open(filename)) == -1) {
        retval = READSTAT_ERROR_OPEN;
        goto cleanup;
    }

    if ((retval = sas_read_header(ctx->fd, hinfo, parser->error_handler, user_ctx)) != READSTAT_OK) {
        goto cleanup;
    }

    ctx->u64 = hinfo->u64;
    ctx->pad1 = hinfo->pad1;
    ctx->bswap = machine_is_little_endian() ^ hinfo->little_endian;
    ctx->header_size = hinfo->header_size;
    ctx->page_count = hinfo->page_count;
    ctx->page_size = hinfo->page_size;

    if (ctx->u64) {
        retval = READSTAT_ERROR_PARSE;
        goto cleanup;
    }

    if (strcmp(hinfo->encoding, "UTF-8") != 0 &&
            strcmp(hinfo->encoding, "US-ASCII") != 0) {
        iconv_t converter = iconv_open("UTF-8", hinfo->encoding);
        if (converter == (iconv_t)-1) {
            retval = READSTAT_ERROR_UNSUPPORTED_CHARSET;
            goto cleanup;
        }
        ctx->converter = converter;
    }

    if ((page = malloc(ctx->page_size)) == NULL) {
        retval = READSTAT_ERROR_MALLOC;
        goto cleanup;
    }
    if (readstat_lseek(ctx->fd, ctx->header_size+SAS_CATALOG_FIRST_INDEX_PAGE*ctx->page_size, SEEK_SET) == -1) {
        retval = READSTAT_ERROR_SEEK;
        goto cleanup;
    }
    if (read(ctx->fd, page, ctx->page_size) < ctx->page_size) {
        retval = READSTAT_ERROR_READ;
        goto cleanup;
    }

    sas_catalog_augment_index(&page[856+2*ctx->pad1], ctx->page_size - 856 - 2*ctx->pad1, ctx);

    // Pass 1 -- find the XLSR entries
    for (i=SAS_CATALOG_USELESS_PAGES; i<ctx->page_count; i++) {
        if (readstat_lseek(ctx->fd, ctx->header_size+i*ctx->page_size, SEEK_SET) == -1) {
            retval = READSTAT_ERROR_SEEK;
            goto cleanup;
        }
        if (read(ctx->fd, page, ctx->page_size) < ctx->page_size) {
            retval = READSTAT_ERROR_READ;
            goto cleanup;
        }
        if (memcmp(&page[16], "XLSR", sizeof("XLSR")-1) == 0) {
            sas_catalog_augment_index(&page[16], ctx->page_size - 16, ctx);
        }
    }

    // Pass 2 -- look up the individual block pointers
    for (i=0; i<ctx->block_pointers_used; i++) {
        int start_page = ctx->block_pointers[i] >> 32;
        int start_page_pos = (ctx->block_pointers[i]) & 0xFFFF;

        int buffer_len = sas_catalog_block_size(start_page, start_page_pos, ctx, &retval);
        if (buffer_len == -1) {
            goto cleanup;
        } else if (buffer_len == 0) {
            continue;
        }
        buffer = realloc(buffer, buffer_len);
        if ((retval = sas_catalog_read_block(buffer, buffer_len, start_page, start_page_pos, ctx)) != READSTAT_OK)
            goto cleanup;
        if ((retval = sas_catalog_parse_block(buffer, buffer_len, ctx)) != READSTAT_OK)
            goto cleanup;
    }

cleanup:
    if (page)
        free(page);
    if (buffer)
        free(buffer);
    if (ctx)
        sas_catalog_ctx_free(ctx);
    if (hinfo)
        free(hinfo);

    return retval;
}
