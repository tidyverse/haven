

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

#define SAS_DEFAULT_STRING_ENCODING "WINDOWS-1252"

#define SAS_ALIGNMENT_OFFSET_4  0x33

#define SAS_FILE_FORMAT_UNIX    '1'
#define SAS_FILE_FORMAT_WINDOWS '2'

#define SAS_ENDIAN_BIG       0x00
#define SAS_ENDIAN_LITTLE    0x01

#define SAS_PAGE_TYPE_META   0x0000
#define SAS_PAGE_TYPE_DATA   0x0100
#define SAS_PAGE_TYPE_MIX    0x0200
#define SAS_PAGE_TYPE_AMD    0x0400
#define SAS_PAGE_TYPE_MASK   0x0F00

#define SAS_PAGE_TYPE_META2  0x4000
#define SAS_PAGE_TYPE_COMP   0x9000

#define SAS_COLUMN_TYPE_NUM  0x01
#define SAS_COLUMN_TYPE_CHR  0x02

#define SAS_COMPRESSION_NONE   0x00
#define SAS_COMPRESSION_TRUNC  0x01
#define SAS_COMPRESSION_ROW    0x04

#define SAS_COMPRESSION_SIGNATURE_RLE  "SASYZCRL"
#define SAS_COMPRESSION_SIGNATURE_RDC  "SASYZCR2"

#define SAS_RLE_COMMAND_COPY64          0
#define SAS_RLE_COMMAND_INSERT_BYTE18   4
#define SAS_RLE_COMMAND_INSERT_BLANK17  6
#define SAS_RLE_COMMAND_INSERT_ZERO17   7
#define SAS_RLE_COMMAND_COPY1           8
#define SAS_RLE_COMMAND_COPY17          9
#define SAS_RLE_COMMAND_COPY33         10
#define SAS_RLE_COMMAND_COPY49         11
#define SAS_RLE_COMMAND_INSERT_BYTE3   12
#define SAS_RLE_COMMAND_INSERT_AT2     13
#define SAS_RLE_COMMAND_INSERT_BLANK2  14
#define SAS_RLE_COMMAND_INSERT_ZERO2   15

static unsigned char sas7bdat_magic_number[32] = {
    0x00, 0x00, 0x00, 0x00,   0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,   0xc2, 0xea, 0x81, 0x60,
    0xb3, 0x14, 0x11, 0xcf,   0xbd, 0x92, 0x08, 0x00,
    0x09, 0xc7, 0x31, 0x8c,   0x18, 0x1f, 0x10, 0x11
};

static unsigned char sas7bcat_magic_number[32] = {
    0x00, 0x00, 0x00, 0x00,   0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,   0xc2, 0xea, 0x81, 0x63,
    0xb3, 0x14, 0x11, 0xcf,   0xbd, 0x92, 0x08, 0x00,
    0x09, 0xc7, 0x31, 0x8c,   0x18, 0x1f, 0x10, 0x11
};

static readstat_charset_entry_t _charset_table[] = { 
    { .code = 0,     .name = SAS_DEFAULT_STRING_ENCODING },
    { .code = 20,    .name = "UTF-8" },
    { .code = 28,    .name = "US-ASCII" },
    { .code = 29,    .name = "ISO-8859-1" },
    { .code = 30,    .name = "ISO-8859-2" },
    { .code = 31,    .name = "ISO-8859-3" },
    { .code = 34,    .name = "ISO-8859-6" },
    { .code = 35,    .name = "ISO-8859-7" },
    { .code = 36,    .name = "ISO-8859-8" },
    { .code = 39,    .name = "ISO-8859-11" },
    { .code = 40,    .name = "ISO-8859-9" },
    { .code = 60,    .name = "WINDOWS-1250" },
    { .code = 61,    .name = "WINDOWS-1251" },
    { .code = 62,    .name = "WINDOWS-1252" },
    { .code = 63,    .name = "WINDOWS-1253" },
    { .code = 64,    .name = "WINDOWS-1254" },
    { .code = 65,    .name = "WINDOWS-1255" },
    { .code = 66,    .name = "WINDOWS-1256" },
    { .code = 119,   .name = "EUC-TW" },
    { .code = 123,   .name = "BIG-5" },
    { .code = 125,   .name = "EUC-CN" },
    { .code = 134,   .name = "EUC-JP" },
    { .code = 138,   .name = "SHIFT-JIS" },
    { .code = 140,   .name = "EUC-KR" }
};

#define SAS_SUBHEADER_SIGNATURE_ROW_SIZE       0xF7F7F7F7
#define SAS_SUBHEADER_SIGNATURE_COLUMN_SIZE    0xF6F6F6F6
#define SAS_SUBHEADER_SIGNATURE_COUNTS         0xFFFFFC00
#define SAS_SUBHEADER_SIGNATURE_COLUMN_FORMAT  0xFFFFFBFE
#define SAS_SUBHEADER_SIGNATURE_COLUMN_UNKNOWN 0xFFFFFFFA
#define SAS_SUBHEADER_SIGNATURE_COLUMN_ATTRS   0xFFFFFFFC
#define SAS_SUBHEADER_SIGNATURE_COLUMN_TEXT    0xFFFFFFFD
#define SAS_SUBHEADER_SIGNATURE_COLUMN_LIST    0xFFFFFFFE
#define SAS_SUBHEADER_SIGNATURE_COLUMN_NAME    0xFFFFFFFF

enum {
    READSTAT_VENDOR_STAT_TRANSFER,
    READSTAT_VENDOR_SAS
};

typedef struct text_ref_s {
    int    index;
    int    offset;
    int    length;
} text_ref_t;

typedef struct col_info_s {
    text_ref_t  name_ref;
    text_ref_t  format_ref;
    text_ref_t  label_ref;

    int    index;
    int    offset;
    int    width;
    int    type;
} col_info_t;

typedef struct sas_header_info_s {
    int      little_endian;
    int      u64;
    int      vendor;
    int64_t  page_size;
    int64_t  page_count;
    char    *encoding;
} sas_header_info_t;

typedef struct sas_catalog_ctx_s {
    readstat_value_label_handler   value_label_handler;
    void         *user_ctx;
    int           u64;
    int           bswap;
    iconv_t       converter;
} sas_catalog_ctx_t;

typedef struct sas_ctx_s {
    readstat_info_handler       info_handler;
    readstat_variable_handler   variable_handler;
    readstat_value_handler      value_handler;
    readstat_error_handler      error_handler;
    readstat_progress_handler   progress_handler;
    int64_t                     file_size;

    int           little_endian;
    int           u64;
    int           vendor;
    void         *user_ctx;
    int           bswap;
    int           did_submit_columns;
    int32_t       row_length;
    int32_t       page_row_count;
    int32_t       parsed_row_count;
    int32_t       total_row_count;
    int32_t       column_count;
    int           text_blob_count;
    size_t       *text_blob_lengths;
    char        **text_blobs;

    int           col_names_count;
    int           col_attrs_count;
    int           col_formats_count;

    int           max_col_width;
    char         *scratch_buffer;
    size_t        scratch_buffer_len;

    int           col_info_count;
    col_info_t   *col_info;

    iconv_t       converter;
} sas_ctx_t;

static uint64_t read8(const char *data, int bswap) {
    uint64_t tmp;
    memcpy(&tmp, data, 8);
    return bswap ? byteswap8(tmp) : tmp;
}

static uint32_t read4(const char *data, int bswap) {
    uint32_t tmp;
    memcpy(&tmp, data, 4);
    return bswap ? byteswap4(tmp) : tmp;
}

static uint16_t read2(const char *data, int bswap) {
    uint16_t tmp;
    memcpy(&tmp, data, 2);
    return bswap ? byteswap2(tmp) : tmp;
}

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

static void sas_catalog_ctx_free(sas_catalog_ctx_t *ctx) {
    if (ctx->converter)
        iconv_close(ctx->converter);

    free(ctx);
}

static readstat_error_t sas_update_progress(int fd, sas_ctx_t *ctx) {
    if (!ctx->progress_handler)
        return READSTAT_OK;

    return readstat_update_progress(fd, ctx->file_size, ctx->progress_handler, ctx->user_ctx);
}

static readstat_error_t sas_read_header(int fd, sas_header_info_t *ctx, 
        readstat_error_handler error_handler, void *user_ctx) {
    sas_header_start_t  header_start;
    sas_header_end_t    header_end;
    int retval = READSTAT_OK;
    char error_buf[1024];
    int a1 = 0;
    if (read(fd, &header_start, sizeof(sas_header_start_t)) < sizeof(sas_header_start_t)) {
        retval = READSTAT_ERROR_READ;
        goto cleanup;
    }
    if (memcmp(header_start.magic, sas7bdat_magic_number, sizeof(sas7bdat_magic_number)) != 0 &&
            memcmp(header_start.magic, sas7bcat_magic_number, sizeof(sas7bdat_magic_number)) != 0) {
        retval = READSTAT_ERROR_PARSE;
        goto cleanup;
    }
    if (header_start.a1 == SAS_ALIGNMENT_OFFSET_4) {
        a1 = 4;
    }
    if (header_start.a2 == SAS_ALIGNMENT_OFFSET_4) {
        ctx->u64 = 1;
    }
    int bswap = 0;
    if (header_start.endian == SAS_ENDIAN_BIG) {
        bswap = machine_is_little_endian();
        ctx->little_endian = 0;
    } else if (header_start.endian == SAS_ENDIAN_LITTLE) {
        bswap = !machine_is_little_endian();
        ctx->little_endian = 1;
    } else {
        retval = READSTAT_ERROR_PARSE;
        goto cleanup;
    }
    int i;
    for (i=0; i<sizeof(_charset_table)/sizeof(_charset_table[0]); i++) {
        if (header_start.encoding == _charset_table[i].code) {
            ctx->encoding = _charset_table[i].name;
            break;
        }
    }
    if (ctx->encoding == NULL) {
        if (error_handler) {
            snprintf(error_buf, sizeof(error_buf), "Unsupported character set code: %d\n", header_start.encoding);
            error_handler(error_buf, user_ctx);
        }
        retval = READSTAT_ERROR_UNSUPPORTED_CHARSET;
        goto cleanup;
    }
    if (readstat_lseek(fd, 196 + a1, SEEK_SET) == -1) {
        retval = READSTAT_ERROR_SEEK;
        if (error_handler) {
            snprintf(error_buf, sizeof(error_buf), "ReadStat: Failed to seek to position %d\n", 196 + a1);
            error_handler(error_buf, user_ctx);
        }
        goto cleanup;
    }

    uint32_t header_size, page_size;

    if (read(fd, &header_size, sizeof(uint32_t)) < sizeof(uint32_t)) {
        retval = READSTAT_ERROR_READ;
        goto cleanup;
    }
    if (read(fd, &page_size, sizeof(uint32_t)) < sizeof(uint32_t)) {
        retval = READSTAT_ERROR_READ;
        goto cleanup;
    }

    header_size = bswap ? byteswap4(header_size) : header_size;

    ctx->page_size = bswap ? byteswap4(page_size) : page_size;

    if (header_size < 1024) {
        retval = READSTAT_ERROR_PARSE;
        goto cleanup;
    }

    if (ctx->u64) {
        uint64_t page_count;
        if (read(fd, &page_count, sizeof(uint64_t)) < sizeof(uint64_t)) {
            retval = READSTAT_ERROR_READ;
            goto cleanup;
        }
        ctx->page_count = bswap ? byteswap8(page_count) : page_count;
    } else {
        uint32_t page_count;
        if (read(fd, &page_count, sizeof(uint32_t)) < sizeof(uint32_t)) {
            retval = READSTAT_ERROR_READ;
            goto cleanup;
        }
        ctx->page_count = bswap ? byteswap4(page_count) : page_count;
    }
    
    if (readstat_lseek(fd, 8, SEEK_CUR) == -1) {
        retval = READSTAT_ERROR_SEEK;
        if (error_handler) {
            snprintf(error_buf, sizeof(error_buf), "ReadStat: Failed to seek forward by %d\n", 8);
            error_handler(error_buf, user_ctx);
        }
        goto cleanup;
    }
    if (read(fd, &header_end, sizeof(sas_header_end_t)) < sizeof(sas_header_end_t)) {
        retval = READSTAT_ERROR_READ;
        goto cleanup;
    }
    if (strncmp(header_end.release, "9.0000M0", sizeof(header_end.release)) == 0) {
        /* A bit of a hack, but most SAS installations are running a minor update */
        ctx->vendor = READSTAT_VENDOR_STAT_TRANSFER;
    } else {
        ctx->vendor = READSTAT_VENDOR_SAS;
    }
    if (readstat_lseek(fd, header_size, SEEK_SET) == -1) {
        retval = READSTAT_ERROR_SEEK;
        if (error_handler) {
            snprintf(error_buf, sizeof(error_buf), "ReadStat: Failed to seek to position %u\n", header_size);
            error_handler(error_buf, user_ctx);
        }
        goto cleanup;
    }

cleanup:
    return retval;
}

static readstat_error_t sas_parse_column_text_subheader(const char *subheader, size_t len, sas_ctx_t *ctx) {
    readstat_error_t retval = READSTAT_OK;
    size_t signature_len = ctx->u64 ? 8 : 4;
    uint16_t remainder = read2(&subheader[signature_len], ctx->bswap);
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
        col_count = read8(&subheader[8], ctx->bswap);
    } else {
        col_count = read4(&subheader[4], ctx->bswap);
    }

    ctx->column_count = col_count;

    return retval;
}

static readstat_error_t sas_parse_row_size_subheader(const char *subheader, size_t len, sas_ctx_t *ctx) {
    readstat_error_t retval = READSTAT_OK;
    uint64_t total_row_count;
    uint64_t row_length, page_row_count;

    if (ctx->u64) {
        row_length = read8(&subheader[40], ctx->bswap);
        total_row_count = read8(&subheader[48], ctx->bswap);
        page_row_count = read8(&subheader[120], ctx->bswap);
    } else {
        row_length = read4(&subheader[20], ctx->bswap);
        total_row_count = read4(&subheader[24], ctx->bswap);
        page_row_count = read4(&subheader[60], ctx->bswap);
    }

    ctx->row_length = row_length;
    ctx->page_row_count = page_row_count;
    ctx->total_row_count = total_row_count;

    return retval;
}

static text_ref_t sas_parse_text_ref(const char *data, sas_ctx_t *ctx) {
    text_ref_t  ref;

    ref.index = read2(&data[0], ctx->bswap);
    ref.offset = read2(&data[2], ctx->bswap);
    ref.length = read2(&data[4], ctx->bswap);

    return ref;
}

static readstat_error_t copy_text_ref(char *out_buffer, size_t out_buffer_len, text_ref_t text_ref, sas_ctx_t *ctx) {
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
    uint16_t remainder = read2(&subheader[signature_len], ctx->bswap);

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
    uint16_t remainder = read2(&subheader[signature_len], ctx->bswap);

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
            ctx->col_info[i].offset = read8(&cap[0], ctx->bswap);
        } else {
            ctx->col_info[i].offset = read4(&cap[0], ctx->bswap);
        }

        off_t off=4;
        if (ctx->u64)
            off=8;

        ctx->col_info[i].width = read4(&cap[off], ctx->bswap);
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

        value.v.double_value = dval;
        value.is_system_missing = isnan(dval);
    }
    cb_retval = ctx->value_handler(ctx->parsed_row_count, col_info->index, 
            value, ctx->user_ctx);

    if (cb_retval)
        retval = READSTAT_ERROR_USER_ABORT;

cleanup:
    return retval;
}

static readstat_error_t sas_parse_single_row(const char *data, sas_ctx_t *ctx) {
    readstat_error_t retval = READSTAT_OK;
    int j;
    ctx->scratch_buffer_len = 4*ctx->max_col_width+1;
    ctx->scratch_buffer = realloc(ctx->scratch_buffer, ctx->scratch_buffer_len);
    for (j=0; j<ctx->column_count; j++) {
        col_info_t *col_info = &ctx->col_info[j];
        retval = handle_data_value(&data[col_info->offset], col_info, ctx);
        if (retval != READSTAT_OK) {
            goto cleanup;
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
    for (i=0; i<ctx->page_row_count && ctx->parsed_row_count < ctx->total_row_count; i++) {
        if ((retval = sas_parse_single_row(&data[row_offset], ctx)) != READSTAT_OK)
            goto cleanup;

        row_offset += ctx->row_length;
    }

cleanup:
    return retval;
}

static readstat_error_t sas_parse_subheader_rle(const char *subheader, size_t len, sas_ctx_t *ctx) {
    /* TODO bounds checking */
    readstat_error_t retval = READSTAT_OK;
    const unsigned char *input = (const unsigned char *)subheader;
    char error_buf[1024];
    char *buffer = malloc(ctx->row_length);
    char *output = buffer;
    if (buffer == NULL) {
        retval = READSTAT_ERROR_MALLOC;
        goto cleanup;
    }
    while (input < (const unsigned char *)subheader + len) {
        unsigned char control = *input++;
        unsigned char command = (control & 0xF0) >> 4;
        unsigned char length = (control & 0x0F);
        int copy_len = 0;
        int insert_len = 0;
        unsigned char insert_byte = '\0';
        switch (command) {
            case SAS_RLE_COMMAND_COPY64:
                copy_len = (*input++) + 64 + length * 256;
                break;
            case SAS_RLE_COMMAND_INSERT_BYTE18:
                insert_len = (*input++) + 18 + length * 16;
                insert_byte = *input++;
                break;
            case SAS_RLE_COMMAND_INSERT_BLANK17:
                insert_len = (*input++) + 17 + length * 256;
                insert_byte = ' ';
                break;
            case SAS_RLE_COMMAND_INSERT_ZERO17:
                insert_len = (*input++) + 17 + length * 256;
                insert_byte = '\0';
                break;
            case SAS_RLE_COMMAND_COPY1:  copy_len = length + 1; break;
            case SAS_RLE_COMMAND_COPY17: copy_len = length + 17; break;
            case SAS_RLE_COMMAND_COPY33: copy_len = length + 33; break;
            case SAS_RLE_COMMAND_COPY49: copy_len = length + 49; break;
            case SAS_RLE_COMMAND_INSERT_BYTE3:
                insert_byte = *input++;
                insert_len = length + 3;
                break;
            case SAS_RLE_COMMAND_INSERT_AT2:
                insert_byte = '@';
                insert_len = length + 2;
                break;
            case SAS_RLE_COMMAND_INSERT_BLANK2:
                insert_byte = ' ';
                insert_len = length + 2;
                break;
            case SAS_RLE_COMMAND_INSERT_ZERO2:
                insert_byte = '\0';
                insert_len = length + 2;
                break;
            default:
                retval = READSTAT_ERROR_PARSE;
                goto cleanup;
        }
        if (copy_len) {
            memcpy(output, input, copy_len);
            input += copy_len;
            output += copy_len;
        }
        if (insert_len) {
            memset(output, insert_byte, insert_len);
            output += insert_len;
        }
    }
    if (output - buffer != ctx->row_length) {
        retval = READSTAT_ERROR_ROW_WIDTH_MISMATCH;
        if (ctx->error_handler) {
            snprintf(error_buf, sizeof(error_buf), 
                    "ReadStat: Row #%d decompressed to %ld bytes (expected %d bytes)\n",
                    ctx->parsed_row_count, output - buffer, ctx->row_length);
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

static readstat_error_t sas_parse_catalog_page(const char *page, size_t page_size, sas_catalog_ctx_t *ctx) {
    readstat_error_t retval = READSTAT_OK;
    if (ctx->u64)
        retval = READSTAT_ERROR_PARSE;

    /* Doubles appear to be stored as big-endian, always */
    int bswap_doubles = machine_is_little_endian();
    int i;
    for (i=16; i<22; i++) {
        if (page[i]) {
            /* not a labels page... I think */
            goto cleanup;
        }
    }

    const char *lsp = &page[22];

    while (lsp < page + page_size) {
        size_t block_size = 255 * (1+lsp[9]);
        size_t pad = (lsp[12] & 0x08) ? 4 : 0; // might be 0x10, not sure
        int label_count_capacity = read4(&lsp[48+pad], ctx->bswap);
        int label_count_used = read4(&lsp[52+pad], ctx->bswap);
        char name[4*32+1];

        retval = readstat_convert(name, sizeof(name), &lsp[18], 8, ctx->converter);
        if (retval != READSTAT_OK)
            goto cleanup;

        int is_string = (name[0] == '$');

        if (pad) {
            pad += 16;
        }

        if ((lsp[12] & 0x80)) { // has long name
            /* Uncomment to return long name to client code instead of short name
            retval = readstat_convert(name, sizeof(name), &lsp[116+pad], 32, ctx->converter);
            if (retval != READSTAT_OK)
                goto cleanup;
                */
            pad += 32;
        }

        const char *lbp1 = &lsp[116+pad];

        /* Pass 1 -- find out the offset of the labels */
        for (i=0; i<label_count_capacity; i++) {
            if (&lbp1[2] - lsp > block_size) {
                retval = READSTAT_ERROR_PARSE;
                goto cleanup;
            }
            lbp1 += 6 + lbp1[2];
        }

        const char *lbp2 = lbp1;
        lbp1 = &lsp[116+pad];

        /* Pass 2 -- parse pairs of values & labels */
        for (i=0; i<label_count_used; i++) {
            if (&lbp1[30] - lsp > block_size ||
                    &lbp2[10] - lsp > block_size) {
                retval = READSTAT_ERROR_PARSE;
                goto cleanup;
            }
            size_t label_len = read2(&lbp2[8], ctx->bswap);
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
                uint64_t val = read8(&lbp1[22], bswap_doubles);
                double dval;
                memcpy(&dval, &val, 8);
                dval *= -1.0;

                value.v.double_value = dval;
            }
            if (ctx->value_label_handler) {
                ctx->value_label_handler(name, value, label, ctx->user_ctx);
            }

            lbp1 += value_entry_len;
            lbp2 += 8 + 2 + label_len + 1;
        }

        lsp += block_size;
    }

cleanup:
    return retval;
}

static readstat_variable_t *sas_init_variable(sas_ctx_t *ctx, int i, readstat_error_t *out_retval) {
    readstat_error_t retval = READSTAT_OK;
    readstat_variable_t *variable = calloc(1, sizeof(readstat_variable_t));

    variable->index = i;
    variable->type = ctx->col_info[i].type;

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
        if (ctx->info_handler(ctx->total_row_count, ctx->column_count, ctx->user_ctx)) {
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

/* First, extract column text */
static readstat_error_t sas_parse_page_pass1(const char *page, size_t page_size, sas_ctx_t *ctx) {
    readstat_error_t retval = READSTAT_OK;

    off_t off = 0;
    if (ctx->u64)
        off = 16;

    uint16_t subheader_count = read2(&page[off+20], ctx->bswap);

    int i;
    const char *shp = &page[off+24];
    for (i=0; i<subheader_count; i++) {
        uint64_t offset = 0, len = 0;
        uint32_t signature = 0;
        unsigned char compression = 0;
        int lshp = 0;
        if (ctx->u64) {
            offset = read8(&shp[0], ctx->bswap);
            len = read8(&shp[8], ctx->bswap);
            compression = shp[16];
            lshp = 24;
        } else {
            offset = read4(&shp[0], ctx->bswap);
            len = read4(&shp[4], ctx->bswap);
            compression = shp[8];
            lshp = 12;
        }

        if (len > 0 && compression != SAS_COMPRESSION_TRUNC) {
            if (offset > page_size || offset + len > page_size ||
                    offset < off+24+subheader_count*lshp) {
                retval = READSTAT_ERROR_PARSE;
                goto cleanup;
            }
            if (compression == SAS_COMPRESSION_NONE) {
                signature = read4(page + offset, ctx->bswap);
                if (!ctx->little_endian && signature == -1 && ctx->u64) {
                    signature = read4(page + offset + 4, ctx->bswap);
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

    off_t off = 0;
    if (ctx->u64)
        off = 16;

    page_type = read2(&page[off+16], ctx->bswap);

    const char *data = NULL;

    if ((page_type & SAS_PAGE_TYPE_MASK) == SAS_PAGE_TYPE_DATA) {
        ctx->page_row_count = read2(&page[off+18], ctx->bswap);
        data = &page[off+24];
    } else if (!(page_type & SAS_PAGE_TYPE_COMP)) {
        uint16_t subheader_count = read2(&page[off+20], ctx->bswap);

        int i;
        const char *shp = &page[off+24];
        for (i=0; i<subheader_count; i++) {
            uint64_t offset = 0, len = 0;
            uint32_t signature = 0;
            unsigned char compression = 0;
            int lshp = 0;
            if (ctx->u64) {
                offset = read8(&shp[0], ctx->bswap);
                len = read8(&shp[8], ctx->bswap);
                compression = shp[16];
                lshp = 24;
            } else {
                offset = read4(&shp[0], ctx->bswap);
                len = read4(&shp[4], ctx->bswap);
                compression = shp[8];
                lshp = 12;
            }

            if (len > 0 && compression != SAS_COMPRESSION_TRUNC) {
                if (offset > page_size || offset + len > page_size ||
                        offset < off+24+subheader_count*lshp) {
                    retval = READSTAT_ERROR_PARSE;
                    goto cleanup;
                }
                if (compression == SAS_COMPRESSION_NONE) {
                    signature = read4(page + offset, ctx->bswap);
                    if (!ctx->little_endian && signature == -1 && ctx->u64) {
                        signature = read4(page + offset + 4, ctx->bswap);
                    }
                    if (signature != SAS_SUBHEADER_SIGNATURE_COLUMN_TEXT) {
                        if ((retval = sas_parse_subheader(signature, page + offset, len, ctx)) != READSTAT_OK) {
                            goto cleanup;
                        }
                    }
                } else if (compression == SAS_COMPRESSION_ROW) {
                    if (!ctx->did_submit_columns) {
                        if ((retval = submit_columns(ctx)) != READSTAT_OK) {
                            goto cleanup;
                        }
                        ctx->did_submit_columns = 1;
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
        if (!ctx->did_submit_columns) {
            if ((retval = submit_columns(ctx)) != READSTAT_OK) {
                goto cleanup;
            }
            ctx->did_submit_columns = 1;
        }
        if (ctx->value_handler) {
            retval = sas_parse_rows(data, ctx);
        }
    } 
cleanup:

    return retval;
}

readstat_error_t readstat_parse_sas7bdat(readstat_parser_t *parser, const char *filename, void *user_ctx) {
    int fd = -1;
    int64_t i;
    readstat_error_t retval = READSTAT_OK;
    int64_t start_pos;
    char error_buf[1024];

    sas_ctx_t  *ctx = calloc(1, sizeof(sas_ctx_t));
    sas_header_info_t  *hinfo = calloc(1, sizeof(sas_header_info_t));
    char *page = NULL;

    ctx->info_handler = parser->info_handler;
    ctx->variable_handler = parser->variable_handler;
    ctx->value_handler = parser->value_handler;
    ctx->error_handler = parser->error_handler;
    ctx->progress_handler = parser->progress_handler;
    ctx->user_ctx = user_ctx;

    if ((fd = readstat_open(filename)) == -1) {
        retval = READSTAT_ERROR_OPEN;
        goto cleanup;
    }

    if ((ctx->file_size = readstat_lseek(fd, 0, SEEK_END)) == -1) {
        retval = READSTAT_ERROR_SEEK;
        if (ctx->error_handler) {
            snprintf(error_buf, sizeof(error_buf), "ReadStat: Failed to seek to end of file\n");
            ctx->error_handler(error_buf, ctx->user_ctx);
        }
        goto cleanup;
    }

    if (readstat_lseek(fd, 0, SEEK_SET) == -1) {
        retval = READSTAT_ERROR_SEEK;
        if (ctx->error_handler) {
            snprintf(error_buf, sizeof(error_buf), "ReadStat: Failed to seek to beginning of file\n");
            ctx->error_handler(error_buf, ctx->user_ctx);
        }
        goto cleanup;
    }

    if ((retval = sas_read_header(fd, hinfo, parser->error_handler, user_ctx)) != READSTAT_OK) {
        goto cleanup;
    }

    ctx->u64 = hinfo->u64;
    ctx->little_endian = hinfo->little_endian;
    ctx->vendor = hinfo->vendor;
    ctx->bswap = machine_is_little_endian() ^ hinfo->little_endian;
    if (strcmp(hinfo->encoding, "UTF-8") != 0 &&
            strcmp(hinfo->encoding, "US-ASCII") != 0) {
        iconv_t converter = iconv_open("UTF-8", hinfo->encoding);
        if (converter == (iconv_t)-1) {
            retval = READSTAT_ERROR_UNSUPPORTED_CHARSET;
            goto cleanup;
        }
        ctx->converter = converter;
    }

    if ((start_pos = readstat_lseek(fd, 0, SEEK_CUR)) == -1) {
        retval = READSTAT_ERROR_SEEK;
        goto cleanup;
    }
    if ((page = malloc(hinfo->page_size)) == NULL) {
        retval = READSTAT_ERROR_MALLOC;
        goto cleanup;
    }

    /* look for META and MIX pages at beginning... */
    for (i=0; i<hinfo->page_count; i++) {
        if (readstat_lseek(fd, start_pos + i*hinfo->page_size, SEEK_SET) == -1) {
            retval = READSTAT_ERROR_SEEK;
            if (ctx->error_handler) {
                snprintf(error_buf, sizeof(error_buf), "ReadStat: Failed to seek to position %lld (= %lld + %lld*%lld)",
                        start_pos + i*hinfo->page_size, start_pos, i, hinfo->page_size);
                ctx->error_handler(error_buf, ctx->user_ctx);
            }
            goto cleanup;
        }

        off_t off = 0;
        if (ctx->u64)
            off = 16;

        size_t head_len = off + 16 + 2;
        size_t tail_len = hinfo->page_size - head_len;

        if (read(fd, page, head_len) < head_len) {
            retval = READSTAT_ERROR_READ;
            goto cleanup;
        }

        uint16_t page_type = read2(&page[off+16], ctx->bswap);

        if ((page_type & SAS_PAGE_TYPE_MASK) == SAS_PAGE_TYPE_DATA)
            break;
        if ((page_type & SAS_PAGE_TYPE_COMP))
            continue;

        if (read(fd, page + head_len, tail_len) < tail_len) {
            retval = READSTAT_ERROR_READ;
            goto cleanup;
        }

        if ((retval = sas_parse_page_pass1(page, hinfo->page_size, ctx)) != READSTAT_OK) {
            if (ctx->error_handler) {
                int64_t pos = readstat_lseek(fd, 0, SEEK_CUR);
                snprintf(error_buf, sizeof(error_buf), 
                        "ReadStat: Error parsing page %lld, bytes %lld-%lld\n", 
                        i, pos - hinfo->page_size, pos-1);
                ctx->error_handler(error_buf, ctx->user_ctx);
            }
            goto cleanup;
        }
    }

    int64_t last_examined_page_pass1 = i;

    /* ...then AMD pages at the end */
    for (i=hinfo->page_count-1; i>last_examined_page_pass1; i--) {
        if (readstat_lseek(fd, start_pos + i*hinfo->page_size, SEEK_SET) == -1) {
            retval = READSTAT_ERROR_SEEK;
            if (ctx->error_handler) {
                snprintf(error_buf, sizeof(error_buf), "ReadStat: Failed to seek to position %lld (= %lld + %lld*%lld)",
                        start_pos + i*hinfo->page_size, start_pos, i, hinfo->page_size);
                ctx->error_handler(error_buf, ctx->user_ctx);
            }
            goto cleanup;
        }

        off_t off = 0;
        if (ctx->u64)
            off = 16;

        size_t head_len = off + 16 + 2;
        size_t tail_len = hinfo->page_size - head_len;

        if (read(fd, page, head_len) < head_len) {
            retval = READSTAT_ERROR_READ;
            goto cleanup;
        }

        uint16_t page_type = read2(&page[off+16], ctx->bswap);

        if ((page_type & SAS_PAGE_TYPE_MASK) == SAS_PAGE_TYPE_DATA)
            break;
        if ((page_type & SAS_PAGE_TYPE_COMP))
            continue;

        if (read(fd, page + head_len, tail_len) < tail_len) {
            retval = READSTAT_ERROR_READ;
            goto cleanup;
        }

        if ((retval = sas_parse_page_pass1(page, hinfo->page_size, ctx)) != READSTAT_OK) {
            if (ctx->error_handler) {
                int64_t pos = readstat_lseek(fd, 0, SEEK_CUR);
                snprintf(error_buf, sizeof(error_buf), 
                        "ReadStat: Error parsing page %lld, bytes %lld-%lld\n", 
                        i, pos - hinfo->page_size, pos-1);
                ctx->error_handler(error_buf, ctx->user_ctx);
            }
            goto cleanup;
        }
    }

    if (readstat_lseek(fd, start_pos, SEEK_SET) == -1) {
        retval = READSTAT_ERROR_SEEK;
        if (ctx->error_handler) {
            snprintf(error_buf, sizeof(error_buf), "ReadStat: Failed to seek to position %lld\n", start_pos);
            ctx->error_handler(error_buf, ctx->user_ctx);
        }
        goto cleanup;
    }

    for (i=0; i<hinfo->page_count; i++) {
        if ((retval = sas_update_progress(fd, ctx)) != READSTAT_OK) {
            goto cleanup;
        }
        if (read(fd, page, hinfo->page_size) < hinfo->page_size) {
            retval = READSTAT_ERROR_READ;
            goto cleanup;
        }

        if ((retval = sas_parse_page_pass2(page, hinfo->page_size, ctx)) != READSTAT_OK) {
            if (ctx->error_handler) {
                int64_t pos = readstat_lseek(fd, 0, SEEK_CUR);
                snprintf(error_buf, sizeof(error_buf), 
                        "ReadStat: Error parsing page %lld, bytes %lld-%lld\n", 
                        i, pos - hinfo->page_size, pos-1);
                ctx->error_handler(error_buf, ctx->user_ctx);
            }
            goto cleanup;
        }
    }
    
    if (!ctx->did_submit_columns) {
        if ((retval = submit_columns(ctx)) != READSTAT_OK) {
            goto cleanup;
        }
        ctx->did_submit_columns = 1;
    }

    if (ctx->value_handler && ctx->parsed_row_count != ctx->total_row_count) {
        retval = READSTAT_ERROR_ROW_COUNT_MISMATCH;
        if (ctx->error_handler) {
            snprintf(error_buf, sizeof(error_buf), "ReadStat: Expected %d rows in file, found %d\n",
                    ctx->total_row_count, ctx->parsed_row_count);
            ctx->error_handler(error_buf, ctx->user_ctx);
        }
        goto cleanup;
    }

    if ((retval = sas_update_progress(fd, ctx)) != READSTAT_OK) {
        goto cleanup;
    }

    char test;
    if (read(fd, &test, 1) == 1) {
        retval = READSTAT_ERROR_PARSE;
        goto cleanup;
    }

cleanup:
    if (retval == READSTAT_ERROR_OPEN ||
            retval == READSTAT_ERROR_READ ||
            retval == READSTAT_ERROR_SEEK) {
        if (ctx->error_handler) {
            snprintf(error_buf, sizeof(error_buf), "ReadStat: %s (retval = %d): %s (errno = %d)\n", 
                    readstat_error_message(retval), retval, strerror(errno), errno);
            ctx->error_handler(error_buf, user_ctx);
        }
    }

    if (page)
        free(page);
    if (ctx)
        sas_ctx_free(ctx);
    if (fd != -1)
        readstat_close(fd);
    if (hinfo)
        free(hinfo);

    return retval;
}

readstat_error_t readstat_parse_sas7bcat(readstat_parser_t *parser, const char *filename, void *user_ctx) {
    int fd = -1;
    readstat_error_t retval = READSTAT_OK;
    int64_t i;
    char *page = NULL;

    sas_catalog_ctx_t *ctx = calloc(1, sizeof(sas_catalog_ctx_t));
    sas_header_info_t *hinfo = calloc(1, sizeof(sas_header_info_t));

    ctx->value_label_handler = parser->value_label_handler;
    ctx->user_ctx = user_ctx;

    if ((fd = readstat_open(filename)) == -1) {
        retval = READSTAT_ERROR_OPEN;
        goto cleanup;
    }

    if ((retval = sas_read_header(fd, hinfo, parser->error_handler, user_ctx)) != READSTAT_OK) {
        goto cleanup;
    }

    ctx->u64 = hinfo->u64;
    ctx->bswap = machine_is_little_endian() ^ hinfo->little_endian;
    if (strcmp(hinfo->encoding, "UTF-8") != 0 &&
            strcmp(hinfo->encoding, "US-ASCII") != 0) {
        iconv_t converter = iconv_open("UTF-8", hinfo->encoding);
        if (converter == (iconv_t)-1) {
            retval = READSTAT_ERROR_UNSUPPORTED_CHARSET;
            goto cleanup;
        }
        ctx->converter = converter;
    }

    page = malloc(hinfo->page_size);
    if (page == NULL) {
        retval = READSTAT_ERROR_MALLOC;
        goto cleanup;
    }
    for (i=0; i<hinfo->page_count; i++) {
        if (read(fd, page, hinfo->page_size) < hinfo->page_size) {
            retval = READSTAT_ERROR_READ;
            goto cleanup;
        }
        
        /* skip the first three pages cuz they suck */
        if (i < 3)
            continue;

        if ((retval = sas_parse_catalog_page(page, hinfo->page_size, ctx)) != READSTAT_OK) {
            goto cleanup;
        }
    }

    char test;
    if (read(fd, &test, 1) == 1) {
        retval = READSTAT_ERROR_PARSE;
        goto cleanup;
    }

cleanup:
    if (page)
        free(page);
    if (ctx)
        sas_catalog_ctx_free(ctx);
    if (fd != -1)
        readstat_close(fd);
    if (hinfo)
        free(hinfo);

    return retval;
}
