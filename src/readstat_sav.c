//
//  sav.c
//  Wizard
//
//  Created by Evan Miller on 12/16/11.
//  Copyright (c) 2011 __MyCompanyName__. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdint.h>
#include <math.h>
#include <iconv.h>

#include "readstat_io.h"
#include "readstat_sav.h"
#include "readstat_sav_parse.h"
#include "readstat_spss.h"
#include "readstat_convert.h"

#define SAV_CHARSET_EBCDIC                1
#define SAV_CHARSET_7_BIT_ASCII           2
#define SAV_CHARSET_8_BIT_ASCII           3
#define SAV_CHARSET_DEC_KANJI             4
#define SAV_CHARSET_UTF8              65001
/* Others defined in table below */

typedef struct sav_charset_entry_s {
    int     code;
    char    name[32];
} sav_charset_entry_t;

/* See http://msdn.microsoft.com/en-us/library/dd317756(VS.85).aspx */
static sav_charset_entry_t _charset_table[] = { 
    { .code = 2,     .name = "ASCII" },
    { .code = 4,     .name = "DEC-KANJI" },
    { .code = 437,   .name = "CP437" },
    { .code = 708,   .name = "ASMO-708" },
    { .code = 737,   .name = "CP737" },
    { .code = 775,   .name = "CP775" },
    { .code = 850,   .name = "CP850" },
    { .code = 852,   .name = "CP852" },
    { .code = 855,   .name = "CP855" },
    { .code = 857,   .name = "CP857" },
    { .code = 858,   .name = "CP858" },
    { .code = 860,   .name = "CP860" },
    { .code = 861,   .name = "CP861" },
    { .code = 862,   .name = "CP862" },
    { .code = 863,   .name = "CP863" },
    { .code = 864,   .name = "CP864" },
    { .code = 865,   .name = "CP865" },
    { .code = 866,   .name = "CP866" },
    { .code = 869,   .name = "CP869" },
    { .code = 874,   .name = "CP874" },
    { .code = 932,   .name = "SHIFT-JIS" },
    { .code = 936,   .name = "ISO-IR-58" },
    { .code = 949,   .name = "ISO-IR-149" },
    { .code = 950,   .name = "BIG-5" },
    { .code = 1200,  .name = "UTF-16LE" },
    { .code = 1201,  .name = "UTF-16BE" },
    { .code = 1250,  .name = "WINDOWS-1250" },
    { .code = 1251,  .name = "WINDOWS-1251" },
    { .code = 1252,  .name = "WINDOWS-1252" },
    { .code = 1253,  .name = "WINDOWS-1253" },
    { .code = 1254,  .name = "WINDOWS-1254" },
    { .code = 1255,  .name = "WINDOWS-1255" },
    { .code = 1256,  .name = "WINDOWS-1256" },
    { .code = 1257,  .name = "WINDOWS-1257" },
    { .code = 1258,  .name = "WINDOWS-1258" },
    { .code = 1361,  .name = "CP1361" },
    { .code = 10000, .name = "MACROMAN" },
    { .code = 10004, .name = "MACARABIC" },
    { .code = 10005, .name = "MACHEBREW" },
    { .code = 10006, .name = "MACGREEK" },
    { .code = 10007, .name = "MACCYRILLIC" },
    { .code = 10010, .name = "MACROMANIA" },
    { .code = 10017, .name = "MACUKRAINE" },
    { .code = 10021, .name = "MACTHAI" },
    { .code = 10029, .name = "MACCENTRALEUROPE" },
    { .code = 10079, .name = "MACICELAND" },
    { .code = 10081, .name = "MACTURKISH" },
    { .code = 10082, .name = "MACCROATIAN" },
    { .code = 12000, .name = "UTF-32LE" },
    { .code = 12001, .name = "UTF-32BE" },
    { .code = 20127, .name = "US-ASCII" },
    { .code = 20866, .name = "KOI8-R" },
    { .code = 20932, .name = "EUC-JP" },
    { .code = 21866, .name = "KOI8-U" },
    { .code = 28591, .name = "ISO-8859-1" },
    { .code = 28592, .name = "ISO-8859-2" },
    { .code = 28593, .name = "ISO-8859-3" },
    { .code = 28594, .name = "ISO-8859-4" },
    { .code = 28595, .name = "ISO-8859-5" },
    { .code = 28596, .name = "ISO-8859-6" },
    { .code = 28597, .name = "ISO-8859-7" },
    { .code = 28598, .name = "ISO-8859-8" },
    { .code = 28599, .name = "ISO-8859-9" },
    { .code = 28603, .name = "ISO-8859-13" },
    { .code = 28605, .name = "ISO-8859-15" },
    { .code = 50220, .name = "ISO-2022-JP" },
    { .code = 50221, .name = "ISO-2022-JP" }, // same as above?
    { .code = 50222, .name = "ISO-2022-JP" }, // same as above?
    { .code = 50225, .name = "ISO-2022-KR" },
    { .code = 50229, .name = "ISO-2022-CN" },
    { .code = 51932, .name = "EUC-JP" },
    { .code = 51936, .name = "EUC-CN" },
    { .code = 51949, .name = "EUC-KR" },
    { .code = 52936, .name = "HZ-GB-2312" },
    { .code = 54936, .name = "GB18030" },
    { .code = 65000, .name = "UTF-7" },
    { .code = 65001, .name = "UTF-8" }
};

#define SAV_VARINFO_INITIAL_CAPACITY  512
#define SAV_EIGHT_SPACES              "        "
#define SAV_LABEL_NAME_PREFIX         "labels"

static void sav_ctx_free(sav_ctx_t *ctx);
static readstat_error_t sav_read_data(int fd, sav_ctx_t *ctx, readstat_handle_value_callback value_cb, void *user_ctx);
static readstat_error_t sav_read_variable_record(int fd, sav_ctx_t *ctx);
static readstat_error_t sav_read_document_record(int fd, sav_ctx_t *ctx);
static readstat_error_t sav_read_value_label_record(int fd, sav_ctx_t *ctx, readstat_handle_value_label_callback value_label_cb, void *user_ctx);
static readstat_error_t sav_read_dictionary_termination_record(int fd, sav_ctx_t *ctx);
static readstat_error_t sav_parse_machine_floating_point_record(void *data, sav_ctx_t *ctx);
static readstat_error_t sav_parse_variable_display_parameter_record(void *data, sav_ctx_t *ctx);
static readstat_error_t sav_parse_machine_integer_info_record(void *data, size_t data_len, sav_ctx_t *ctx);

sav_ctx_t *sav_ctx_init(sav_file_header_record_t *header) {
    sav_ctx_t *ctx = NULL;
    if ((ctx = malloc(sizeof(sav_ctx_t))) == NULL) {
        return NULL;
    }
    memset(ctx, 0, sizeof(sav_ctx_t));
    
    if (header->layout_code == 2 || header->layout_code == 3) {
        ctx->machine_needs_byte_swap = 0;
    } else {
        ctx->machine_needs_byte_swap = 1;
    }
    
    ctx->data_is_compressed = (header->compressed != 0);
    ctx->record_count = ctx->machine_needs_byte_swap ? byteswap4(header->ncases) : header->ncases;
    
    double bias = ctx->machine_needs_byte_swap ? byteswap_double(header->bias) : header->bias;
    
    if (bias != 100.0) {
        sav_ctx_free(ctx);
        return NULL;
    }
    
    ctx->varinfo_capacity = SAV_VARINFO_INITIAL_CAPACITY;
    
    if ((ctx->varinfo = calloc(ctx->varinfo_capacity, sizeof(sav_varinfo_t))) == NULL) {
        sav_ctx_free(ctx);
        return NULL;
    }
    
    return ctx;
}

static void sav_ctx_free(sav_ctx_t *ctx) {
    if (ctx->varinfo) {
        int i;
        for (i=0; i<ctx->var_count; i++) {
            free(ctx->varinfo[i].label);
        }
        free(ctx->varinfo);
    }
    if (ctx->converter) {
        iconv_close(ctx->converter);
    }
    free(ctx);
}

static readstat_error_t sav_read_variable_record(int fd, sav_ctx_t *ctx) {
    sav_variable_record_t variable;
    readstat_error_t retval = READSTAT_OK;
    if (ctx->var_index == ctx->varinfo_capacity) {
        if ((ctx->varinfo = realloc(ctx->varinfo, (ctx->varinfo_capacity *= 2) * sizeof(sav_varinfo_t))) == NULL) {
            retval = READSTAT_ERROR_MALLOC;
            goto cleanup;
        }
    }
    if (read(fd, &variable, sizeof(sav_variable_record_t)) < sizeof(sav_variable_record_t)) {
        retval = READSTAT_ERROR_READ;
        goto cleanup;
    }
    variable.print = ctx->machine_needs_byte_swap ? byteswap4(variable.print) : variable.print;
    variable.write = ctx->machine_needs_byte_swap ? byteswap4(variable.write) : variable.write;

    readstat_types_t dta_type = READSTAT_TYPE_DOUBLE;
    int32_t type = ctx->machine_needs_byte_swap ? byteswap4(variable.type) : variable.type;
    int i;
    if (type < 0) {
        if (ctx->var_index == 0) {
            return READSTAT_ERROR_PARSE;
        }
        ctx->var_offset++;
        sav_varinfo_t *prev = &ctx->varinfo[ctx->var_index-1];
        prev->width++;
        return 0;
    }
    if (type > 0) {
        dta_type = READSTAT_TYPE_STRING;
        // len = type;
    }
    sav_varinfo_t *info = &ctx->varinfo[ctx->var_index];
    memset(info, 0, sizeof(sav_varinfo_t));
    info->width = 1;
    info->index = ctx->var_index;
    info->offset = ctx->var_offset;
    info->type = dta_type;

    retval = readstat_convert(info->name, sizeof(info->name), variable.name, 8, ctx->converter);
    if (retval != READSTAT_OK)
        goto cleanup;

    retval = readstat_convert(info->longname, sizeof(info->longname), variable.name, 8, ctx->converter);
    if (retval != READSTAT_OK)
        goto cleanup;

    info->print_format.decimal_places = (variable.print & 0x000000FF);
    info->print_format.width = (variable.print & 0x0000FF00) >> 8;
    info->print_format.type = (variable.print  & 0x00FF0000) >> 16;

    info->write_format.decimal_places = (variable.write & 0x000000FF);
    info->write_format.width = (variable.write & 0x0000FF00) >> 8;
    info->write_format.type = (variable.write  & 0x00FF0000) >> 16;
    
    if (variable.has_var_label) {
        int32_t label_len;
        if (read(fd, &label_len, sizeof(int32_t)) < sizeof(int32_t)) {
            retval = READSTAT_ERROR_READ;
            goto cleanup;
        }
        label_len = ctx->machine_needs_byte_swap ? byteswap4(label_len) : label_len;
        int32_t label_capacity = (label_len + 3) / 4 * 4;
        char *label_buf = malloc(label_capacity);
        size_t out_label_len = label_len*4+1;
        info->label = malloc(out_label_len);
        if (label_buf == NULL || info->label == NULL) {
            retval = READSTAT_ERROR_MALLOC;
            goto cleanup;
        }
        if (read(fd, label_buf, label_capacity) < label_capacity) {
            retval = READSTAT_ERROR_READ;
            free(label_buf);
            free(info->label);
            info->label = NULL;
            goto cleanup;
        }
        retval = readstat_convert(info->label, out_label_len, label_buf, label_len, ctx->converter);
        free(label_buf);
        if (retval != READSTAT_OK)
            goto cleanup;
    }
    
    ctx->varinfo[ctx->var_index].labels_index = -1;
    
    if (variable.n_missing_values) {
        info->n_missing_values = ctx->machine_needs_byte_swap ? byteswap4(variable.n_missing_values) : variable.n_missing_values;
        if (info->n_missing_values < 0) {
            info->missing_range = 1;
            info->n_missing_values = abs(info->n_missing_values);
        } else {
            info->missing_range = 0;
        }
        if (info->n_missing_values > 3) {
            retval = READSTAT_ERROR_PARSE;
            goto cleanup;
        }
        if ((read(fd, info->missing_values, info->n_missing_values * sizeof(double))) < info->n_missing_values * sizeof(double)) {
            retval = READSTAT_ERROR_READ;
            goto cleanup;
        }
        if (ctx->machine_needs_byte_swap) {
            for (i=0; i<info->n_missing_values; i++) {
                info->missing_values[i] = byteswap_double(info->missing_values[i]);
            }
        }
    }
    
    ctx->var_index++;
    ctx->var_offset++;
    
cleanup:
    
    return retval;
}

static readstat_error_t sav_read_value_label_record(int fd, sav_ctx_t *ctx, readstat_handle_value_label_callback value_label_cb, void *user_ctx) {
    int32_t label_count;
    readstat_error_t retval = READSTAT_OK;
    int32_t *vars = NULL;
    int32_t rec_type;
    int32_t var_count;
    readstat_types_t value_type = READSTAT_TYPE_STRING;
    char label_name_buf[256];
    char label_buf[256];
    typedef struct value_label_s {
        char          value[8];
        unsigned char label_len;
        char          label[256*4+1];
    } value_label_t;
    value_label_t *value_labels = NULL;

    if (read(fd, &label_count, sizeof(int32_t)) < sizeof(int32_t)) {
        retval = READSTAT_ERROR_READ;
        goto cleanup;
    }
    if (ctx->machine_needs_byte_swap)
        label_count = byteswap4(label_count);
    
    if ((value_labels = malloc(label_count * sizeof(value_label_t))) == NULL) {
        retval = READSTAT_ERROR_MALLOC;
        goto cleanup;
    }
    
    sprintf(label_name_buf, SAV_LABEL_NAME_PREFIX "%d", ctx->value_labels_count);
    int i;
    for (i=0; i<label_count; i++) {
        value_label_t *vlabel = &value_labels[i];
        if (read(fd, vlabel, 9) < 9) {
            retval = READSTAT_ERROR_READ;
            goto cleanup;
        }
        size_t label_len = (vlabel->label_len + 8) / 8 * 8 - 1;
        if (read(fd, label_buf, label_len) < label_len) {
            retval = READSTAT_ERROR_READ;
            goto cleanup;
        }
        retval = readstat_convert(vlabel->label, sizeof(vlabel->label), label_buf, label_len, ctx->converter);
        if (retval != READSTAT_OK)
            goto cleanup;
    }

    if (read(fd, &rec_type, sizeof(int32_t)) < sizeof(int32_t)) {
        retval = READSTAT_ERROR_READ;
        goto cleanup;
    }
    if (ctx->machine_needs_byte_swap)
        rec_type = byteswap4(rec_type);
    
    if (rec_type != 4) {
        retval = READSTAT_ERROR_PARSE;
        goto cleanup;
    }
    if (read(fd, &var_count, sizeof(int32_t)) < sizeof(int32_t)) {
        retval = READSTAT_ERROR_READ;
        goto cleanup;
    }
    if (ctx->machine_needs_byte_swap)
        var_count = byteswap4(var_count);
    
    if ((vars = malloc(var_count * sizeof(int32_t))) == NULL) {
        retval = READSTAT_ERROR_MALLOC;
        goto cleanup;
    }
    
    if (read(fd, vars, var_count * sizeof(int32_t)) < var_count * sizeof(int32_t)) {
        retval = READSTAT_ERROR_READ;
        goto cleanup;
    }
    for (i=0; i<var_count; i++) {
        int var_offset = vars[i]-1; // Why subtract 1????
        sav_varinfo_t *var = bsearch_b(&var_offset, ctx->varinfo, ctx->var_index, sizeof(sav_varinfo_t), ^(const void *elem1, const void *elem2) {
            int offset = *(int *)elem1;
            const sav_varinfo_t *v = (const sav_varinfo_t *)elem2;
            if (offset < v->offset)
                return -1;
            return (offset > v->offset);
        });
        if (var) {
            value_type = var->type;
            var->labels_index = ctx->value_labels_count;
        }
    }
    for (i=0; i<label_count; i++) {
        value_label_t *vlabel = &value_labels[i];
        if (value_type == READSTAT_TYPE_DOUBLE) {
            double val_d = *(double *)vlabel->value;
            if (ctx->machine_needs_byte_swap)
                val_d = byteswap_double(val_d);
            value_label_cb(label_name_buf, &val_d, value_type, vlabel->label, user_ctx);
        } else {
            char unpadded_val[8*4+1];
            retval = readstat_convert(unpadded_val, sizeof(unpadded_val), vlabel->value, 8, ctx->converter);
            if (retval != READSTAT_OK)
                break;
            value_label_cb(label_name_buf, unpadded_val, value_type, vlabel->label, user_ctx);
        }
    }
    ctx->value_labels_count++;
cleanup:
    if (vars)
        free(vars);
    if (value_labels)
        free(value_labels);
    
    return retval;
}

static readstat_error_t sav_read_document_record(int fd, sav_ctx_t *ctx) {
    int32_t n_lines;
    readstat_error_t retval = READSTAT_OK;
    if (read(fd, &n_lines, sizeof(int32_t)) < sizeof(int32_t)) {
        retval = READSTAT_ERROR_READ;
        goto cleanup;
    }
    if (ctx->machine_needs_byte_swap)
        n_lines = byteswap4(n_lines);
    if (lseek(fd, n_lines * 80, SEEK_CUR) == -1) {
        retval = READSTAT_ERROR_READ;
        goto cleanup;
    }
    
cleanup:
    return retval;
}

static readstat_error_t sav_read_dictionary_termination_record(int fd, sav_ctx_t *ctx) {
    int32_t filler;
    readstat_error_t retval = READSTAT_OK;
    if (read(fd, &filler, sizeof(int32_t)) < sizeof(int32_t)) {
        retval = READSTAT_ERROR_READ;
    }
    return retval;
}

double handle_missing_double(double fp_value, sav_varinfo_t *info) {
    if (info->missing_range) {
        if (fp_value >= info->missing_values[0] && fp_value <= info->missing_values[1]) {
            return NAN;
        }
    } else {
        if (info->n_missing_values > 0 && fp_value == info->missing_values[0]) {
            return NAN;
        } else if (info->n_missing_values > 1 && fp_value == info->missing_values[1]) {
            return NAN;
        }
    }
    if (info->n_missing_values == 3 && fp_value == info->missing_values[2]) {
        return NAN;
    }
    if (*(uint64_t *)&fp_value == SAV_MISSING_DOUBLE)
        return NAN;
    if (*(uint64_t *)&fp_value == SAV_LOWEST_DOUBLE)
        return NAN;
    if (*(uint64_t *)&fp_value == SAV_HIGHEST_DOUBLE)
        return NAN;
    
    return fp_value;
}

static readstat_error_t sav_read_data(int fd, sav_ctx_t *ctx, readstat_handle_value_callback value_cb, void *user_ctx) {
    unsigned char value[8];
    char *raw_str_value = NULL;
    char *utf8_str_value = NULL;
    unsigned char uncompressed_value[8];
    int offset = 0;
    int segment_offset = 0;
    int row = 0, var_index = 0, col = 0;
    readstat_error_t retval = READSTAT_OK;
    int i;
    double fp_value;
    int longest_string = 256;
    int raw_str_used = 0;
    size_t utf8_str_value_len;
    for (i=0; i<ctx->var_count; i++) {
        sav_varinfo_t *info = &ctx->varinfo[i];
        if (info->string_length > longest_string) {
            longest_string = info->string_length;
        }
    }
    if ((raw_str_value = malloc(longest_string)) == NULL) {
        retval = READSTAT_ERROR_MALLOC;
        goto done;
    }
    utf8_str_value_len = longest_string*4+1;
    if ((utf8_str_value = malloc(utf8_str_value_len)) == NULL) {
        retval = READSTAT_ERROR_MALLOC;
        goto done;
    }
    while (1) {
        if (read(fd, value, 8) < 8) {
            break;
        }
        
        sav_varinfo_t *col_info = &ctx->varinfo[col];
        sav_varinfo_t *var_info = &ctx->varinfo[var_index];
        if (ctx->data_is_compressed) {
            for (i=0; i<8; i++) {
                if (offset > 31) {
                    retval = READSTAT_ERROR_PARSE;
                    goto done;
                }
                col_info = &ctx->varinfo[col];
                var_info = &ctx->varinfo[var_index];
                switch (value[i]) {
                    case 0:
                        break;
                    case 252:
                        goto done;
                    case 253:
                        if (read(fd, uncompressed_value, 8) < 8) {
                            retval = READSTAT_ERROR_READ;
                            goto done;
                        }
                        if (var_info->type == READSTAT_TYPE_STRING) {
                            if (raw_str_used + 8 <= longest_string) {
                                memcpy(raw_str_value + raw_str_used, uncompressed_value, 8);
                                raw_str_used += 8;
                            }
                            offset++;
                            if (offset == col_info->width) {
                                segment_offset++;
                                if (segment_offset == var_info->n_segments) {
                                    retval = readstat_convert(utf8_str_value, utf8_str_value_len, 
                                            raw_str_value, raw_str_used, ctx->converter);
                                    if (retval != READSTAT_OK)
                                        goto done;
                                    if (value_cb(row, var_info->index, utf8_str_value, READSTAT_TYPE_STRING, user_ctx)) {
                                        retval = READSTAT_ERROR_USER_ABORT;
                                        goto done;
                                    }
                                    raw_str_used = 0;
                                    segment_offset = 0;
                                    var_index += var_info->n_segments;
                                }
                                offset = 0;
                                col++;
                            }
                        } else if (var_info->type == READSTAT_TYPE_DOUBLE) {
                            fp_value = *(double *)uncompressed_value;
                            if (ctx->machine_needs_byte_swap) {
                                fp_value = byteswap_double(fp_value);
                            }
                            fp_value = handle_missing_double(fp_value, var_info);
                            if (value_cb(row, var_info->index, &fp_value, READSTAT_TYPE_DOUBLE, user_ctx)) {
                                retval = READSTAT_ERROR_USER_ABORT;
                                goto done;
                            }
                            var_index += var_info->n_segments;
                            col++;
                        }
                        break;
                    case 254:
                        if (var_info->type == READSTAT_TYPE_STRING) {
                            if (raw_str_used + 8 <= longest_string) {
                                memcpy(raw_str_value + raw_str_used, SAV_EIGHT_SPACES, 8);
                                raw_str_used += 8;
                            }
                            offset++;
                            if (offset == col_info->width) {
                                segment_offset++;
                                if (segment_offset == var_info->n_segments) {
                                    retval = readstat_convert(utf8_str_value, utf8_str_value_len, 
                                            raw_str_value, raw_str_used, ctx->converter);
                                    if (retval != READSTAT_OK)
                                        goto done;
                                    if (value_cb(row, var_info->index, utf8_str_value, READSTAT_TYPE_STRING, user_ctx)) {
                                        retval = READSTAT_ERROR_USER_ABORT;
                                        goto done;
                                    }
                                    raw_str_used = 0;
                                    segment_offset = 0;
                                    var_index += var_info->n_segments;
                                }
                                offset = 0;
                                col++;
                            }
                        } else {
                            retval = READSTAT_ERROR_PARSE;
                            goto done;
                        }
                        break;
                    case 255:
                        fp_value = NAN;
                        if (value_cb(row, var_info->index, &fp_value, var_info->type, user_ctx)) {
                            retval = READSTAT_ERROR_USER_ABORT;
                            goto done;
                        }
                        var_index += var_info->n_segments;
                        col++;
                        break;
                    default:
                        fp_value = value[i] - 100.0;
                        fp_value = handle_missing_double(fp_value, var_info);
                        if (value_cb(row, var_info->index, &fp_value, var_info->type, user_ctx)) {
                            retval = READSTAT_ERROR_USER_ABORT;
                            goto done;
                        }
                        var_index += var_info->n_segments;
                        col++;
                        break;
                }
                if (col == ctx->var_index) {
                    col = 0;
                    var_index = 0;
                    row++;
                }
            }
        } else {
            if (offset > 31) {
                retval = READSTAT_ERROR_PARSE;
                goto done;
            }
            if (var_info->type == READSTAT_TYPE_STRING) {
                if (raw_str_used + 8 <= longest_string) {
                    memcpy(raw_str_value + raw_str_used, value, 8);
                    raw_str_used += 8;
                }
                offset++;
                if (offset == col_info->width) {
                    segment_offset++;
                    if (segment_offset == var_info->n_segments) {
                        retval = readstat_convert(utf8_str_value, utf8_str_value_len, 
                                raw_str_value, raw_str_used, ctx->converter);
                        if (retval != READSTAT_OK)
                            goto done;
                        if (value_cb(row, var_info->index, utf8_str_value, READSTAT_TYPE_STRING, user_ctx)) {
                            retval = READSTAT_ERROR_USER_ABORT;
                            goto done;
                        }
                        raw_str_used = 0;
                        segment_offset = 0;
                        var_index += var_info->n_segments;
                    }
                    offset = 0;
                    col++;
                }
            } else if (var_info->type == READSTAT_TYPE_DOUBLE) {
                fp_value = *(double *)value;
                if (ctx->machine_needs_byte_swap) {
                    fp_value = byteswap_double(fp_value);
                }
                fp_value = handle_missing_double(fp_value, var_info);
                if (value_cb(row, var_info->index, &fp_value, READSTAT_TYPE_DOUBLE, user_ctx)) {
                    retval = READSTAT_ERROR_USER_ABORT;
                    goto done;
                }
                var_index += var_info->n_segments;
                col++;
            }
            if (col == ctx->var_index) {
                col = 0;
                var_index = 0;
                row++;
            }
        }
    }
    if (row != ctx->record_count) {
        retval = READSTAT_ERROR_PARSE;
    }
done:
    
    if (raw_str_value)
        free(raw_str_value);
    if (utf8_str_value)
        free(utf8_str_value);
    
    return retval;
}

static readstat_error_t sav_parse_machine_integer_info_record(void *data, size_t data_len, sav_ctx_t *ctx) {
    if (data_len != 32)
        return READSTAT_ERROR_PARSE;

    char *src_charset = NULL;
    int32_t character_code = ((int32_t *)data)[7];
    if (ctx->machine_needs_byte_swap) {
        character_code = byteswap4(character_code);
    }
    if (character_code == SAV_CHARSET_7_BIT_ASCII || character_code == SAV_CHARSET_UTF8) {
        /* do nothing */
    } else {
        int i;
        for (i=0; i<sizeof(_charset_table)/sizeof(_charset_table[0]); i++) {
            if (character_code  == _charset_table[i].code) {
                src_charset = _charset_table[i].name;
                break;
            }
        }
        if (src_charset == NULL) {
            dprintf(STDERR_FILENO, "Unsupported character set: %d\n", character_code);
            return READSTAT_ERROR_UNSUPPORTED_CHARSET;
        }
    }
    if (src_charset) {
        ctx->converter = iconv_open("UTF-8", src_charset);
        if (ctx->converter == (iconv_t)-1) {
            return READSTAT_ERROR_UNSUPPORTED_CHARSET;
        }
    }
    return READSTAT_OK;
}

static readstat_error_t sav_parse_machine_floating_point_record(void *data, sav_ctx_t *ctx) {
    return READSTAT_OK;
}

static readstat_error_t sav_parse_variable_display_parameter_record(void *data, sav_ctx_t *ctx) {
    return READSTAT_OK;
}

int parse_sav(const char *filename, void *user_ctx,
              readstat_handle_info_callback info_cb, readstat_handle_variable_callback variable_cb,
              readstat_handle_value_callback value_cb, readstat_handle_value_label_callback value_label_cb) {
    int fd;
    readstat_error_t retval = READSTAT_OK;
    sav_file_header_record_t header;
    sav_ctx_t *ctx = NULL;
    void *data_buf = NULL;
    size_t data_buf_capacity = 4096;
    
    if ((fd = readstat_open(filename)) == -1) {
        return READSTAT_ERROR_OPEN;
    }
    
    if ((read(fd, &header, sizeof(sav_file_header_record_t))) < sizeof(sav_file_header_record_t)) {
        retval = READSTAT_ERROR_READ;
        goto cleanup;
    }
    
    ctx = sav_ctx_init(&header);
    if (ctx == NULL) {
        retval = READSTAT_ERROR_PARSE;
        goto cleanup;
    }
    
    if ((data_buf = malloc(data_buf_capacity)) == NULL) {
        retval = READSTAT_ERROR_MALLOC;
        goto cleanup;
    }
    
    while (1) {
        int32_t rec_type;
        int32_t extra_info[3];
        size_t data_len = 0;
        int i;
        int done = 0;
        if (read(fd, &rec_type, sizeof(int32_t)) < sizeof(int32_t)) {
            retval = READSTAT_ERROR_READ;
            goto cleanup;
        }
        
        if (ctx->machine_needs_byte_swap) {
            rec_type = byteswap4(rec_type);
        }
        
        switch (rec_type) {
            case SAV_RECORD_TYPE_VARIABLE:
                retval = sav_read_variable_record(fd, ctx);
                if (retval)
                    goto cleanup;
                break;
            case SAV_RECORD_TYPE_VALUE_LABEL:
                retval = sav_read_value_label_record(fd, ctx, value_label_cb, user_ctx);
                if (retval)
                    goto cleanup;
                break;
            case SAV_RECORD_TYPE_DOCUMENT:
                retval = sav_read_document_record(fd, ctx);
                if (retval)
                    goto cleanup;
                break;
            case SAV_RECORD_TYPE_DICT_TERMINATION:
                retval = sav_read_dictionary_termination_record(fd, ctx);
                if (retval)
                    goto cleanup;
                done = 1;
                break;
            case SAV_RECORD_TYPE_HAS_DATA:
                if (read(fd, extra_info, sizeof(extra_info)) < sizeof(extra_info)) {
                    retval = READSTAT_ERROR_READ;
                    goto cleanup;
                }
                if (ctx->machine_needs_byte_swap) {
                    for (i=0; i<3; i++)
                        extra_info[i] = byteswap4(extra_info[i]);
                }
                int subtype = extra_info[0];
                int size = extra_info[1];
                int count = extra_info[2];
                data_len = size * count;
                if (data_buf_capacity < data_len) {
                    if ((data_buf = realloc(data_buf, data_buf_capacity = data_len)) == NULL) {
                        retval = READSTAT_ERROR_MALLOC;
                        goto cleanup;
                    }
                }
                if (read(fd, data_buf, data_len) < data_len) {
                    retval = READSTAT_ERROR_PARSE;
                    goto cleanup;
                }
                
                switch (subtype) {
                    case SAV_RECORD_SUBTYPE_INTEGER_INFO:
                        retval = sav_parse_machine_integer_info_record(data_buf, data_len, ctx);
                        if (retval != READSTAT_OK)
                            goto cleanup;
                        break;
                    case SAV_RECORD_SUBTYPE_FP_INFO:
                        retval = sav_parse_machine_floating_point_record(data_buf, ctx);
                        if (retval != READSTAT_OK)
                            goto cleanup;
                        break;
                    case SAV_RECORD_SUBTYPE_VAR_DISPLAY:
                        retval = sav_parse_variable_display_parameter_record(data_buf, ctx);
                        if (retval != READSTAT_OK)
                            goto cleanup;
                        break;
                    case SAV_RECORD_SUBTYPE_LONG_VAR_NAME:
                        retval = sav_parse_long_variable_names_record(data_buf, count, ctx);
                        if (retval != READSTAT_OK)
                            goto cleanup;
                        break;
                    case SAV_RECORD_SUBTYPE_VERY_LONG_STR:
                        retval = sav_parse_very_long_string_record(data_buf, count, ctx);
                        if (retval != READSTAT_OK)
                            goto cleanup;
                        break;
                    default: /* misc. info */
                        break;
                }
                break;
            default:
                retval = READSTAT_ERROR_PARSE;
                goto cleanup;
                break;
        }
        if (done)
            break;
    }
    
    int i;
    for (i=0; i<ctx->var_index;) {
        sav_varinfo_t *info = &ctx->varinfo[i];
        if (info->string_length) {
            info->n_segments = (info->string_length + 251) / 252;
        } else {
            info->n_segments = 1;
        }
        info->index = ctx->var_count++;
        i += info->n_segments;
    }
    
    if (info_cb) {
        if (info_cb(ctx->record_count, ctx->var_count, user_ctx)) {
            retval = READSTAT_ERROR_USER_ABORT;
            goto cleanup;
        }
    }
    if (variable_cb) {
        for (i=0; i<ctx->var_index;) {
            char label_name_buf[256];
            sav_varinfo_t *info = &ctx->varinfo[i];
            sprintf(label_name_buf, SAV_LABEL_NAME_PREFIX "%d", info->labels_index);

            char *format = NULL;
            char buf[80];
            if (spss_format_is_date(info->print_format.type)) {
                const char *fmt = spss_format(info->print_format.type);
                sprintf(buf, "%%ts%s", fmt ? fmt : "");
                format = buf;
            }

            int cb_retval = variable_cb(info->index, info->longname, format, info->label, 
                                        info->labels_index == -1 ? NULL : label_name_buf,
                                        info->type, 0, user_ctx);
            if (cb_retval) {
                retval = READSTAT_ERROR_USER_ABORT;
                goto cleanup;
            }
            i += info->n_segments;
        }
    }
    
    if (value_cb) {
        retval = sav_read_data(fd, ctx, value_cb, user_ctx);
    }
    
cleanup:
    if (fd > 0)
        readstat_close(fd);
    if (data_buf)
        free(data_buf);
    if (ctx)
        sav_ctx_free(ctx);
    
    return retval;
}
