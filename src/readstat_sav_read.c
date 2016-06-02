
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <stdint.h>
#include <math.h>
#include <float.h>
#include <time.h>

#include "readstat_sav.h"
#include "readstat_sav_parse.h"
#include "readstat_convert.h"

#define DATA_BUFFER_SIZE            65536

/* Others defined in table below */

/* See http://msdn.microsoft.com/en-us/library/dd317756(VS.85).aspx */
static readstat_charset_entry_t _charset_table[] = { 
    { .code = 1,     .name = "EBCDIC-US" },
    { .code = 2,     .name = "WINDOWS-1252" }, /* supposed to be ASCII, but some files are miscoded */
    { .code = 3,     .name = "WINDOWS-1252" },
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

#define SAV_EIGHT_SPACES              "        "
#define SAV_LABEL_NAME_PREFIX         "labels"

typedef struct value_label_s {
    char          value[8];
    unsigned char label_len;
    char          label[256*4+1];
} value_label_t;

static readstat_error_t sav_update_progress(sav_ctx_t *ctx);
static readstat_error_t sav_read_data(sav_ctx_t *ctx);
static readstat_error_t sav_read_compressed_data(size_t longest_string, 
        sav_ctx_t *ctx, int *out_rows);
static readstat_error_t sav_read_uncompressed_data(size_t longest_string, 
        sav_ctx_t *ctx, int *out_rows);

static readstat_error_t sav_skip_variable_record(sav_ctx_t *ctx);
static readstat_error_t sav_read_variable_record(sav_ctx_t *ctx);

static readstat_error_t sav_skip_document_record(sav_ctx_t *ctx);
static readstat_error_t sav_read_document_record(sav_ctx_t *ctx);

static readstat_error_t sav_skip_value_label_record(sav_ctx_t *ctx);
static readstat_error_t sav_read_value_label_record(sav_ctx_t *ctx);

static readstat_error_t sav_read_dictionary_termination_record(sav_ctx_t *ctx);

static readstat_error_t sav_parse_machine_floating_point_record(void *data, sav_ctx_t *ctx);
static readstat_error_t sav_parse_variable_display_parameter_record(void *data, long count, sav_ctx_t *ctx);
static readstat_error_t sav_parse_machine_integer_info_record(void *data, size_t data_len, sav_ctx_t *ctx);
static readstat_error_t sav_parse_long_value_labels_record(void *data, size_t data_len, sav_ctx_t *ctx);

static readstat_error_t sav_update_progress(sav_ctx_t *ctx) {
    readstat_io_t *io = ctx->io;
    return io->update(ctx->file_size, ctx->progress_handler, ctx->user_ctx, io->io_ctx);
}

static readstat_error_t sav_skip_variable_record(sav_ctx_t *ctx) {
    sav_variable_record_t variable;
    readstat_error_t retval = READSTAT_OK;
    readstat_io_t *io = ctx->io;
    if (io->read(&variable, sizeof(sav_variable_record_t), io->io_ctx) < sizeof(sav_variable_record_t)) {
        retval = READSTAT_ERROR_READ;
        goto cleanup;
    }
    if (variable.has_var_label) {
        int32_t label_len;
        if (io->read(&label_len, sizeof(int32_t), io->io_ctx) < sizeof(int32_t)) {
            retval = READSTAT_ERROR_READ;
            goto cleanup;
        }
        label_len = ctx->machine_needs_byte_swap ? byteswap4(label_len) : label_len;
        int32_t label_capacity = (label_len + 3) / 4 * 4;
        if (io->seek(label_capacity, READSTAT_SEEK_CUR, io->io_ctx) == -1) {
            retval = READSTAT_ERROR_SEEK;
            goto cleanup;
        }
    }
    if (variable.n_missing_values) {
        int n_missing_values = ctx->machine_needs_byte_swap ? byteswap4(variable.n_missing_values) : variable.n_missing_values;
        if (io->seek(abs(n_missing_values) * sizeof(double), READSTAT_SEEK_CUR, io->io_ctx) == -1) {
            retval = READSTAT_ERROR_SEEK;
            goto cleanup;
        }
    }
cleanup:
    return retval;
}

static readstat_error_t sav_read_variable_record(sav_ctx_t *ctx) {
    readstat_io_t *io = ctx->io;
    sav_variable_record_t variable;
    readstat_error_t retval = READSTAT_OK;
    if (ctx->var_index == ctx->varinfo_capacity) {
        if ((ctx->varinfo = realloc(ctx->varinfo, (ctx->varinfo_capacity *= 2) * sizeof(spss_varinfo_t))) == NULL) {
            retval = READSTAT_ERROR_MALLOC;
            goto cleanup;
        }
    }
    if (io->read(&variable, sizeof(sav_variable_record_t), io->io_ctx) < sizeof(sav_variable_record_t)) {
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
        spss_varinfo_t *prev = &ctx->varinfo[ctx->var_index-1];
        prev->width++;
        return 0;
    }
    if (type > 0) {
        dta_type = READSTAT_TYPE_STRING;
        // len = type;
    }
    spss_varinfo_t *info = &ctx->varinfo[ctx->var_index];
    memset(info, 0, sizeof(spss_varinfo_t));
    info->width = 1;
    info->n_segments = 1;
    info->index = ctx->var_index;
    info->offset = ctx->var_offset;
    info->type = dta_type;

    retval = readstat_convert(info->name, sizeof(info->name),
            variable.name, sizeof(variable.name), ctx->converter);
    if (retval != READSTAT_OK)
        goto cleanup;

    retval = readstat_convert(info->longname, sizeof(info->longname), 
            variable.name, sizeof(variable.name), ctx->converter);
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
        if (io->read(&label_len, sizeof(int32_t), io->io_ctx) < sizeof(int32_t)) {
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
        if (io->read(label_buf, label_capacity, io->io_ctx) < label_capacity) {
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
        if (io->read(info->missing_values, info->n_missing_values * sizeof(double), io->io_ctx) < info->n_missing_values * sizeof(double)) {
            retval = READSTAT_ERROR_READ;
            goto cleanup;
        }
        if (ctx->machine_needs_byte_swap) {
            for (i=0; i<info->n_missing_values; i++) {
                info->missing_values[i] = byteswap_double(info->missing_values[i]);
            }
        }
        info->missingness = spss_missingness_for_info(info);
    }
    
    ctx->var_index++;
    ctx->var_offset++;
    
cleanup:
    
    return retval;
}

static readstat_error_t sav_skip_value_label_record(sav_ctx_t *ctx) {
    int32_t label_count;
    int32_t rec_type;
    int32_t var_count;
    readstat_error_t retval = READSTAT_OK;
    readstat_io_t *io = ctx->io;
    if (io->read(&label_count, sizeof(int32_t), io->io_ctx) < sizeof(int32_t)) {
        retval = READSTAT_ERROR_READ;
        goto cleanup;
    }
    if (ctx->machine_needs_byte_swap)
        label_count = byteswap4(label_count);
    int i;
    for (i=0; i<label_count; i++) {
        value_label_t vlabel;
        if (io->read(&vlabel, 9, io->io_ctx) < 9) {
            retval = READSTAT_ERROR_READ;
            goto cleanup;
        }
        size_t label_len = (vlabel.label_len + 8) / 8 * 8 - 1;
        if (io->seek(label_len, READSTAT_SEEK_CUR, io->io_ctx) == -1) {
            retval = READSTAT_ERROR_SEEK;
            goto cleanup;
        }
    }

    if (io->read(&rec_type, sizeof(int32_t), io->io_ctx) < sizeof(int32_t)) {
        retval = READSTAT_ERROR_READ;
        goto cleanup;
    }
    if (ctx->machine_needs_byte_swap)
        rec_type = byteswap4(rec_type);
    
    if (rec_type != 4) {
        retval = READSTAT_ERROR_PARSE;
        goto cleanup;
    }
    if (io->read(&var_count, sizeof(int32_t), io->io_ctx) < sizeof(int32_t)) {
        retval = READSTAT_ERROR_READ;
        goto cleanup;
    }
    if (ctx->machine_needs_byte_swap)
        var_count = byteswap4(var_count);
    
    if (io->seek(var_count * sizeof(int32_t), READSTAT_SEEK_CUR, io->io_ctx) == -1) {
        retval = READSTAT_ERROR_SEEK;
        goto cleanup;
    }

cleanup:
    return retval;
}

static readstat_error_t sav_submit_value_labels(value_label_t *value_labels, int32_t label_count, 
        readstat_types_t value_type, readstat_missingness_t *missingness, sav_ctx_t *ctx) {
    char label_name_buf[256];
    readstat_error_t retval = READSTAT_OK;
    int32_t i;

    snprintf(label_name_buf, sizeof(label_name_buf), SAV_LABEL_NAME_PREFIX "%d", ctx->value_labels_count);

    for (i=0; i<label_count; i++) {
        value_label_t *vlabel = &value_labels[i];
        readstat_value_t value = { .type = value_type };
        if (value_type == READSTAT_TYPE_DOUBLE) {
            double val_d = 0.0;
            memcpy(&val_d, vlabel->value, 8);
            if (ctx->machine_needs_byte_swap)
                val_d = byteswap_double(val_d);

            value.v.double_value = val_d;
            spss_tag_missing_double(&value, missingness);
        } else {
            char unpadded_val[8*4+1];
            retval = readstat_convert(unpadded_val, sizeof(unpadded_val), vlabel->value, 8, ctx->converter);
            if (retval != READSTAT_OK)
                break;

            value.v.string_value = unpadded_val;
        }
        ctx->value_label_handler(label_name_buf, value, vlabel->label, ctx->user_ctx);
    }
    return retval;
}

static readstat_error_t sav_read_value_label_record(sav_ctx_t *ctx) {
    int32_t label_count;
    readstat_error_t retval = READSTAT_OK;
    readstat_io_t *io = ctx->io;
    int32_t *vars = NULL;
    int32_t rec_type;
    int32_t var_count;
    readstat_types_t value_type = READSTAT_TYPE_STRING;
    char label_buf[256];
    value_label_t *value_labels = NULL;
    readstat_missingness_t *missingness = NULL;

    if (io->read(&label_count, sizeof(int32_t), io->io_ctx) < sizeof(int32_t)) {
        retval = READSTAT_ERROR_READ;
        goto cleanup;
    }
    if (ctx->machine_needs_byte_swap)
        label_count = byteswap4(label_count);
    
    if ((value_labels = malloc(label_count * sizeof(value_label_t))) == NULL) {
        retval = READSTAT_ERROR_MALLOC;
        goto cleanup;
    }
    
    int i;
    for (i=0; i<label_count; i++) {
        value_label_t *vlabel = &value_labels[i];
        if (io->read(vlabel, 9, io->io_ctx) < 9) {
            retval = READSTAT_ERROR_READ;
            goto cleanup;
        }
        size_t label_len = (vlabel->label_len + 8) / 8 * 8 - 1;
        if (io->read(label_buf, label_len, io->io_ctx) < label_len) {
            retval = READSTAT_ERROR_READ;
            goto cleanup;
        }
        retval = readstat_convert(vlabel->label, sizeof(vlabel->label), label_buf, label_len, ctx->converter);
        if (retval != READSTAT_OK)
            goto cleanup;
    }

    if (io->read(&rec_type, sizeof(int32_t), io->io_ctx) < sizeof(int32_t)) {
        retval = READSTAT_ERROR_READ;
        goto cleanup;
    }
    if (ctx->machine_needs_byte_swap)
        rec_type = byteswap4(rec_type);
    
    if (rec_type != 4) {
        retval = READSTAT_ERROR_PARSE;
        goto cleanup;
    }
    if (io->read(&var_count, sizeof(int32_t), io->io_ctx) < sizeof(int32_t)) {
        retval = READSTAT_ERROR_READ;
        goto cleanup;
    }
    if (ctx->machine_needs_byte_swap)
        var_count = byteswap4(var_count);
    
    if ((vars = malloc(var_count * sizeof(int32_t))) == NULL) {
        retval = READSTAT_ERROR_MALLOC;
        goto cleanup;
    }
    
    if (io->read(vars, var_count * sizeof(int32_t), io->io_ctx) < var_count * sizeof(int32_t)) {
        retval = READSTAT_ERROR_READ;
        goto cleanup;
    }
    for (i=0; i<var_count; i++) {
        int var_offset = vars[i]-1; // Why subtract 1????
        spss_varinfo_t *var = bsearch(&var_offset, ctx->varinfo, ctx->var_index, sizeof(spss_varinfo_t),
                &spss_varinfo_compare);
        if (var) {
            var->labels_index = ctx->value_labels_count;

            value_type = var->type;
            missingness = &var->missingness;
        }
    }
    if (ctx->value_label_handler) {
        sav_submit_value_labels(value_labels, label_count, value_type, missingness, ctx);
    }
    ctx->value_labels_count++;
cleanup:
    if (vars)
        free(vars);
    if (value_labels)
        free(value_labels);
    
    return retval;
}

static readstat_error_t sav_skip_document_record(sav_ctx_t *ctx) {
    int32_t n_lines;
    readstat_error_t retval = READSTAT_OK;
    readstat_io_t *io = ctx->io;
    if (io->read(&n_lines, sizeof(int32_t), io->io_ctx) < sizeof(int32_t)) {
        retval = READSTAT_ERROR_READ;
        goto cleanup;
    }
    if (ctx->machine_needs_byte_swap)
        n_lines = byteswap4(n_lines);
    if (io->seek(n_lines * 80, READSTAT_SEEK_CUR, io->io_ctx) == -1) {
        retval = READSTAT_ERROR_SEEK;
        goto cleanup;
    }
    
cleanup:
    return retval;
}

static readstat_error_t sav_read_document_record(sav_ctx_t *ctx) {
    return sav_skip_document_record(ctx);
}

static readstat_error_t sav_read_dictionary_termination_record(sav_ctx_t *ctx) {
    int32_t filler;
    readstat_error_t retval = READSTAT_OK;
    readstat_io_t *io = ctx->io;
    if (io->read(&filler, sizeof(int32_t), io->io_ctx) < sizeof(int32_t)) {
        retval = READSTAT_ERROR_READ;
    }
    return retval;
}

static readstat_error_t sav_read_data(sav_ctx_t *ctx) {
    readstat_error_t retval = READSTAT_OK;
    int longest_string = 256;
    int rows = 0;
    int i;

    for (i=0; i<ctx->var_count; i++) {
        spss_varinfo_t *info = &ctx->varinfo[i];
        if (info->string_length > longest_string) {
            longest_string = info->string_length;
        }
    }
    if (ctx->data_is_compressed) {
        retval = sav_read_compressed_data(longest_string, ctx, &rows);
    } else {
        retval = sav_read_uncompressed_data(longest_string, ctx, &rows);
    }
    if (retval != READSTAT_OK)
        goto done;

    if (ctx->record_count != -1 && rows != ctx->record_count) {
        retval = READSTAT_ERROR_PARSE;
    }

done:
    return retval;
}

static readstat_error_t sav_read_uncompressed_data(size_t longest_string, 
        sav_ctx_t *ctx, int *out_rows) {
    readstat_error_t retval = READSTAT_OK;
    readstat_io_t *io = ctx->io;
    int segment_offset = 0;
    int row = 0, var_index = 0, col = 0;
    double fp_value;
    int offset = 0;
    off_t data_offset = 0;
    int raw_str_used = 0;
    char *raw_str_value = NULL;
    char *utf8_str_value = NULL;
    size_t utf8_str_value_len = 0;
    unsigned char buffer[DATA_BUFFER_SIZE];
    int buffer_used = 0;

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
        if (data_offset >= buffer_used) {
            retval = sav_update_progress(ctx);
            if (retval != READSTAT_OK)
                goto done;

            if ((buffer_used = io->read(buffer, sizeof(buffer), io->io_ctx)) == -1 ||
                buffer_used == 0 || (buffer_used % 8) != 0)
                goto done;

            data_offset = 0;
        }

        spss_varinfo_t *col_info = &ctx->varinfo[col];
        spss_varinfo_t *var_info = &ctx->varinfo[var_index];
        readstat_value_t value = { .type = var_info->type };
        if (offset > 31) {
            retval = READSTAT_ERROR_PARSE;
            goto done;
        }
        if (var_info->type == READSTAT_TYPE_STRING) {
            if (raw_str_used + 8 <= longest_string) {
                memcpy(raw_str_value + raw_str_used, &buffer[data_offset], 8);
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
                    value.v.string_value = utf8_str_value;
                    if (ctx->value_handler(row, var_info->index, value, ctx->user_ctx)) {
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
            memcpy(&fp_value, &buffer[data_offset], 8);
            if (ctx->machine_needs_byte_swap) {
                fp_value = byteswap_double(fp_value);
            }
            value.v.double_value = fp_value;
            spss_tag_missing_double(&value, &var_info->missingness);
            if (ctx->value_handler(row, var_info->index, value, ctx->user_ctx)) {
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
        data_offset += 8;
    }
done:
    if (retval == READSTAT_OK) {
        if (out_rows)
            *out_rows = row;
    }
    if (raw_str_value)
        free(raw_str_value);
    if (utf8_str_value)
        free(utf8_str_value);

    return retval;
}

static readstat_error_t sav_read_compressed_data(size_t longest_string,
        sav_ctx_t *ctx, int *out_rows) {
    readstat_error_t retval = READSTAT_OK;
    readstat_io_t *io = ctx->io;
    unsigned char chunk[8];
    int offset = 0;
    int segment_offset = 0;
    int row = 0, var_index = 0, col = 0;
    int i;
    double fp_value;
    off_t data_offset = 0;
    int raw_str_used = 0;
    char *raw_str_value = NULL;
    char *utf8_str_value = NULL;
    size_t utf8_str_value_len = 0;
    unsigned char buffer[DATA_BUFFER_SIZE];
    int buffer_used = 0;

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
        if (data_offset >= buffer_used) {
            retval = sav_update_progress(ctx);
            if (retval != READSTAT_OK)
                goto done;

            if ((buffer_used = io->read(buffer, sizeof(buffer), io->io_ctx)) == -1 ||
                buffer_used == 0 || (buffer_used % 8) != 0)
                goto done;

            data_offset = 0;
        }

        memcpy(chunk, &buffer[data_offset], 8);
        data_offset += 8;
        
        spss_varinfo_t *col_info = &ctx->varinfo[col];
        spss_varinfo_t *var_info = &ctx->varinfo[var_index];
        for (i=0; i<8; i++) {
            if (offset > 31) {
                retval = READSTAT_ERROR_PARSE;
                goto done;
            }
            col_info = &ctx->varinfo[col];
            var_info = &ctx->varinfo[var_index];
            readstat_value_t value = { .type = var_info->type };
            switch (chunk[i]) {
                case 0:
                    break;
                case 252:
                    goto done;
                case 253:
                    if (data_offset >= buffer_used) {
                        if ((buffer_used = io->read(buffer, sizeof(buffer), io->io_ctx)) == -1 ||
                            buffer_used == 0 || (buffer_used % 8) != 0)
                            goto done;

                        data_offset = 0;
                    }
                    if (var_info->type == READSTAT_TYPE_STRING) {
                        if (raw_str_used + 8 <= longest_string) {
                            memcpy(raw_str_value + raw_str_used, &buffer[data_offset], 8);
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
                                value.v.string_value = utf8_str_value;
                                if (ctx->value_handler(row, var_info->index, value, ctx->user_ctx)) {
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
                        memcpy(&fp_value, &buffer[data_offset], 8);
                        if (ctx->machine_needs_byte_swap) {
                            fp_value = byteswap_double(fp_value);
                        }
                        value.v.double_value = fp_value;
                        spss_tag_missing_double(&value, &var_info->missingness);
                        if (ctx->value_handler(row, var_info->index, value, ctx->user_ctx)) {
                            retval = READSTAT_ERROR_USER_ABORT;
                            goto done;
                        }
                        var_index += var_info->n_segments;
                        col++;
                    }
                    data_offset += 8;
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
                                value.v.string_value = utf8_str_value;
                                if (ctx->value_handler(row, var_info->index, value, ctx->user_ctx)) {
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
                    value.v.double_value = NAN;
                    value.is_system_missing = 1;
                    if (ctx->value_handler(row, var_info->index, value, ctx->user_ctx)) {
                        retval = READSTAT_ERROR_USER_ABORT;
                        goto done;
                    }
                    var_index += var_info->n_segments;
                    col++;
                    break;
                default:
                    value.v.double_value = chunk[i] - 100.0;
                    spss_tag_missing_double(&value, &var_info->missingness);
                    if (ctx->value_handler(row, var_info->index, value, ctx->user_ctx)) {
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
    }
done:
    if (retval == READSTAT_OK) {
        if (out_rows)
            *out_rows = row;
    }
    if (raw_str_value)
        free(raw_str_value);
    if (utf8_str_value)
        free(utf8_str_value);

    return retval;
}

static readstat_error_t sav_parse_machine_integer_info_record(void *data, size_t data_len, sav_ctx_t *ctx) {
    if (data_len != 32)
        return READSTAT_ERROR_PARSE;

    const char *src_charset = NULL;
    const char *dst_charset = ctx->output_encoding;
    sav_machine_integer_info_record_t record;
    memcpy(&record, data, data_len);
    if (ctx->machine_needs_byte_swap) {
        record.character_code = byteswap4(record.character_code);
    }
    if (ctx->input_encoding) {
        src_charset = ctx->input_encoding;
    } else if (record.character_code == SAV_CHARSET_UTF8) {
        /* do nothing */
    } else {
        int i;
        for (i=0; i<sizeof(_charset_table)/sizeof(_charset_table[0]); i++) {
            if (record.character_code  == _charset_table[i].code) {
                src_charset = _charset_table[i].name;
                break;
            }
        }
        if (src_charset == NULL) {
            if (ctx->error_handler) {
                char error_buf[1024];
                snprintf(error_buf, sizeof(error_buf), "Unsupported character set: %d\n", record.character_code);
                ctx->error_handler(error_buf, ctx->user_ctx);
            }
            return READSTAT_ERROR_UNSUPPORTED_CHARSET;
        }
    }
    if (src_charset && dst_charset && strcmp(src_charset, dst_charset) != 0) {
        iconv_t converter = iconv_open(dst_charset, src_charset);
        if (converter == (iconv_t)-1) {
            return READSTAT_ERROR_UNSUPPORTED_CHARSET;
        }
        ctx->converter = converter;
    }
    return READSTAT_OK;
}

static readstat_error_t sav_parse_machine_floating_point_record(void *data, sav_ctx_t *ctx) {
    return READSTAT_OK;
}

static readstat_error_t sav_parse_variable_display_parameter_record(void *data, long count, sav_ctx_t *ctx) {
    int i;
    char *data_ptr = data;
    long var_count = 0;
    for (i=0; i<ctx->var_index;) {
        spss_varinfo_t *info = &ctx->varinfo[i];
        i += info->n_segments;
        var_count++;
    }
    if (count != 2 * var_count && count != 3 * var_count) {
        return READSTAT_ERROR_PARSE;
    }
    long field_count = count / var_count;
    for (i=0; i<ctx->var_index;) {
        int32_t measure = 0, alignment = 0;
        spss_varinfo_t *info = &ctx->varinfo[i];

        memcpy(&measure, data_ptr, sizeof(int32_t));
        info->measure = spss_measure_to_readstat_measure(measure);
        data_ptr += (field_count - 1) * sizeof(int32_t);

        memcpy(&alignment, data_ptr, sizeof(int32_t));
        info->alignment = spss_alignment_to_readstat_alignment(alignment);
        data_ptr += sizeof(int32_t);

        i += info->n_segments;
    }
    return READSTAT_OK;
}

static readstat_error_t sav_parse_long_value_labels_record(void *data, size_t data_len, sav_ctx_t *ctx) {
    if (!ctx->value_label_handler)
        return READSTAT_OK;

    readstat_error_t retval = READSTAT_OK;
    int32_t label_name_len = 0;
    int32_t label_count = 0;
    int32_t i = 0;
    char *data_ptr = data;
    char *data_end = data_ptr + data_len;
    char var_name_buf[256*4+1];
    char label_name_buf[256];
    char *value_buffer = NULL;
    char *label_buffer = NULL;
    
    memset(label_name_buf, '\0', sizeof(label_name_buf));

    if (data_ptr + sizeof(int32_t) > data_end) {
        retval = READSTAT_ERROR_PARSE;
        goto cleanup;
    }

    memcpy(&label_name_len, data_ptr, sizeof(int32_t));
    if (ctx->machine_needs_byte_swap)
        label_name_len = byteswap4(label_name_len);

    data_ptr += sizeof(int32_t);

    if (data_ptr + label_name_len > data_end) {
        retval = READSTAT_ERROR_PARSE;
        goto cleanup;
    }

    retval = readstat_convert(var_name_buf, sizeof(var_name_buf), data_ptr, label_name_len, ctx->converter);
    if (retval != READSTAT_OK)
        goto cleanup;

    data_ptr += label_name_len;

    for (i=0; i<ctx->var_index;) {
        spss_varinfo_t *info = &ctx->varinfo[i];
        if (strcmp(var_name_buf, info->longname) == 0) {
            info->labels_index = ctx->value_labels_count++;
            snprintf(label_name_buf, sizeof(label_name_buf),
                     SAV_LABEL_NAME_PREFIX "%d", info->labels_index);
            break;
        }
        i += info->n_segments;
    }
    
    if (label_name_buf[0] == '\0') {
        retval = READSTAT_ERROR_PARSE;
        goto cleanup;
    }

    data_ptr += sizeof(int32_t);

    if (data_ptr + sizeof(int32_t) > data_end) {
        retval = READSTAT_ERROR_PARSE;
        goto cleanup;
    }

    memcpy(&label_count, data_ptr, sizeof(int32_t));
    if (ctx->machine_needs_byte_swap)
        label_count = byteswap4(label_count);

    data_ptr += sizeof(int32_t);

    for (i=0; i<label_count; i++) {
        int32_t value_len = 0, label_len = 0;
        int32_t value_buffer_len = 0, label_buffer_len = 0;

        if (data_ptr + sizeof(int32_t) > data_end) {
            retval = READSTAT_ERROR_PARSE;
            goto cleanup;
        }

        memcpy(&value_len, data_ptr, sizeof(int32_t));
        if (ctx->machine_needs_byte_swap)
            value_len = byteswap4(value_len);

        data_ptr += sizeof(int32_t);

        value_buffer_len = value_len*4+1;
        value_buffer = realloc(value_buffer, value_buffer_len);

        if (data_ptr + value_len > data_end) {
            retval = READSTAT_ERROR_PARSE;
            goto cleanup;
        }

        retval = readstat_convert(value_buffer, value_buffer_len, data_ptr, value_len, ctx->converter);
        if (retval != READSTAT_OK)
            goto cleanup;

        data_ptr += value_len;

        if (data_ptr + sizeof(int32_t) > data_end) {
            retval = READSTAT_ERROR_PARSE;
            goto cleanup;
        }

        memcpy(&label_len, data_ptr, sizeof(int32_t));
        if (ctx->machine_needs_byte_swap)
            label_len = byteswap4(label_len);

        data_ptr += sizeof(int32_t);

        label_buffer_len = label_len*4+1;
        label_buffer = realloc(label_buffer, label_buffer_len);

        if (data_ptr + label_len > data_end) {
            retval = READSTAT_ERROR_PARSE;
            goto cleanup;
        }

        retval = readstat_convert(label_buffer, label_buffer_len, data_ptr, label_len, ctx->converter);
        if (retval != READSTAT_OK)
            goto cleanup;

        data_ptr += label_len;

        readstat_value_t value = { .type = READSTAT_TYPE_STRING };
        value.v.string_value = value_buffer;

        ctx->value_label_handler(label_name_buf, value, label_buffer, ctx->user_ctx);
    }

cleanup:
    if (value_buffer)
        free(value_buffer);
    if (label_buffer)
        free(label_buffer);
    return retval;
}

static readstat_error_t sav_parse_records_pass1(sav_ctx_t *ctx) {
    char data_buf[4096];
    readstat_error_t retval = READSTAT_OK;
    readstat_io_t *io = ctx->io;
    while (1) {
        int32_t rec_type;
        int32_t extra_info[3];
        size_t data_len = 0;
        int i;
        int done = 0;
        if (io->read(&rec_type, sizeof(int32_t), io->io_ctx) < sizeof(int32_t)) {
            retval = READSTAT_ERROR_READ;
            goto cleanup;
        }
        
        if (ctx->machine_needs_byte_swap) {
            rec_type = byteswap4(rec_type);
        }
        
        switch (rec_type) {
            case SAV_RECORD_TYPE_VARIABLE:
                retval = sav_skip_variable_record(ctx);
                if (retval)
                    goto cleanup;
                break;
            case SAV_RECORD_TYPE_VALUE_LABEL:
                retval = sav_skip_value_label_record(ctx);
                if (retval)
                    goto cleanup;
                break;
            case SAV_RECORD_TYPE_DOCUMENT:
                retval = sav_skip_document_record(ctx);
                if (retval)
                    goto cleanup;
                break;
            case SAV_RECORD_TYPE_DICT_TERMINATION:
                done = 1;
                break;
            case SAV_RECORD_TYPE_HAS_DATA:
                if (io->read(extra_info, sizeof(extra_info), io->io_ctx) < sizeof(extra_info)) {
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
                if (subtype == SAV_RECORD_SUBTYPE_INTEGER_INFO) {
                    if (data_len > sizeof(data_buf)) {
                        retval = READSTAT_ERROR_PARSE;
                        goto cleanup;
                    }
                    if (io->read(data_buf, data_len, io->io_ctx) < data_len) {
                        retval = READSTAT_ERROR_PARSE;
                        goto cleanup;
                    }
                    retval = sav_parse_machine_integer_info_record(data_buf, data_len, ctx);
                    if (retval != READSTAT_OK)
                        goto cleanup;
                } else {
                    if (io->seek(data_len, READSTAT_SEEK_CUR, io->io_ctx) == -1) {
                        retval = READSTAT_ERROR_SEEK;
                        goto cleanup;
                    }
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
cleanup:
    return retval;
}

static readstat_error_t sav_parse_records_pass2(sav_ctx_t *ctx) {
    void *data_buf = NULL;
    size_t data_buf_capacity = 4096;
    readstat_error_t retval = READSTAT_OK;
    readstat_io_t *io = ctx->io;

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
        if (io->read(&rec_type, sizeof(int32_t), io->io_ctx) < sizeof(int32_t)) {
            retval = READSTAT_ERROR_READ;
            goto cleanup;
        }
        
        if (ctx->machine_needs_byte_swap) {
            rec_type = byteswap4(rec_type);
        }
        
        switch (rec_type) {
            case SAV_RECORD_TYPE_VARIABLE:
                retval = sav_read_variable_record(ctx);
                if (retval)
                    goto cleanup;
                break;
            case SAV_RECORD_TYPE_VALUE_LABEL:
                retval = sav_read_value_label_record(ctx);
                if (retval)
                    goto cleanup;
                break;
            case SAV_RECORD_TYPE_DOCUMENT:
                retval = sav_read_document_record(ctx);
                if (retval)
                    goto cleanup;
                break;
            case SAV_RECORD_TYPE_DICT_TERMINATION:
                retval = sav_read_dictionary_termination_record(ctx);
                if (retval)
                    goto cleanup;
                done = 1;
                break;
            case SAV_RECORD_TYPE_HAS_DATA:
                if (io->read(extra_info, sizeof(extra_info), io->io_ctx) < sizeof(extra_info)) {
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
                if (io->read(data_buf, data_len, io->io_ctx) < data_len) {
                    retval = READSTAT_ERROR_PARSE;
                    goto cleanup;
                }
                
                switch (subtype) {
                    case SAV_RECORD_SUBTYPE_INTEGER_INFO:
                        /* parsed in pass 1 */
                        break;
                    case SAV_RECORD_SUBTYPE_FP_INFO:
                        retval = sav_parse_machine_floating_point_record(data_buf, ctx);
                        if (retval != READSTAT_OK)
                            goto cleanup;
                        break;
                    case SAV_RECORD_SUBTYPE_VAR_DISPLAY:
                        retval = sav_parse_variable_display_parameter_record(data_buf, count, ctx);
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
                    case SAV_RECORD_SUBTYPE_LONG_VALUE_LABELS:
                        retval = sav_parse_long_value_labels_record(data_buf, count, ctx);
                        if (retval != READSTAT_OK)
                            goto cleanup;
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
cleanup:
    if (data_buf)
        free(data_buf);
    return retval;
}

static void sav_set_n_segments_and_var_count(sav_ctx_t *ctx) {
    int i;
    for (i=0; i<ctx->var_index;) {
        spss_varinfo_t *info = &ctx->varinfo[i];
        if (info->string_length) {
            info->n_segments = (info->string_length + 251) / 252;
        }
        info->index = ctx->var_count++;
        i += info->n_segments;
    }
}

static readstat_error_t sav_handle_variables(readstat_parser_t *parser, sav_ctx_t *ctx) {
    int i;
    readstat_error_t retval = READSTAT_OK;

    if (!parser->variable_handler)
        return retval;

    for (i=0; i<ctx->var_index;) {
        char label_name_buf[256];
        spss_varinfo_t *info = &ctx->varinfo[i];
        readstat_variable_t *variable = spss_init_variable_for_info(info);

        snprintf(label_name_buf, sizeof(label_name_buf), SAV_LABEL_NAME_PREFIX "%d", info->labels_index);

        int cb_retval = parser->variable_handler(info->index, variable,
                info->labels_index == -1 ? NULL : label_name_buf,
                ctx->user_ctx);

        spss_free_variable(variable);

        if (cb_retval) {
            retval = READSTAT_ERROR_USER_ABORT;
            goto cleanup;
        }
        i += info->n_segments;
    }
cleanup:
    return retval;
}

static readstat_error_t sav_handle_fweight(readstat_parser_t *parser, sav_ctx_t *ctx) {
    readstat_error_t retval = READSTAT_OK;
    int i;
    if (parser->fweight_handler && ctx->fweight_index) {
        for (i=0; i<ctx->var_index;) {
            spss_varinfo_t *info = &ctx->varinfo[i];
            if (info->offset == ctx->fweight_index - 1) {
                if (parser->fweight_handler(info->index, ctx->user_ctx)) {
                    retval = READSTAT_ERROR_USER_ABORT;
                    goto cleanup;
                }
                break;
            }
            i += info->n_segments;
        }
    }
cleanup:
    return retval;
}

readstat_error_t readstat_parse_sav(readstat_parser_t *parser, const char *path, void *user_ctx) {
    readstat_error_t retval = READSTAT_OK;
    readstat_io_t *io = parser->io;
    sav_file_header_record_t header;
    sav_ctx_t *ctx = NULL;
    size_t file_size = 0;
    
    if (io->open(path, io->io_ctx) == -1) {
        return READSTAT_ERROR_OPEN;
    }

    file_size = io->seek(0, READSTAT_SEEK_END, io->io_ctx);
    if (file_size == -1) {
        retval = READSTAT_ERROR_SEEK;
        goto cleanup;
    }

    if (io->seek(0, READSTAT_SEEK_SET, io->io_ctx) == -1) {
        retval = READSTAT_ERROR_SEEK;
        goto cleanup;
    }

    if (io->read(&header, sizeof(sav_file_header_record_t), io->io_ctx) < sizeof(sav_file_header_record_t)) {
        retval = READSTAT_ERROR_READ;
        goto cleanup;
    }
    
    ctx = sav_ctx_init(&header, io);
    if (ctx == NULL) {
        retval = READSTAT_ERROR_PARSE;
        goto cleanup;
    }

    ctx->progress_handler = parser->progress_handler;
    ctx->error_handler = parser->error_handler;
    ctx->value_handler = parser->value_handler;
    ctx->value_label_handler = parser->value_label_handler;
    ctx->input_encoding = parser->input_encoding;
    ctx->output_encoding = parser->output_encoding;
    ctx->user_ctx = user_ctx;
    ctx->file_size = file_size;
    
    if ((retval = sav_parse_records_pass1(ctx)) != READSTAT_OK)
        goto cleanup;
    
    if (io->seek(sizeof(sav_file_header_record_t), READSTAT_SEEK_SET, io->io_ctx) == -1) {
        retval = READSTAT_ERROR_SEEK;
        goto cleanup;
    }

    if ((retval = sav_update_progress(ctx)) != READSTAT_OK)
        goto cleanup;

    if ((retval = sav_parse_records_pass2(ctx)) != READSTAT_OK)
        goto cleanup;
 
    sav_set_n_segments_and_var_count(ctx);

    if (parser->info_handler) {
        if (parser->info_handler(ctx->record_count, ctx->var_count, ctx->user_ctx)) {
            retval = READSTAT_ERROR_USER_ABORT;
            goto cleanup;
        }
    }
    if ((retval = sav_handle_variables(parser, ctx)) != READSTAT_OK)
        goto cleanup;

    if ((retval = sav_handle_fweight(parser, ctx)) != READSTAT_OK)
        goto cleanup;

    if (ctx->value_handler) {
        retval = sav_read_data(ctx);
    }
    
cleanup:
    io->close(io->io_ctx);
    if (ctx)
        sav_ctx_free(ctx);
    
    return retval;
}
