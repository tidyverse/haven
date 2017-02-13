#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <math.h>
#include "readstat_sas.h"
#include "../readstat_iconv.h"
#include "../readstat_convert.h"

#define SAS_CATALOG_FIRST_INDEX_PAGE 1
#define SAS_CATALOG_USELESS_PAGES    3

typedef struct sas7bcat_ctx_s {
    readstat_metadata_handler      metadata_handler;
    readstat_value_label_handler   value_label_handler;
    void          *user_ctx;
    readstat_io_t *io;
    int            u64;
    int            pad1;
    int            bswap;
    int64_t        page_count;
    int64_t        page_size;
    int64_t        header_size;
    uint64_t      *block_pointers;
    int            block_pointers_used;
    int            block_pointers_capacity;
    const char    *input_encoding;
    const char    *output_encoding;
    iconv_t        converter;
} sas7bcat_ctx_t;

static void sas7bcat_ctx_free(sas7bcat_ctx_t *ctx) {
    if (ctx->converter)
        iconv_close(ctx->converter);
    if (ctx->block_pointers)
        free(ctx->block_pointers);

    free(ctx);
}

static readstat_error_t sas7bcat_parse_value_labels(const char *value_start, size_t value_labels_len, 
        int label_count_used, int label_count_capacity, const char *name, sas7bcat_ctx_t *ctx) {
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
                if (value.tag) {
                    value.is_tagged_missing = 1;
                } else {
                    value.is_system_missing = 1;
                }
            } else {
                memcpy(&dval, &val, 8);
                dval *= -1.0;
            }

            value.v.double_value = dval;
        }
        if (ctx->value_label_handler) {
            if (ctx->value_label_handler(name, value, label, ctx->user_ctx) != READSTAT_HANDLER_OK) {
                retval = READSTAT_ERROR_USER_ABORT;
                goto cleanup;
            }
        }

        lbp2 += 8 + 2 + label_len + 1;
    }

cleanup:
    if (value_offset)
        free(value_offset);

    return retval;
}

static readstat_error_t sas7bcat_parse_block(const char *data, size_t data_size, sas7bcat_ctx_t *ctx) {
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

    if ((retval = sas7bcat_parse_value_labels(&data[106+pad], data_size - 106 - pad, 
                    label_count_used, label_count_capacity, name, ctx)) != READSTAT_OK)
        goto cleanup;

cleanup:
    return retval;
}

static void sas7bcat_augment_index(const char *index, size_t len, sas7bcat_ctx_t *ctx) {
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

static int compare_block_pointers(const void *elem1, const void *elem2) {
    uint64_t v1 = *(const uint64_t *)elem1;
    uint64_t v2 = *(const uint64_t *)elem2;
    return v1 - v2;
}

static void sas7bcat_sort_index(sas7bcat_ctx_t *ctx) {
    if (ctx->block_pointers_used == 0)
        return;

    int i;
    for (i=1; i<ctx->block_pointers_used; i++) {
        if (ctx->block_pointers[i] < ctx->block_pointers[i-1]) {
            qsort(ctx->block_pointers, ctx->block_pointers_used, sizeof(uint64_t), &compare_block_pointers);
            break;
        }
    }
}

static void sas7bcat_uniq_index(sas7bcat_ctx_t *ctx) {
    if (ctx->block_pointers_used == 0)
        return;

    int i;
    int out_i = 1;
    for (i=1; i<ctx->block_pointers_used; i++) {
        if (ctx->block_pointers[i] != ctx->block_pointers[i-1]) {
            if (out_i != i) {
                ctx->block_pointers[out_i] = ctx->block_pointers[i];
            }
            out_i++;
        }
    }
    ctx->block_pointers_used = out_i;
}

static int sas7bcat_block_size(int start_page, int start_page_pos, sas7bcat_ctx_t *ctx, readstat_error_t *outError) {
    readstat_error_t retval = READSTAT_OK;
    readstat_io_t *io = ctx->io;
    int next_page = start_page;
    int next_page_pos = start_page_pos;

    int buffer_len = 0;
    int chain_link_len = 0;

    char chain_link[16];

    // calculate buffer size needed
    while (next_page > 0 && next_page_pos > 0 && next_page <= ctx->page_count) {
        if (io->seek(ctx->header_size+(next_page-1)*ctx->page_size+next_page_pos, READSTAT_SEEK_SET, io->io_ctx) == -1) {
            retval = READSTAT_ERROR_SEEK;
            goto cleanup;
        }
        if (io->read(chain_link, sizeof(chain_link), io->io_ctx) < sizeof(chain_link)) {
            retval = READSTAT_ERROR_READ;
            goto cleanup;
        }
        next_page = sas_read4(&chain_link[0], ctx->bswap);
        next_page_pos = sas_read2(&chain_link[4], ctx->bswap);
        chain_link_len = sas_read2(&chain_link[6], ctx->bswap);

        buffer_len += chain_link_len;
    }

cleanup:
    if (outError)
        *outError = retval;

    return retval == READSTAT_OK ? buffer_len : -1;
}

static readstat_error_t sas7bcat_read_block(char *buffer, size_t buffer_len,
        int start_page, int start_page_pos, sas7bcat_ctx_t *ctx) {
    readstat_error_t retval = READSTAT_OK;
    readstat_io_t *io = ctx->io;
    int next_page = start_page;
    int next_page_pos = start_page_pos;

    int chain_link_len = 0;
    int buffer_offset = 0;

    char chain_link[16];

    while (next_page > 0 && next_page_pos > 0) {
        if (io->seek(ctx->header_size+(next_page-1)*ctx->page_size+next_page_pos, READSTAT_SEEK_SET, io->io_ctx) == -1) {
            retval = READSTAT_ERROR_SEEK;
            goto cleanup;
        }
        if (io->read(chain_link, sizeof(chain_link), io->io_ctx) < sizeof(chain_link)) {
            retval = READSTAT_ERROR_READ;
            goto cleanup;
        }
        next_page = sas_read4(&chain_link[0], ctx->bswap);
        next_page_pos = sas_read2(&chain_link[4], ctx->bswap);
        chain_link_len = sas_read2(&chain_link[6], ctx->bswap);
        if (io->read(buffer + buffer_offset, chain_link_len, io->io_ctx) < chain_link_len) {
            retval = READSTAT_ERROR_READ;
            goto cleanup;
        }
        buffer_offset += chain_link_len;
    }
cleanup:

    return retval;
}

readstat_error_t readstat_parse_sas7bcat(readstat_parser_t *parser, const char *path, void *user_ctx) {
    readstat_error_t retval = READSTAT_OK;
    readstat_io_t *io = parser->io;
    int64_t i;
    char *page = NULL;
    char *buffer = NULL;

    sas7bcat_ctx_t *ctx = calloc(1, sizeof(sas7bcat_ctx_t));
    sas_header_info_t *hinfo = calloc(1, sizeof(sas_header_info_t));

    ctx->block_pointers = malloc((ctx->block_pointers_capacity = 200) * sizeof(uint64_t));

    ctx->value_label_handler = parser->value_label_handler;
    ctx->metadata_handler = parser->metadata_handler;
    ctx->input_encoding = parser->input_encoding;
    ctx->output_encoding = parser->output_encoding;
    ctx->user_ctx = user_ctx;
    ctx->io = io;

    if (io->open(path, io->io_ctx) == -1) {
        retval = READSTAT_ERROR_OPEN;
        goto cleanup;
    }

    if ((retval = sas_read_header(io, hinfo, parser->error_handler, user_ctx)) != READSTAT_OK) {
        goto cleanup;
    }

    ctx->u64 = hinfo->u64;
    ctx->pad1 = hinfo->pad1;
    ctx->bswap = machine_is_little_endian() ^ hinfo->little_endian;
    ctx->header_size = hinfo->header_size;
    ctx->page_count = hinfo->page_count;
    ctx->page_size = hinfo->page_size;
    if (ctx->input_encoding == NULL) {
        ctx->input_encoding = hinfo->encoding;
    }

    if (ctx->u64) {
        retval = READSTAT_ERROR_PARSE;
        goto cleanup;
    }

    if (ctx->input_encoding && ctx->output_encoding && strcmp(ctx->input_encoding, ctx->output_encoding) != 0) {
        iconv_t converter = iconv_open(ctx->output_encoding, ctx->input_encoding);
        if (converter == (iconv_t)-1) {
            retval = READSTAT_ERROR_UNSUPPORTED_CHARSET;
            goto cleanup;
        }
        ctx->converter = converter;
    }

    if (parser->metadata_handler) {
        char file_label[4*64+1];
        retval = readstat_convert(file_label, sizeof(file_label), 
                hinfo->file_label, sizeof(hinfo->file_label), ctx->converter);
        if (retval != READSTAT_OK)
            goto cleanup;

        if (ctx->metadata_handler(file_label, hinfo->modification_time, 
                    10000 * hinfo->major_version + hinfo->minor_version, ctx->user_ctx) != READSTAT_HANDLER_OK) {
            retval = READSTAT_ERROR_USER_ABORT;
            goto cleanup;
        }
    }

    if ((page = malloc(ctx->page_size)) == NULL) {
        retval = READSTAT_ERROR_MALLOC;
        goto cleanup;
    }
    if (io->seek(ctx->header_size+SAS_CATALOG_FIRST_INDEX_PAGE*ctx->page_size, READSTAT_SEEK_SET, io->io_ctx) == -1) {
        retval = READSTAT_ERROR_SEEK;
        goto cleanup;
    }
    if (io->read(page, ctx->page_size, io->io_ctx) < ctx->page_size) {
        retval = READSTAT_ERROR_READ;
        goto cleanup;
    }

    sas7bcat_augment_index(&page[856+2*ctx->pad1], ctx->page_size - 856 - 2*ctx->pad1, ctx);

    // Pass 1 -- find the XLSR entries
    for (i=SAS_CATALOG_USELESS_PAGES; i<ctx->page_count; i++) {
        if (io->seek(ctx->header_size+i*ctx->page_size, READSTAT_SEEK_SET, io->io_ctx) == -1) {
            retval = READSTAT_ERROR_SEEK;
            goto cleanup;
        }
        if (io->read(page, ctx->page_size, io->io_ctx) < ctx->page_size) {
            retval = READSTAT_ERROR_READ;
            goto cleanup;
        }
        if (memcmp(&page[16], "XLSR", sizeof("XLSR")-1) == 0) {
            sas7bcat_augment_index(&page[16], ctx->page_size - 16, ctx);
        }
    }

    sas7bcat_sort_index(ctx);
    sas7bcat_uniq_index(ctx);

    // Pass 2 -- look up the individual block pointers
    for (i=0; i<ctx->block_pointers_used; i++) {
        int start_page = ctx->block_pointers[i] >> 32;
        int start_page_pos = (ctx->block_pointers[i]) & 0xFFFF;

        int buffer_len = sas7bcat_block_size(start_page, start_page_pos, ctx, &retval);
        if (buffer_len == -1) {
            goto cleanup;
        } else if (buffer_len == 0) {
            continue;
        }
        buffer = realloc(buffer, buffer_len);
        if ((retval = sas7bcat_read_block(buffer, buffer_len, start_page, start_page_pos, ctx)) != READSTAT_OK)
            goto cleanup;
        if ((retval = sas7bcat_parse_block(buffer, buffer_len, ctx)) != READSTAT_OK)
            goto cleanup;
    }

cleanup:
    io->close(io->io_ctx);
    if (page)
        free(page);
    if (buffer)
        free(buffer);
    if (ctx)
        sas7bcat_ctx_free(ctx);
    if (hinfo)
        free(hinfo);

    return retval;
}
