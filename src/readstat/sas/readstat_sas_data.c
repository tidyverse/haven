
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <math.h>
#include <inttypes.h>
#include "readstat_sas.h"
#include "readstat_sas_rle.h"
#include "../readstat_iconv.h"
#include "../readstat_convert.h"

#define ERROR_BUF_SIZE 1024

#define SAS_COMPRESSION_SIGNATURE_RLE  "SASYZCRL"
#define SAS_COMPRESSION_SIGNATURE_RDC  "SASYZCR2"

typedef struct col_info_s {
    sas_text_ref_t  name_ref;
    sas_text_ref_t  format_ref;
    sas_text_ref_t  label_ref;

    int    index;
    int    offset;
    int    width;
    int    type;
} col_info_t;

typedef struct sas_ctx_s {
    readstat_info_handler       info_handler;
    readstat_metadata_handler   metadata_handler;
    readstat_variable_handler   variable_handler;
    readstat_value_handler      value_handler;
    readstat_error_handler      error_handler;
    readstat_progress_handler   progress_handler;
    int64_t                     file_size;

    int            little_endian;
    int            u64;
    int            vendor;
    void          *user_ctx;
    readstat_io_t *io;
    int            bswap;
    int            did_submit_columns;

    int32_t        row_length;
    int32_t        page_row_count;
    int32_t        parsed_row_count;
    int32_t        column_count;
    int32_t        row_limit;

    int64_t        header_size;
    int64_t        page_count;
    int64_t        page_size;
    int64_t        page_header_size;
    int64_t        subheader_pointer_size;

    int            text_blob_count;
    size_t        *text_blob_lengths;
    char         **text_blobs;

    int            col_names_count;
    int            col_attrs_count;
    int            col_formats_count;

    int            max_col_width;
    char          *scratch_buffer;
    size_t         scratch_buffer_len;

    int            col_info_count;
    col_info_t    *col_info;

    const char    *input_encoding;
    const char    *output_encoding;
    iconv_t        converter;

    time_t         timestamp;
    int            version;
    char           file_label[4*64+1];
} sas_ctx_t;

static void sas_ctx_free(sas_ctx_t *ctx) {
    int i;
    if (ctx->text_blobs) {
        for (i=0; i<ctx->text_blob_count; i++) {
            free(ctx->text_blobs[i]);
        }
        free(ctx->text_blobs);
        free(ctx->text_blob_lengths);
    }
    if (ctx->col_info)
        free(ctx->col_info);

    if (ctx->scratch_buffer)
        free(ctx->scratch_buffer);

    if (ctx->converter)
        iconv_close(ctx->converter);

    free(ctx);
}

static readstat_error_t sas_update_progress(sas_ctx_t *ctx) {
    readstat_io_t *io = ctx->io;
    return io->update(ctx->file_size, ctx->progress_handler, ctx->user_ctx, io->io_ctx);
}

static readstat_error_t sas_parse_column_text_subheader(const char *subheader, size_t len, sas_ctx_t *ctx) {
    readstat_error_t retval = READSTAT_OK;
    size_t signature_len = ctx->u64 ? 8 : 4;
    uint16_t remainder = sas_read2(&subheader[signature_len], ctx->bswap);
    char *blob = NULL;
    if (remainder != len - (4+2*signature_len)) {
        retval = READSTAT_ERROR_PARSE;
        goto cleanup;
    }
    ctx->text_blob_count++;
    ctx->text_blobs = realloc(ctx->text_blobs, ctx->text_blob_count * sizeof(char *));
    ctx->text_blob_lengths = realloc(ctx->text_blob_lengths,
            ctx->text_blob_count * sizeof(ctx->text_blob_lengths[0]));

    if ((blob = malloc(len-signature_len)) == NULL) {
        retval = READSTAT_ERROR_MALLOC;
        goto cleanup;
    }
    memcpy(blob, subheader+signature_len, len-signature_len);
    ctx->text_blob_lengths[ctx->text_blob_count-1] = len-signature_len;
    ctx->text_blobs[ctx->text_blob_count-1] = blob;

    /* another bit of a hack */
    if (len-signature_len > 12 + sizeof(SAS_COMPRESSION_SIGNATURE_RDC)-1 &&
            strncmp(blob + 12, SAS_COMPRESSION_SIGNATURE_RDC, sizeof(SAS_COMPRESSION_SIGNATURE_RDC)-1) == 0) {
        retval = READSTAT_ERROR_UNSUPPORTED_COMPRESSION;
        goto cleanup;
    }

cleanup:
    return retval;
}

static readstat_error_t sas_parse_column_size_subheader(const char *subheader, size_t len, sas_ctx_t *ctx) {
    readstat_error_t retval = READSTAT_OK;

    uint64_t col_count;

    if (ctx->u64) {
        col_count = sas_read8(&subheader[8], ctx->bswap);
    } else {
        col_count = sas_read4(&subheader[4], ctx->bswap);
    }

    ctx->column_count = col_count;

    return retval;
}

static readstat_error_t sas_parse_row_size_subheader(const char *subheader, size_t len, sas_ctx_t *ctx) {
    readstat_error_t retval = READSTAT_OK;
    uint64_t total_row_count;
    uint64_t row_length, page_row_count;

    if (ctx->u64) {
        row_length = sas_read8(&subheader[40], ctx->bswap);
        total_row_count = sas_read8(&subheader[48], ctx->bswap);
        page_row_count = sas_read8(&subheader[120], ctx->bswap);
    } else {
        row_length = sas_read4(&subheader[20], ctx->bswap);
        total_row_count = sas_read4(&subheader[24], ctx->bswap);
        page_row_count = sas_read4(&subheader[60], ctx->bswap);
    }

    ctx->row_length = row_length;
    ctx->page_row_count = page_row_count;
    if (ctx->row_limit == 0 || total_row_count < ctx->row_limit)
        ctx->row_limit = total_row_count;

    return retval;
}

static sas_text_ref_t sas_parse_text_ref(const char *data, sas_ctx_t *ctx) {
    sas_text_ref_t  ref;

    ref.index = sas_read2(&data[0], ctx->bswap);
    ref.offset = sas_read2(&data[2], ctx->bswap);
    ref.length = sas_read2(&data[4], ctx->bswap);

    return ref;
}

static readstat_error_t copy_text_ref(char *out_buffer, size_t out_buffer_len, sas_text_ref_t text_ref, sas_ctx_t *ctx) {
    if (text_ref.index < 0 || text_ref.index >= ctx->text_blob_count)
        return READSTAT_ERROR_PARSE;
    
    if (text_ref.length == 0) {
        out_buffer[0] = '\0';
        return READSTAT_OK;
    }

    char *blob = ctx->text_blobs[text_ref.index];

    if (text_ref.offset < 0 || text_ref.length < 0)
        return READSTAT_ERROR_PARSE;
    
    if (text_ref.offset + text_ref.length > ctx->text_blob_lengths[text_ref.index])
        return READSTAT_ERROR_PARSE;

    return readstat_convert(out_buffer, out_buffer_len, &blob[text_ref.offset], text_ref.length,
            ctx->converter);
}

static readstat_error_t sas_parse_column_name_subheader(const char *subheader, size_t len, sas_ctx_t *ctx) {
    readstat_error_t retval = READSTAT_OK;
    size_t signature_len = ctx->u64 ? 8 : 4;
    int cmax = ctx->u64 ? (len-28)/8 : (len-20)/8;
    int i;
    const char *cnp = &subheader[signature_len+8];
    uint16_t remainder = sas_read2(&subheader[signature_len], ctx->bswap);

    if (remainder != len - (4+2*signature_len)) {
        retval = READSTAT_ERROR_PARSE;
        goto cleanup;
    }

    ctx->col_names_count += cmax;
    if (ctx->col_info_count < ctx->col_names_count) {
        ctx->col_info_count = ctx->col_names_count;
        ctx->col_info = realloc(ctx->col_info, ctx->col_info_count * sizeof(col_info_t));
    }
    for (i=ctx->col_names_count-cmax; i<ctx->col_names_count; i++) {
        ctx->col_info[i].name_ref = sas_parse_text_ref(cnp, ctx);
        cnp += 8;
    }

cleanup:

    return retval;
}

static readstat_error_t sas_parse_column_attributes_subheader(const char *subheader, size_t len, sas_ctx_t *ctx) {
    readstat_error_t retval = READSTAT_OK;
    size_t signature_len = ctx->u64 ? 8 : 4;
    int cmax = ctx->u64 ? (len-28)/16 : (len-20)/12;
    int i;
    const char *cap = &subheader[signature_len+8];
    uint16_t remainder = sas_read2(&subheader[signature_len], ctx->bswap);

    if (remainder != len - (4+2*signature_len)) {
        retval = READSTAT_ERROR_PARSE;
        goto cleanup;
    }
    ctx->col_attrs_count += cmax;
    if (ctx->col_info_count < ctx->col_attrs_count) {
        ctx->col_info_count = ctx->col_attrs_count;
        ctx->col_info = realloc(ctx->col_info, ctx->col_info_count * sizeof(col_info_t));
    }
    for (i=ctx->col_attrs_count-cmax; i<ctx->col_attrs_count; i++) {
        if (ctx->u64) {
            ctx->col_info[i].offset = sas_read8(&cap[0], ctx->bswap);
        } else {
            ctx->col_info[i].offset = sas_read4(&cap[0], ctx->bswap);
        }

        off_t off=4;
        if (ctx->u64)
            off=8;

        ctx->col_info[i].width = sas_read4(&cap[off], ctx->bswap);
        if (ctx->col_info[i].width > ctx->max_col_width)
            ctx->max_col_width = ctx->col_info[i].width;

        if (cap[off+6] == SAS_COLUMN_TYPE_NUM) {
            ctx->col_info[i].type = READSTAT_TYPE_DOUBLE;
        } else if (cap[off+6] == SAS_COLUMN_TYPE_CHR) {
            ctx->col_info[i].type = READSTAT_TYPE_STRING;
        } else {
            retval = READSTAT_ERROR_PARSE;
            goto cleanup;
        }
        ctx->col_info[i].index = i;
        cap += off+8;
    }

cleanup:

    return retval;
}

static readstat_error_t sas_parse_column_format_subheader(const char *subheader, size_t len, sas_ctx_t *ctx) {
    readstat_error_t retval = READSTAT_OK;

    ctx->col_formats_count++;
    if (ctx->col_info_count < ctx->col_formats_count) {
        ctx->col_info_count = ctx->col_formats_count;
    }

    ctx->col_info[ctx->col_formats_count-1].format_ref = sas_parse_text_ref(
            ctx->u64 ? &subheader[46] : &subheader[34], ctx);
    ctx->col_info[ctx->col_formats_count-1].label_ref = sas_parse_text_ref(
            ctx->u64 ? &subheader[52] : &subheader[40], ctx);

    return retval;
}

static readstat_error_t handle_data_value(const char *col_data, col_info_t *col_info, sas_ctx_t *ctx) {
    readstat_error_t retval = READSTAT_OK;
    int cb_retval = 0;
    readstat_value_t value;
    memset(&value, 0, sizeof(readstat_value_t));

    value.type = col_info->type;

    if (col_info->type == READSTAT_TYPE_STRING) {
        retval = readstat_convert(ctx->scratch_buffer, ctx->scratch_buffer_len,
                col_data, col_info->width, ctx->converter);
        if (retval != READSTAT_OK)
            goto cleanup;

        value.v.string_value = ctx->scratch_buffer;
    } else if (col_info->type == READSTAT_TYPE_DOUBLE) {
        uint64_t  val = 0;
        double dval = NAN;
        if (ctx->little_endian) {
            int k;
            for (k=0; k<col_info->width; k++) {
                val = (val << 8) | (unsigned char)col_data[col_info->width-1-k];
            }
        } else {
            int k;
            for (k=0; k<col_info->width; k++) {
                val = (val << 8) | (unsigned char)col_data[k];
            }
        }
        val <<= (8-col_info->width)*8;

        memcpy(&dval, &val, 8);

        if (isnan(dval)) {
            value.v.double_value = NAN;
            value.tag = ~((val >> 40) & 0xFF);
            if (value.tag) {
                value.is_tagged_missing = 1;
            } else {
                value.is_system_missing = 1;
            }
        } else {
            value.v.double_value = dval;
        }
    }
    cb_retval = ctx->value_handler(ctx->parsed_row_count, col_info->index, 
            value, ctx->user_ctx);

    if (cb_retval)
        retval = READSTAT_ERROR_USER_ABORT;

cleanup:
    return retval;
}

static readstat_error_t sas_parse_single_row(const char *data, sas_ctx_t *ctx) {
    if (ctx->parsed_row_count == ctx->row_limit)
        return READSTAT_OK;

    readstat_error_t retval = READSTAT_OK;
    int j;
    if (ctx->value_handler) {
        ctx->scratch_buffer_len = 4*ctx->max_col_width+1;
        ctx->scratch_buffer = realloc(ctx->scratch_buffer, ctx->scratch_buffer_len);
        for (j=0; j<ctx->column_count; j++) {
            col_info_t *col_info = &ctx->col_info[j];
            retval = handle_data_value(&data[col_info->offset], col_info, ctx);
            if (retval != READSTAT_OK) {
                goto cleanup;
            }
        }
    }
    ctx->parsed_row_count++;

cleanup:
    return retval;
}

static readstat_error_t sas_parse_rows(const char *data, sas_ctx_t *ctx) {
    readstat_error_t retval = READSTAT_OK;
    int i;
    size_t row_offset=0;
    for (i=0; i<ctx->page_row_count && ctx->parsed_row_count < ctx->row_limit; i++) {
        if ((retval = sas_parse_single_row(&data[row_offset], ctx)) != READSTAT_OK)
            goto cleanup;

        row_offset += ctx->row_length;
    }

cleanup:
    return retval;
}

static readstat_error_t sas_parse_subheader_rle(const char *subheader, size_t len, sas_ctx_t *ctx) {
    if (ctx->row_limit == ctx->parsed_row_count)
        return READSTAT_OK;

    readstat_error_t retval = READSTAT_OK;
    char error_buf[ERROR_BUF_SIZE];
    char *buffer = malloc(ctx->row_length);
    if (buffer == NULL) {
        retval = READSTAT_ERROR_MALLOC;
        goto cleanup;
    }
    size_t bytes_decompressed = sas_rle_decompress(
            buffer, ctx->row_length, subheader, len);

    if (bytes_decompressed != ctx->row_length) {
        retval = READSTAT_ERROR_ROW_WIDTH_MISMATCH;
        if (ctx->error_handler) {
            snprintf(error_buf, sizeof(error_buf), 
                    "ReadStat: Row #%d decompressed to %ld bytes (expected %d bytes)\n",
                    ctx->parsed_row_count, (long)(bytes_decompressed), ctx->row_length);
            ctx->error_handler(error_buf, ctx->user_ctx);
        }
        goto cleanup;
    }
    retval = sas_parse_single_row(buffer, ctx);
cleanup:
    if (buffer)
        free(buffer);

    return retval;
}

static readstat_error_t sas_parse_subheader(uint32_t signature, const char *subheader, size_t len, sas_ctx_t *ctx) {
    readstat_error_t retval = READSTAT_OK;

    if (len < 6) {
        retval = READSTAT_ERROR_PARSE;
        goto cleanup;
    }
    if (signature == SAS_SUBHEADER_SIGNATURE_ROW_SIZE) {
        retval = sas_parse_row_size_subheader(subheader, len, ctx);
    } else if (signature == SAS_SUBHEADER_SIGNATURE_COLUMN_SIZE) {
        retval = sas_parse_column_size_subheader(subheader, len, ctx);
    } else if (signature == SAS_SUBHEADER_SIGNATURE_COUNTS) {
        /* void */
    } else if (signature == SAS_SUBHEADER_SIGNATURE_COLUMN_TEXT) {
        retval = sas_parse_column_text_subheader(subheader, len, ctx);
    } else if (signature == SAS_SUBHEADER_SIGNATURE_COLUMN_NAME) {
        retval = sas_parse_column_name_subheader(subheader, len, ctx);
    } else if (signature == SAS_SUBHEADER_SIGNATURE_COLUMN_ATTRS) {
        retval = sas_parse_column_attributes_subheader(subheader, len, ctx);
    } else if (signature == SAS_SUBHEADER_SIGNATURE_COLUMN_FORMAT) {
        retval = sas_parse_column_format_subheader(subheader, len, ctx);
    } else if (signature == SAS_SUBHEADER_SIGNATURE_COLUMN_LIST) {
        /* void */
    } else if (signature == SAS_SUBHEADER_SIGNATURE_COLUMN_UNKNOWN) {
        /* void */
    } else {
        retval = READSTAT_ERROR_PARSE;
    }

cleanup:

    return retval;
}

static readstat_variable_t *sas_init_variable(sas_ctx_t *ctx, int i, readstat_error_t *out_retval) {
    readstat_error_t retval = READSTAT_OK;
    readstat_variable_t *variable = calloc(1, sizeof(readstat_variable_t));

    variable->index = i;
    variable->type = ctx->col_info[i].type;
    variable->storage_width = ctx->col_info[i].width;

    if ((retval = copy_text_ref(variable->name, sizeof(variable->name), 
                    ctx->col_info[i].name_ref, ctx)) != READSTAT_OK) {
        goto cleanup;
    }
    if ((retval = copy_text_ref(variable->format, sizeof(variable->format), 
                    ctx->col_info[i].format_ref, ctx)) != READSTAT_OK) {
        goto cleanup;
    }
    if ((retval = copy_text_ref(variable->label, sizeof(variable->label), 
                    ctx->col_info[i].label_ref, ctx)) != READSTAT_OK) {
        goto cleanup;
    }

cleanup:
    if (retval != READSTAT_OK) {
        free(variable);
        if (out_retval)
            *out_retval = retval;

        return NULL;
    }

    return variable;
}

static readstat_error_t submit_columns(sas_ctx_t *ctx) {
    readstat_error_t retval = READSTAT_OK;
    if (ctx->info_handler) {
        if (ctx->info_handler(ctx->row_limit, ctx->column_count, ctx->user_ctx)) {
            retval = READSTAT_ERROR_USER_ABORT;
            goto cleanup;
        }
    }
    if (ctx->metadata_handler) {
        if (ctx->metadata_handler(ctx->file_label, ctx->timestamp, ctx->version, ctx->user_ctx)) {
            retval = READSTAT_ERROR_USER_ABORT;
            goto cleanup;
        }
    }
    if (ctx->variable_handler) {
        int i;
        for (i=0; i<ctx->column_count; i++) {
            readstat_variable_t *variable = sas_init_variable(ctx, i, &retval);
            if (variable == NULL)
                break;

            int cb_retval = ctx->variable_handler(i, variable, variable->format, ctx->user_ctx);
            free(variable);
            if (cb_retval) {
                retval = READSTAT_ERROR_USER_ABORT;
                goto cleanup;
            }
        }
    }
cleanup:
    return retval;
}

static readstat_error_t submit_columns_if_needed(sas_ctx_t *ctx) {
    readstat_error_t retval = READSTAT_OK;
    if (!ctx->did_submit_columns) {
        if ((retval = submit_columns(ctx)) != READSTAT_OK) {
            goto cleanup;
        }
        ctx->did_submit_columns = 1;
    }
cleanup:
    return retval;
}

static int sas_signature_is_recognized(uint32_t signature) {
    return (signature == SAS_SUBHEADER_SIGNATURE_ROW_SIZE ||
            signature == SAS_SUBHEADER_SIGNATURE_COLUMN_SIZE ||
            signature == SAS_SUBHEADER_SIGNATURE_COUNTS ||
            signature == SAS_SUBHEADER_SIGNATURE_COLUMN_FORMAT ||
            (signature >= SAS_SUBHEADER_SIGNATURE_COLUMN_UNKNOWN &&
             signature <= SAS_SUBHEADER_SIGNATURE_COLUMN_NAME));
}

/* First, extract column text */
static readstat_error_t sas_parse_page_pass1(const char *page, size_t page_size, sas_ctx_t *ctx) {
    readstat_error_t retval = READSTAT_OK;

    uint16_t subheader_count = sas_read2(&page[ctx->page_header_size-4], ctx->bswap);

    int i;
    const char *shp = &page[ctx->page_header_size];
    for (i=0; i<subheader_count; i++) {
        uint64_t offset = 0, len = 0;
        uint32_t signature = 0;
        unsigned char compression = 0;
        int lshp = ctx->subheader_pointer_size;
        if (ctx->u64) {
            offset = sas_read8(&shp[0], ctx->bswap);
            len = sas_read8(&shp[8], ctx->bswap);
            compression = shp[16];
        } else {
            offset = sas_read4(&shp[0], ctx->bswap);
            len = sas_read4(&shp[4], ctx->bswap);
            compression = shp[8];
        }

        if (len > 0 && compression != SAS_COMPRESSION_TRUNC) {
            if (offset > page_size || offset + len > page_size ||
                    offset < ctx->page_header_size+subheader_count*lshp) {
                retval = READSTAT_ERROR_PARSE;
                goto cleanup;
            }
            if (compression == SAS_COMPRESSION_NONE) {
                signature = sas_read4(page + offset, ctx->bswap);
                if (!ctx->little_endian && signature == -1 && ctx->u64) {
                    signature = sas_read4(page + offset + 4, ctx->bswap);
                }
                if (signature == SAS_SUBHEADER_SIGNATURE_COLUMN_TEXT) {
                    if ((retval = sas_parse_subheader(signature, page + offset, len, ctx)) != READSTAT_OK) {
                        goto cleanup;
                    }
                }
            } else if (compression == SAS_COMPRESSION_ROW) {
                /* void */
            } else {
                retval = READSTAT_ERROR_UNSUPPORTED_COMPRESSION;
                goto cleanup;
            }
        }

        shp += lshp;
    }

cleanup:

    return retval;
}

static readstat_error_t sas_parse_page_pass2(const char *page, size_t page_size, sas_ctx_t *ctx) {
    uint16_t page_type;

    readstat_error_t retval = READSTAT_OK;

    page_type = sas_read2(&page[ctx->page_header_size-8], ctx->bswap);

    const char *data = NULL;

    if ((page_type & SAS_PAGE_TYPE_MASK) == SAS_PAGE_TYPE_DATA) {
        ctx->page_row_count = sas_read2(&page[ctx->page_header_size-6], ctx->bswap);
        data = &page[ctx->page_header_size];
    } else if (!(page_type & SAS_PAGE_TYPE_COMP)) {
        uint16_t subheader_count = sas_read2(&page[ctx->page_header_size-4], ctx->bswap);

        int i;
        const char *shp = &page[ctx->page_header_size];
        for (i=0; i<subheader_count; i++) {
            uint64_t offset = 0, len = 0;
            uint32_t signature = 0;
            unsigned char compression = 0;
            unsigned char is_compressed_data = 0;
            int lshp = ctx->subheader_pointer_size;
            if (ctx->u64) {
                offset = sas_read8(&shp[0], ctx->bswap);
                len = sas_read8(&shp[8], ctx->bswap);
                compression = shp[16];
                is_compressed_data = shp[17];
            } else {
                offset = sas_read4(&shp[0], ctx->bswap);
                len = sas_read4(&shp[4], ctx->bswap);
                compression = shp[8];
                is_compressed_data = shp[9];
            }

            if (len > 0 && compression != SAS_COMPRESSION_TRUNC) {
                if (offset > page_size || offset + len > page_size ||
                        offset < ctx->page_header_size+subheader_count*lshp) {
                    retval = READSTAT_ERROR_PARSE;
                    goto cleanup;
                }
                if (compression == SAS_COMPRESSION_NONE) {
                    signature = sas_read4(page + offset, ctx->bswap);
                    if (!ctx->little_endian && signature == -1 && ctx->u64) {
                        signature = sas_read4(page + offset + 4, ctx->bswap);
                    }
                    if (is_compressed_data && !sas_signature_is_recognized(signature)) {
                        if (len != ctx->row_length) {
                            retval = READSTAT_ERROR_ROW_WIDTH_MISMATCH;
                            goto cleanup;
                        }
                        if ((retval = submit_columns_if_needed(ctx)) != READSTAT_OK) {
                            goto cleanup;
                        }
                        if ((retval = sas_parse_single_row(page + offset, ctx)) != READSTAT_OK) {
                            goto cleanup;
                        }
                    } else {
                        if (signature != SAS_SUBHEADER_SIGNATURE_COLUMN_TEXT) {
                            if ((retval = sas_parse_subheader(signature, page + offset, len, ctx)) != READSTAT_OK) {
                                goto cleanup;
                            }
                        }
                    }
                } else if (compression == SAS_COMPRESSION_ROW) {
                    if ((retval = submit_columns_if_needed(ctx)) != READSTAT_OK) {
                        goto cleanup;
                    }
                    if ((retval = sas_parse_subheader_rle(page + offset, len, ctx)) != READSTAT_OK) {
                        goto cleanup;
                    }
                } else {
                    retval = READSTAT_ERROR_UNSUPPORTED_COMPRESSION;
                    goto cleanup;
                }
            }

            shp += lshp;
        }

        if ((page_type & SAS_PAGE_TYPE_MASK) == SAS_PAGE_TYPE_MIX) {
            /* HACK - this is supposed to obey 8-byte boundaries but
             * some files created by Stat/Transfer don't. So verify that the
             * padding is { 0, 0, 0, 0 } or { ' ', ' ', ' ', ' ' } (or that
             * the file is not from Stat/Transfer) before skipping it */
            if ((shp-page)%8 == 4 && 
                    (*(uint32_t *)shp == 0x00000000 ||
                     *(uint32_t *)shp == 0x20202020 ||
                     ctx->vendor != READSTAT_VENDOR_STAT_TRANSFER)) {
                data = shp + 4;
            } else {
                data = shp;
            }
        }
    }
    if (data) {
        if ((retval = submit_columns_if_needed(ctx)) != READSTAT_OK) {
            goto cleanup;
        }
        if (ctx->value_handler) {
            retval = sas_parse_rows(data, ctx);
        }
    } 
cleanup:

    return retval;
}

static readstat_error_t parse_meta_pages_pass1(sas_ctx_t *ctx, int64_t *outLastExaminedPage) {
    readstat_error_t retval = READSTAT_OK;
    readstat_io_t *io = ctx->io;
    int64_t i;
    char error_buf[ERROR_BUF_SIZE];
    char *page = malloc(ctx->page_size);

    /* look for META and MIX pages at beginning... */
    for (i=0; i<ctx->page_count; i++) {
        if (io->seek(ctx->header_size + i*ctx->page_size, READSTAT_SEEK_SET, io->io_ctx) == -1) {
            retval = READSTAT_ERROR_SEEK;
            if (ctx->error_handler) {
                snprintf(error_buf, sizeof(error_buf), "ReadStat: Failed to seek to position %" PRId64 
                        " (= %" PRId64 " + %" PRId64 "*%" PRId64 ")",
                        ctx->header_size + i*ctx->page_size, ctx->header_size, i, ctx->page_size);
                ctx->error_handler(error_buf, ctx->user_ctx);
            }
            goto cleanup;
        }

        off_t off = 0;
        if (ctx->u64)
            off = 16;

        size_t head_len = off + 16 + 2;
        size_t tail_len = ctx->page_size - head_len;

        if (io->read(page, head_len, io->io_ctx) < head_len) {
            retval = READSTAT_ERROR_READ;
            goto cleanup;
        }

        uint16_t page_type = sas_read2(&page[off+16], ctx->bswap);

        if ((page_type & SAS_PAGE_TYPE_MASK) == SAS_PAGE_TYPE_DATA)
            break;
        if ((page_type & SAS_PAGE_TYPE_COMP))
            continue;

        if (io->read(page + head_len, tail_len, io->io_ctx) < tail_len) {
            retval = READSTAT_ERROR_READ;
            goto cleanup;
        }

        if ((retval = sas_parse_page_pass1(page, ctx->page_size, ctx)) != READSTAT_OK) {
            if (ctx->error_handler && retval != READSTAT_ERROR_USER_ABORT) {
                int64_t pos = io->seek(0, READSTAT_SEEK_CUR, io->io_ctx);
                snprintf(error_buf, sizeof(error_buf), 
                        "ReadStat: Error parsing page %" PRId64 ", bytes %" PRId64 "-%" PRId64 "\n", 
                        i, pos - ctx->page_size, pos-1);
                ctx->error_handler(error_buf, ctx->user_ctx);
            }
            goto cleanup;
        }
    }

cleanup:
    if (page)
        free(page);
    if (outLastExaminedPage)
        *outLastExaminedPage = i;

    return retval;
}

static readstat_error_t parse_amd_pages_pass1(int64_t last_examined_page_pass1, sas_ctx_t *ctx) {
    readstat_error_t retval = READSTAT_OK;
    readstat_io_t *io = ctx->io;
    int64_t i;
    char error_buf[ERROR_BUF_SIZE];
    char *page = malloc(ctx->page_size);
    int64_t amd_page_count = 0;

    /* ...then AMD pages at the end */
    for (i=ctx->page_count-1; i>last_examined_page_pass1; i--) {
        if (io->seek(ctx->header_size + i*ctx->page_size, READSTAT_SEEK_SET, io->io_ctx) == -1) {
            retval = READSTAT_ERROR_SEEK;
            if (ctx->error_handler) {
                snprintf(error_buf, sizeof(error_buf), "ReadStat: Failed to seek to position %" PRId64 
                        " (= %" PRId64 " + %" PRId64 "*%" PRId64 ")",
                        ctx->header_size + i*ctx->page_size, ctx->header_size, i, ctx->page_size);
                ctx->error_handler(error_buf, ctx->user_ctx);
            }
            goto cleanup;
        }

        off_t off = 0;
        if (ctx->u64)
            off = 16;

        size_t head_len = off + 16 + 2;
        size_t tail_len = ctx->page_size - head_len;

        if (io->read(page, head_len, io->io_ctx) < head_len) {
            retval = READSTAT_ERROR_READ;
            goto cleanup;
        }

        uint16_t page_type = sas_read2(&page[off+16], ctx->bswap);

        if ((page_type & SAS_PAGE_TYPE_MASK) == SAS_PAGE_TYPE_DATA) {
            /* Usually AMD pages are at the end but sometimes data pages appear after them */
            if (amd_page_count > 0)
                break;
            continue;
        }
        if ((page_type & SAS_PAGE_TYPE_COMP))
            continue;

        if (io->read(page + head_len, tail_len, io->io_ctx) < tail_len) {
            retval = READSTAT_ERROR_READ;
            goto cleanup;
        }

        if ((retval = sas_parse_page_pass1(page, ctx->page_size, ctx)) != READSTAT_OK) {
            if (ctx->error_handler && retval != READSTAT_ERROR_USER_ABORT) {
                int64_t pos = io->seek(0, READSTAT_SEEK_CUR, io->io_ctx);
                snprintf(error_buf, sizeof(error_buf), 
                        "ReadStat: Error parsing page %" PRId64 ", bytes %" PRId64 "-%" PRId64 "\n", 
                        i, pos - ctx->page_size, pos-1);
                ctx->error_handler(error_buf, ctx->user_ctx);
            }
            goto cleanup;
        }

        amd_page_count++;
    }

cleanup:
    if (page)
        free(page);

    return retval;
}

static readstat_error_t parse_all_pages_pass2(sas_ctx_t *ctx) {
    readstat_error_t retval = READSTAT_OK;
    readstat_io_t *io = ctx->io;
    int64_t i;
    char error_buf[ERROR_BUF_SIZE];
    char *page = malloc(ctx->page_size);

    for (i=0; i<ctx->page_count; i++) {
        if ((retval = sas_update_progress(ctx)) != READSTAT_OK) {
            goto cleanup;
        }
        if (io->read(page, ctx->page_size, io->io_ctx) < ctx->page_size) {
            retval = READSTAT_ERROR_READ;
            goto cleanup;
        }

        if ((retval = sas_parse_page_pass2(page, ctx->page_size, ctx)) != READSTAT_OK) {
            if (ctx->error_handler && retval != READSTAT_ERROR_USER_ABORT) {
                int64_t pos = io->seek(0, READSTAT_SEEK_CUR, io->io_ctx);
                snprintf(error_buf, sizeof(error_buf), 
                        "ReadStat: Error parsing page %" PRId64 ", bytes %" PRId64 "-%" PRId64 "\n", 
                        i, pos - ctx->page_size, pos-1);
                ctx->error_handler(error_buf, ctx->user_ctx);
            }
            goto cleanup;
        }
        if (ctx->parsed_row_count == ctx->row_limit)
            break;
    }
cleanup:
    if (page)
        free(page);

    return retval;
}

readstat_error_t readstat_parse_sas7bdat(readstat_parser_t *parser, const char *path, void *user_ctx) {
    int64_t last_examined_page_pass1 = 0;
    readstat_error_t retval = READSTAT_OK;
    readstat_io_t *io = parser->io;
    char error_buf[ERROR_BUF_SIZE];

    sas_ctx_t  *ctx = calloc(1, sizeof(sas_ctx_t));
    sas_header_info_t  *hinfo = calloc(1, sizeof(sas_header_info_t));

    ctx->info_handler = parser->info_handler;
    ctx->metadata_handler = parser->metadata_handler;
    ctx->variable_handler = parser->variable_handler;
    ctx->value_handler = parser->value_handler;
    ctx->error_handler = parser->error_handler;
    ctx->progress_handler = parser->progress_handler;
    ctx->input_encoding = parser->input_encoding;
    ctx->output_encoding = parser->output_encoding;
    ctx->user_ctx = user_ctx;
    ctx->io = parser->io;
    ctx->row_limit = parser->row_limit;

    if (io->open(path, io->io_ctx) == -1) {
        retval = READSTAT_ERROR_OPEN;
        goto cleanup;
    }

    if ((ctx->file_size = io->seek(0, READSTAT_SEEK_END, io->io_ctx)) == -1) {
        retval = READSTAT_ERROR_SEEK;
        if (ctx->error_handler) {
            snprintf(error_buf, sizeof(error_buf), "ReadStat: Failed to seek to end of file\n");
            ctx->error_handler(error_buf, ctx->user_ctx);
        }
        goto cleanup;
    }

    if (io->seek(0, READSTAT_SEEK_SET, io->io_ctx) == -1) {
        retval = READSTAT_ERROR_SEEK;
        if (ctx->error_handler) {
            snprintf(error_buf, sizeof(error_buf), "ReadStat: Failed to seek to beginning of file\n");
            ctx->error_handler(error_buf, ctx->user_ctx);
        }
        goto cleanup;
    }

    if ((retval = sas_read_header(io, hinfo, ctx->error_handler, user_ctx)) != READSTAT_OK) {
        goto cleanup;
    }

    ctx->u64 = hinfo->u64;
    ctx->little_endian = hinfo->little_endian;
    ctx->vendor = hinfo->vendor;
    ctx->bswap = machine_is_little_endian() ^ hinfo->little_endian;
    ctx->header_size = hinfo->header_size;
    ctx->page_count = hinfo->page_count;
    ctx->page_size = hinfo->page_size;
    ctx->page_header_size = hinfo->page_header_size;
    ctx->subheader_pointer_size = hinfo->subheader_pointer_size;
    ctx->timestamp = hinfo->modification_time;
    ctx->version = 10000 * hinfo->major_version + hinfo->minor_version;
    if (ctx->input_encoding == NULL) {
        ctx->input_encoding = hinfo->encoding;
    }

    if (ctx->input_encoding && ctx->output_encoding && strcmp(ctx->input_encoding, ctx->output_encoding) != 0) {
        iconv_t converter = iconv_open(ctx->output_encoding, ctx->input_encoding);
        if (converter == (iconv_t)-1) {
            retval = READSTAT_ERROR_UNSUPPORTED_CHARSET;
            goto cleanup;
        }
        ctx->converter = converter;
    }

    if ((retval = readstat_convert(ctx->file_label, sizeof(ctx->file_label),
                hinfo->file_label, sizeof(hinfo->file_label), ctx->converter)) != READSTAT_OK) {
        goto cleanup;
    }

    if ((retval = parse_meta_pages_pass1(ctx, &last_examined_page_pass1)) != READSTAT_OK) {
        goto cleanup;
    }

    if ((retval = parse_amd_pages_pass1(last_examined_page_pass1, ctx)) != READSTAT_OK) {
        goto cleanup;
    }

    if (io->seek(ctx->header_size, READSTAT_SEEK_SET, io->io_ctx) == -1) {
        retval = READSTAT_ERROR_SEEK;
        if (ctx->error_handler) {
            snprintf(error_buf, sizeof(error_buf), "ReadStat: Failed to seek to position %" PRId64 "\n", 
                    ctx->header_size);
            ctx->error_handler(error_buf, ctx->user_ctx);
        }
        goto cleanup;
    }

    if ((retval = parse_all_pages_pass2(ctx)) != READSTAT_OK) {
        goto cleanup;
    }
    
    if ((retval = submit_columns_if_needed(ctx)) != READSTAT_OK) {
        goto cleanup;
    }

    if (ctx->value_handler && ctx->parsed_row_count != ctx->row_limit) {
        retval = READSTAT_ERROR_ROW_COUNT_MISMATCH;
        if (ctx->error_handler) {
            snprintf(error_buf, sizeof(error_buf), "ReadStat: Expected %d rows in file, found %d\n",
                    ctx->row_limit, ctx->parsed_row_count);
            ctx->error_handler(error_buf, ctx->user_ctx);
        }
        goto cleanup;
    }

    if ((retval = sas_update_progress(ctx)) != READSTAT_OK) {
        goto cleanup;
    }

cleanup:
    io->close(io->io_ctx);

    if (retval == READSTAT_ERROR_OPEN ||
            retval == READSTAT_ERROR_READ ||
            retval == READSTAT_ERROR_SEEK) {
        if (ctx->error_handler) {
            snprintf(error_buf, sizeof(error_buf), "ReadStat: %s (retval = %d): %s (errno = %d)\n", 
                    readstat_error_message(retval), retval, strerror(errno), errno);
            ctx->error_handler(error_buf, user_ctx);
        }
    }

    if (ctx)
        sas_ctx_free(ctx);
    if (hinfo)
        free(hinfo);

    return retval;
}
