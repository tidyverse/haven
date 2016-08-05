//
//  readstat_por.c
//  Wizard
//
//  Created by Evan Miller on 4/17/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <stdint.h>
#include <math.h>

#include "readstat.h"
#include "readstat_por_parse.h"
#include "readstat_spss.h"
#include "CKHashTable.h"

#define POR_LINE_LENGTH 80
#define POR_LABEL_NAME_PREFIX         "labels"

static uint16_t unicode_lookup[256] = {
    '0', '0', '0', '0', '0', '0', '0', '0', '0', '0',
    '0', '0', '0', '0', '0', '0', '0', '0', '0', '0',
    '0', '0', '0', '0', '0', '0', '0', '0', '0', '0',
    '0', '0', '0', '0', '0', '0', '0', '0', '0', '0',
    '0', '0', '0', '0', '0', '0', '0', '0', '0', '0',
    '0', '0', '0', '0', '0', '0', '0', '0', '0', '0',
    '0', '0', '0', '0', '0', '1', '2', '3', '4', '5', 
    '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F', 
    'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 
    'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j',
    'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't',
    'u', 'v', 'w', 'x', 'y', 'z', ' ', '.', '<', '(',
    '+', '|', '&', '[', ']', '!', '$', '*', ')', ';',
    '^', '-', '/', 0x00A3, ',', '%', '_', '>', '?', 0x2018,
    ':', 0x00A6, '@', 0x2019, '=', '"', 0x2264, 0x25A1, 0x00B1, 0x25A0,
    0x00B0, 0x2020, '~', 0x2013, 0x2514, 0x250C, 0x2265, 0x2070, 0x2071, 0x00B2,
    0x00B3, 0x2074, 0x2075, 0x2076, 0x2077, 0x2078, 0x2079, 0x2518, 0x2510, 0x2260,
    0x2014, 0x207D, 0x207E, 0x2E38, '{', '}', '\\', 0x00A2, 0x2022, '0',
    '0', '0', '0', '0', '0', '0', '0', '0', '0', '0',
    '0', '0', '0', '0', '0', '0', '0', '0', '0', '0',
    '0', '0', '0', '0', '0', '0', '0', '0', '0', '0',
    '0', '0', '0', '0', '0', '0', '0', '0', '0', '0',
    '0', '0', '0', '0', '0', '0', '0', '0', '0', '0',
    '0', '0', '0', '0', '0', '0', '0', '0', '0', '0',
    '0', '0', '0', '0', '0', '0' };


typedef struct readstat_por_ctx_s {
    readstat_info_handler           info_handler;
    readstat_variable_handler       variable_handler;
    readstat_value_handler          value_handler;
    readstat_value_label_handler    value_label_handler;
    readstat_error_handler          error_handler;
    readstat_progress_handler       progress_handler;
    size_t                          file_size;
    void                           *user_ctx;
    int            pos;
    readstat_io_t *io;
    unsigned char  buf[100];
    size_t         buf_used;
    size_t         buf_pos;
    char           space;
    char           fweight_name[9];
    uint16_t       lookup[256];
    unsigned char *string_buffer;
    size_t         string_buffer_len;
    int            labels_offset;
    int            obs_count;
    int            var_count;
    int            var_offset;
    int            row_limit;
    spss_varinfo_t *varinfo;
    ck_hash_table_t *var_dict;
} readstat_por_ctx_t;


static int skip_newline(readstat_io_t *io);
static int utf8_encode(const unsigned char *input, size_t input_len, char *output, size_t output_len, uint16_t lookup[256]);
static int read_bytes(readstat_por_ctx_t *ctx, void *dst, size_t len);
static int read_string(readstat_por_ctx_t *ctx, char *data, size_t len);

static void por_ctx_free(readstat_por_ctx_t *ctx) {
    if (ctx->string_buffer)
        free(ctx->string_buffer);
    if (ctx->varinfo) {
        int i;
        for (i=0; i<ctx->var_count; i++) {
            if (ctx->varinfo[i].label)
                free(ctx->varinfo[i].label);
        }
        free(ctx->varinfo);
    }
    if (ctx->var_dict)
        ck_hash_table_free(ctx->var_dict);
    free(ctx);
}

static readstat_error_t por_update_progress(readstat_por_ctx_t *ctx) {
    readstat_io_t *io = ctx->io;
    return io->update(ctx->file_size, ctx->progress_handler, ctx->user_ctx, io->io_ctx);
}

static int skip_newline(readstat_io_t *io) {
    char newline[1];
    
    if (io->read(newline, sizeof(newline), io->io_ctx) != sizeof(newline)) {
        return -1;
    }
    
    if (newline[0] == '\r') {
        if (io->read(newline, sizeof(newline), io->io_ctx) != sizeof(newline)) {
            return -1;
        }
    }
    if (newline[0] != '\n') {
        return -1;
    }
    return 0;
}

static int read_bytes(readstat_por_ctx_t *ctx, void *dst, size_t len) {
    int bytes_left = len;
    int offset = 0;
    char buf[82];
    int i;
    readstat_io_t *io = ctx->io;

    while (bytes_left > POR_LINE_LENGTH - ctx->pos) {
        int line_len = POR_LINE_LENGTH - ctx->pos;
        if (io->read(buf, line_len, io->io_ctx) == -1)
            return -1;
        
        for (i=0; i<line_len; i++) {
            if (buf[i] == '\n' || (buf[i] == '\r' && buf[i+1] == '\n')) {
                break;
            }
        }
        for (; i<line_len; i++) {
            buf[i] = ctx->space;
            if (io->seek(-1, READSTAT_SEEK_CUR, io->io_ctx) == -1)
                return -1;
        }
        if (skip_newline(ctx->io) == -1)
            return -1;
        
        memcpy((char *)dst + offset, buf, line_len);
        
        ctx->pos = 0;
        
        offset += line_len;
        bytes_left -= line_len;
    }
    if (bytes_left) {
        if (io->read(buf, bytes_left, io->io_ctx) == -1)
            return -1;
        
        for (i=0; i<bytes_left; i++) {
            if (buf[i] == '\n' || (buf[i] == '\r' && buf[i+1] == '\n')) {
                break;
            }
        }
        if (i == bytes_left) {
            ctx->pos += bytes_left;
        } else {
            ctx->pos = 0;
        }
        for (; i<bytes_left; i++) {
            buf[i] = ctx->space;
            if (io->seek(-1, READSTAT_SEEK_CUR, io->io_ctx) == -1)
                return -1;
        }
        memcpy((char *)dst + offset, buf, bytes_left);
        
        offset += bytes_left;
        
        if (ctx->pos == POR_LINE_LENGTH) {
            if (skip_newline(ctx->io) == -1)
                return -1;
            ctx->pos = 0;
        }
    }
    
    return offset;
}

static uint16_t read_tag(readstat_por_ctx_t *ctx) {
    uint16_t tr_tag;
    unsigned char tag;
    if (ctx->buf_used - ctx->buf_pos > 0) {
        tr_tag = ctx->buf[ctx->buf_pos];
        ctx->buf_pos++;
    } else {
        if (read_bytes(ctx, &tag, 1) != 1) {
            return -1;
        }
        tr_tag = ctx->lookup[tag];
    }
    return tr_tag;
}

static int read_double(readstat_por_ctx_t *ctx, double *out_double) {
    size_t bytes_read = 0;
    if (ctx->buf_used == 0) {
        bytes_read = read_bytes(ctx, ctx->buf, 100);
        if (bytes_read == -1) {
            return -1;
        }
        ctx->buf_used = bytes_read;
        ctx->buf_pos = 0;
    }
    if (ctx->buf_used - ctx->buf_pos < 40) {
        memcpy(ctx->buf, &ctx->buf[ctx->buf_pos], ctx->buf_used - ctx->buf_pos);
        ctx->buf_used -= ctx->buf_pos;
        ctx->buf_pos = 0;
        bytes_read = read_bytes(ctx, &ctx->buf[ctx->buf_used], sizeof(ctx->buf)-ctx->buf_used);
        if (bytes_read == -1) {
            return -1;
        }
        ctx->buf_used += bytes_read;
    }
    char utf8_buffer[300];
    size_t len = utf8_encode(ctx->buf + ctx->buf_pos, ctx->buf_used - ctx->buf_pos, utf8_buffer, sizeof(utf8_buffer), ctx->lookup);
    
    if (utf8_buffer[0] == 'Z')
        return 1;
    
    double value;
    bytes_read = readstat_por_parse_double(utf8_buffer, len, &value, ctx->error_handler, ctx->user_ctx);
    if (bytes_read == -1) {
        return -1;
    }
    
    ctx->buf_pos += bytes_read;
    
    if (out_double) {
        *out_double = value;
    }
//    printf("Read double: %lf\n", value);
    return 0;
}

static int read_string(readstat_por_ctx_t *ctx, char *data, size_t len) {
    int retval = 0;
    double value;
    retval = read_double(ctx, &value);
    if (retval != 0)
        return retval;
    
    size_t string_length = (size_t)value;
    if (string_length <= 0 || string_length > 20000)
        return -1;
    
    if (string_length > ctx->string_buffer_len) {
        ctx->string_buffer_len = string_length;
        ctx->string_buffer = realloc(ctx->string_buffer, ctx->string_buffer_len);
    }
    
    int bytes_cached = 0, bytes_copied = 0;
    if (ctx->buf_used - ctx->buf_pos > 0) {
        bytes_cached = ctx->buf_used - ctx->buf_pos;
        if (bytes_cached <= string_length) {
            memcpy(data, &ctx->buf[ctx->buf_pos], bytes_cached);
            ctx->buf_pos = 0;
            ctx->buf_used = 0;
            bytes_copied = bytes_cached;
        } else {
            memcpy(data, &ctx->buf[ctx->buf_pos], string_length);
            ctx->buf_pos += string_length;
            bytes_copied = string_length;
        }
    }
    
    size_t bytes_encoded = 0;
    
    if (string_length > bytes_copied) {
        if (read_bytes(ctx, ctx->string_buffer, string_length - bytes_copied) == -1) {
            return -1;
        }
        bytes_encoded = utf8_encode(ctx->string_buffer, string_length - bytes_copied, data + bytes_copied, len - bytes_copied -1, ctx->lookup);
    }
    
    data[bytes_copied + bytes_encoded] = '\0';
//    printf("Read string: %s\n", data);
    return bytes_encoded;
}

static int utf8_encode(const unsigned char *input, size_t input_len, 
        char *output, size_t output_len, uint16_t lookup[256]) {
    int offset = 0;
    int i;
    for (i=0; i<input_len; i++) {
        uint16_t codepoint = lookup[input[i]];
        if (codepoint <= 0x7F) {
            if (offset + 1 > output_len)
                return offset;
            
            output[offset++] = codepoint;
        } else {
            if (codepoint <= 0x07FF) {
                if (offset + 2 > output_len)
                    return offset;
            } else /* if (codepoint <= 0xFFFF) */{
                if (offset + 3 > output_len)
                    return offset;
            }
            int printed = sprintf(output + offset, "%lc", codepoint);
            if (printed > 0) {
                offset += printed;
            } else {
                offset++;
            }
        }
    }
    return offset;
}

static readstat_error_t read_variable_count_record(readstat_por_ctx_t *ctx) {
    double value;
    if (read_double(ctx, &value) == -1) {
        return READSTAT_ERROR_PARSE;
    }
    ctx->var_count = (int)value;
    ctx->varinfo = calloc(ctx->var_count, sizeof(spss_varinfo_t));
    if (read_double(ctx, NULL) == -1) {
        return READSTAT_ERROR_PARSE;
    }
    return READSTAT_OK;
}

static readstat_error_t read_case_weight_record(readstat_por_ctx_t *ctx) {
    if (read_string(ctx, ctx->fweight_name, sizeof(ctx->fweight_name)) == -1)
        return READSTAT_ERROR_PARSE;

    return READSTAT_OK;
}

static readstat_error_t read_variable_record(readstat_por_ctx_t *ctx) {
    double value;
    int i;
    ctx->var_offset++;

    spss_varinfo_t *varinfo = &ctx->varinfo[ctx->var_offset];
    spss_format_t *formats[2] = { &varinfo->print_format, &varinfo->write_format };

    if (read_double(ctx, &value) == -1) {
        return READSTAT_ERROR_PARSE;
    }
    varinfo->labels_index = -1;
    varinfo->width = (int)value;
    if (varinfo->width == 0) {
        varinfo->type = READSTAT_TYPE_DOUBLE;
    } else {
        varinfo->type = READSTAT_TYPE_STRING;
    }
    if (read_string(ctx, varinfo->name, sizeof(varinfo->name)) == -1) {
        return READSTAT_ERROR_PARSE;
    }
    ck_str_hash_insert(varinfo->name, varinfo, ctx->var_dict);

    for (i=0; i<sizeof(formats)/sizeof(spss_format_t *); i++) {
        spss_format_t *format = formats[i];
        if (read_double(ctx, &value) == -1) {
            return READSTAT_ERROR_PARSE;
        }
        format->type = (int)value;

        if (read_double(ctx, &value) == -1) {
            return READSTAT_ERROR_PARSE;
        }
        format->width = (int)value;

        if (read_double(ctx, &value) == -1) {
            return READSTAT_ERROR_PARSE;
        }
        format->decimal_places = (int)value;
    }

    return READSTAT_OK;
}

static readstat_error_t read_missing_value_record(readstat_por_ctx_t *ctx) {
    double value;
    char string[256];
    if (ctx->varinfo[ctx->var_offset].type == READSTAT_TYPE_DOUBLE) {
        if (read_double(ctx, &value) == -1) {
            return READSTAT_ERROR_PARSE;
        }
        ctx->varinfo[ctx->var_offset].missing_values[ctx->varinfo[ctx->var_offset].n_missing_values++] = value;
        if (ctx->varinfo[ctx->var_offset].n_missing_values > 3) {
            return READSTAT_ERROR_PARSE;
        }
    } else {
        if (read_string(ctx, string, sizeof(string))) {
            return READSTAT_ERROR_PARSE;
        }
    }
    return READSTAT_OK;
}

static readstat_error_t read_missing_value_range_record(readstat_por_ctx_t *ctx) {
    double value;
    char string[256];
    if (ctx->varinfo[ctx->var_offset].type == READSTAT_TYPE_DOUBLE) {
        ctx->varinfo[ctx->var_offset].missing_range = 1;
        if (read_double(ctx, &value) == -1) {
            return READSTAT_ERROR_PARSE;
        }
        ctx->varinfo[ctx->var_offset].missing_values[0] = value;
        if (read_double(ctx, &value) == -1) {
            return READSTAT_ERROR_PARSE;
        }
        ctx->varinfo[ctx->var_offset].missing_values[1] = value;
        ctx->varinfo[ctx->var_offset].n_missing_values = 2;
    } else {
        if (read_string(ctx, string, sizeof(string))) {
            return READSTAT_ERROR_PARSE;
        }
        if (read_string(ctx, string, sizeof(string))) {
            return READSTAT_ERROR_PARSE;
        }
    }
    return READSTAT_OK;
}

static readstat_error_t read_missing_value_lo_range_record(readstat_por_ctx_t *ctx) {
    double value;
    char string[256];
    if (ctx->varinfo[ctx->var_offset].type == READSTAT_TYPE_DOUBLE) {
        ctx->varinfo[ctx->var_offset].missing_range = 1;
        if (read_double(ctx, &value) == -1) {
            return READSTAT_ERROR_PARSE;
        }
        ctx->varinfo[ctx->var_offset].missing_values[0] = -HUGE_VAL;
        ctx->varinfo[ctx->var_offset].missing_values[1] = value;
        ctx->varinfo[ctx->var_offset].n_missing_values = 2;
    } else {
        if (read_string(ctx, string, sizeof(string))) {
            return READSTAT_ERROR_PARSE;
        }
    }
    return READSTAT_OK;
}

static readstat_error_t read_missing_value_hi_range_record(readstat_por_ctx_t *ctx) {
    double value;
    char string[256];
    if (ctx->varinfo[ctx->var_offset].type == READSTAT_TYPE_DOUBLE) {
        if (read_double(ctx, &value) == -1) {
            return READSTAT_ERROR_PARSE;
        }
        ctx->varinfo[ctx->var_offset].missing_values[0] = value;
        ctx->varinfo[ctx->var_offset].missing_values[1] = HUGE_VAL;
        ctx->varinfo[ctx->var_offset].n_missing_values = 2;
    } else {
        if (read_string(ctx, string, sizeof(string)) == -1) {
            return READSTAT_ERROR_PARSE;
        }
    }
    return READSTAT_OK;
}

static readstat_error_t read_document_record(readstat_por_ctx_t *ctx) {
    double value;
    char string[256];
    int i;
    int line_count = 0;
    if (read_double(ctx, &value) == -1) {
        return READSTAT_ERROR_PARSE;
    }
    line_count = (int)value;
    for (i=0; i<line_count; i++) {
        if (read_string(ctx, string, sizeof(string)) == -1) {
            return READSTAT_ERROR_PARSE;
        }
    }
    return READSTAT_OK;
}

static readstat_error_t read_variable_label_record(readstat_por_ctx_t *ctx) {
    char string[256];
    if (read_string(ctx, string, sizeof(string)) == -1) {
        return READSTAT_ERROR_PARSE;
    }

    ctx->varinfo[ctx->var_offset].label = malloc(strlen(string) + 1);
    strcpy(ctx->varinfo[ctx->var_offset].label, string);

    return READSTAT_OK;
}

static readstat_error_t read_value_label_record(readstat_por_ctx_t *ctx) {
    double dval;
    int i;
    char string[256];
    int count = 0, label_count = 0;
    char label_name_buf[256];
    char label_buf[256];
    snprintf(label_name_buf, sizeof(label_name_buf), POR_LABEL_NAME_PREFIX "%d", ctx->labels_offset);
    readstat_types_t value_type = READSTAT_TYPE_DOUBLE;
    if (read_double(ctx, &dval) == -1) {
        return READSTAT_ERROR_PARSE;
    }
    count = (int)dval;
    for (i=0; i<count; i++) {
        if (read_string(ctx, string, sizeof(string)) == -1) {
            return READSTAT_ERROR_PARSE;
        }
        spss_varinfo_t *info = (spss_varinfo_t *)ck_str_hash_lookup(string, ctx->var_dict);
        if (info) {
            value_type = info->type;
            info->labels_index = ctx->labels_offset;
        }
    }
    if (read_double(ctx, &dval) == -1) {
        return READSTAT_ERROR_PARSE;
    }
    label_count = (int)dval;
    for (i=0; i<label_count; i++) {
        readstat_value_t value = { .type = value_type };
        if (value_type == READSTAT_TYPE_STRING) {
            if (read_string(ctx, string, sizeof(string)) == -1) {
                return READSTAT_ERROR_PARSE;
            }
            if (read_string(ctx, label_buf, sizeof(label_buf)) == -1) {
                return READSTAT_ERROR_PARSE;
            }
            value.v.string_value = string;
        } else {
            if (read_double(ctx, &dval) == -1) {
                return READSTAT_ERROR_PARSE;
            }
            if (read_string(ctx, label_buf, sizeof(label_buf)) == -1) {
                return READSTAT_ERROR_PARSE;
            }
            value.v.double_value = dval;
        }
        ctx->value_label_handler(label_name_buf, value, label_buf, ctx->user_ctx);
    }
    ctx->labels_offset++;
    return READSTAT_OK;
}

static readstat_error_t read_por_file_data(readstat_por_ctx_t *ctx) {
    int i;
    char string[256];
    char error_buf[1024];
    int retval = 0;
    readstat_error_t rs_retval = READSTAT_OK;

    while (1) {
        for (i=0; i<ctx->var_count; i++) {
            spss_varinfo_t *info = &ctx->varinfo[i];
            readstat_value_t value = { .type = info->type };

            if (info->type == READSTAT_TYPE_STRING) {
                retval = read_string(ctx, string, sizeof(string));
                if (i == 0 && retval == 1) {
                    return 0;
                } else if (retval == -1) {
                    if (ctx->error_handler) {
                        snprintf(error_buf, sizeof(error_buf), "Error in %s\n", info->name);
                        ctx->error_handler(error_buf, ctx->user_ctx);
                    }
                    rs_retval = READSTAT_ERROR_PARSE;
                    goto cleanup;
                }
                value.v.string_value = string;
//                printf("String value: %s\n", string);
            } else if (info->type == READSTAT_TYPE_DOUBLE) {
                retval = read_double(ctx, &value.v.double_value);
                if (i == 0 && retval == 1) {
                    return READSTAT_OK;
                } else if (retval != 0) {
                    if (ctx->error_handler) {
                        snprintf(error_buf, sizeof(error_buf), "Error in %s\n", info->name);
                        ctx->error_handler(error_buf, ctx->user_ctx);
                    }
                    rs_retval = READSTAT_ERROR_PARSE;
                    goto cleanup;
                }
                spss_tag_missing_double(&value, &info->missingness);
            }
            if (ctx->value_handler) {
                ctx->value_handler(ctx->obs_count, i, value, ctx->user_ctx);
            }
        }
        ctx->obs_count++;

        rs_retval = por_update_progress(ctx);
        if (rs_retval != READSTAT_OK)
            break;

        if (ctx->obs_count == ctx->row_limit)
            break;
    }
cleanup:
    return rs_retval;
}

readstat_error_t readstat_parse_por(readstat_parser_t *parser, const char *path, void *user_ctx) {
    readstat_error_t retval = READSTAT_OK;
    readstat_io_t *io = parser->io;
    unsigned char reverse_lookup[256];
    char vanity[200];
    
    readstat_por_ctx_t *ctx = calloc(1, sizeof(readstat_por_ctx_t));

    ctx->space = ' ';
    ctx->var_dict = ck_hash_table_init(1024);
    ctx->info_handler = parser->info_handler;
    ctx->variable_handler = parser->variable_handler;
    ctx->value_handler = parser->value_handler;
    ctx->value_label_handler = parser->value_label_handler;
    ctx->error_handler = parser->error_handler;
    ctx->progress_handler = parser->progress_handler;
    ctx->user_ctx = user_ctx;
    ctx->io = io;
    ctx->row_limit = parser->row_limit;
    
    if (io->open(path, io->io_ctx) == -1) {
        free(ctx);
        return READSTAT_ERROR_OPEN;
    }

    if ((ctx->file_size = io->seek(0, READSTAT_SEEK_END, io->io_ctx)) == -1) {
        retval = READSTAT_ERROR_SEEK;
        goto cleanup;
    }

    if (io->seek(0, READSTAT_SEEK_SET, io->io_ctx) == -1) {
        retval = READSTAT_ERROR_SEEK;
        goto cleanup;
    }
    
    if (read_bytes(ctx, vanity, sizeof(vanity)) != sizeof(vanity)) {
        retval = READSTAT_ERROR_READ;
        goto cleanup;
    }
    
    if (read_bytes(ctx, reverse_lookup, sizeof(reverse_lookup)) != sizeof(reverse_lookup)) {
        retval = READSTAT_ERROR_READ;
        goto cleanup;
    }
    
    ctx->space = reverse_lookup[126];

    int i;

    for (i=0; i<256; i++)
        ctx->lookup[reverse_lookup[i]] = unicode_lookup[i];
        
    unsigned char check[9];
    char tr_check[9];
    
    if (read_bytes(ctx, check, sizeof(check)-1) != sizeof(check)-1) {
        retval = READSTAT_ERROR_READ;
        goto cleanup;
    }
    
    check[8] = '\0';
    
    utf8_encode(check, sizeof(check), tr_check, sizeof(tr_check), ctx->lookup);

    if (strcmp("SPSSPORT", tr_check) != 0) {
        retval = READSTAT_ERROR_PARSE;
        goto cleanup;
    }
    
    ctx->var_offset = -1;
    
    unsigned char version;
    char string[256];

    if (read_bytes(ctx, &version, sizeof(version)) != sizeof(version)) {
        retval = READSTAT_ERROR_READ;
        goto cleanup;
    }
    if (read_string(ctx, string, sizeof(string)) == -1) { /* creation date */
        retval = READSTAT_ERROR_PARSE;
        goto cleanup;
    }
    if (read_string(ctx, string, sizeof(string)) == -1) { /* creation time */
        retval = READSTAT_ERROR_PARSE;
        goto cleanup;
    }
        
    while (1) {
        uint16_t tr_tag = read_tag(ctx);
        switch (tr_tag) {
            case '1': /* product ID */
            case '2': /* author ID */
            case '3': /* sub-product ID */
                if (read_string(ctx, string, sizeof(string)) == -1) {
                    retval = READSTAT_ERROR_PARSE;
                    goto cleanup;
                }
                break;
            case '4': /* variable count */
                retval = read_variable_count_record(ctx);
                if (retval != READSTAT_OK)
                    goto cleanup;
                break;
            case '6': /* case weight */
                retval = read_case_weight_record(ctx);
                if (retval != READSTAT_OK)
                    goto cleanup;
                break;
            case '7': /* variable */
                retval = read_variable_record(ctx);
                if (retval != READSTAT_OK)
                    goto cleanup;
                break;
            case '8': /* missing value */
                retval = read_missing_value_record(ctx);
                if (retval != READSTAT_OK)
                    goto cleanup;
                break;
            case 'B': /* missing value range */
                retval = read_missing_value_range_record(ctx);
                if (retval != READSTAT_OK)
                    goto cleanup;
                break;
            case '9': /* LO THRU x */
                retval = read_missing_value_lo_range_record(ctx);
                if (retval != READSTAT_OK)
                    goto cleanup;
                break;
            case 'A': /* x THRU HI */
                retval = read_missing_value_hi_range_record(ctx);
                if (retval != READSTAT_OK)
                    goto cleanup;
                break;
            case 'C': /* variable label */
                retval = read_variable_label_record(ctx);
                if (retval != READSTAT_OK)
                    goto cleanup;
                break;
            case 'D': /* value label */
                retval = read_value_label_record(ctx);
                if (retval != READSTAT_OK)
                    goto cleanup;
                break;
            case 'E': /* document record */
                retval = read_document_record(ctx);
                if (retval != READSTAT_OK)
                    goto cleanup;
                break;
            case 'F': /* file data */
                if (ctx->var_offset != ctx->var_count - 1) {
                    retval = READSTAT_ERROR_PARSE;
                    goto cleanup;
                }
                for (i=0; i<ctx->var_count; i++) {
                    char label_name_buf[256];
                    spss_varinfo_t *info = &ctx->varinfo[i];
                    info->missingness = spss_missingness_for_info(info);

                    readstat_variable_t *variable = spss_init_variable_for_info(info);

                    snprintf(label_name_buf, sizeof(label_name_buf), POR_LABEL_NAME_PREFIX "%d", info->labels_index);

                    int cb_retval = 0;
                    
                    if (ctx->variable_handler) {
                        ctx->variable_handler(i, variable,
                                info->labels_index == -1 ? NULL : label_name_buf,
                                user_ctx);
                    }

                    spss_free_variable(variable);

                    if (cb_retval) {
                        retval = READSTAT_ERROR_USER_ABORT;
                        goto cleanup;
                    }
                }
                if (parser->fweight_handler && ctx->fweight_name[0]) {
                    for (i=0; i<ctx->var_count; i++) {
                        spss_varinfo_t *info = &ctx->varinfo[i];
                        if (strcmp(info->name, ctx->fweight_name) == 0) {
                            if (parser->fweight_handler(i, user_ctx)) {
                                retval = READSTAT_ERROR_USER_ABORT;
                                goto cleanup;
                            }
                            break;
                        }
                    }
                }
                retval = read_por_file_data(ctx);
                if (retval != READSTAT_OK)
                    goto cleanup;
                
                if (parser->info_handler) {
                    if (parser->info_handler(ctx->obs_count, ctx->var_count, ctx->user_ctx)) {
                        retval = READSTAT_ERROR_USER_ABORT;
                    }
                }
                goto cleanup;
                
                break;
            default:
                retval = READSTAT_ERROR_PARSE;
                goto cleanup;
                
                break;
        }
    }
    
cleanup:
    io->close(io->io_ctx);
    por_ctx_free(ctx);
    
    return retval;
}
