//
//  readstat_por.c
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <stdint.h>
#include <inttypes.h>
#include <math.h>
#include <time.h>

#include "../readstat.h"
#include "../readstat_iconv.h"
#include "../readstat_convert.h"
#include "../CKHashTable.h"

#include "readstat_por_parse.h"
#include "readstat_spss.h"
#include "readstat_por.h"

#define POR_LINE_LENGTH         80
#define POR_LABEL_NAME_PREFIX   "labels"

static ssize_t read_bytes(por_ctx_t *ctx, void *dst, size_t len);
static readstat_error_t read_string(por_ctx_t *ctx, char *data, size_t len);

static readstat_error_t por_update_progress(por_ctx_t *ctx) {
    readstat_io_t *io = ctx->io;
    return io->update(ctx->file_size, ctx->progress_handler, ctx->user_ctx, io->io_ctx);
}

static ssize_t read_bytes(por_ctx_t *ctx, void *dst, size_t len) {
    char *dst_pos = (char *)dst;
    readstat_io_t *io = ctx->io;
    char byte;

    while (dst_pos < (char *)dst + len) {
        if (ctx->num_spaces) {
            *dst_pos++ = ctx->space;
            ctx->num_spaces--;
            continue;
        }
        ssize_t bytes_read = io->read(&byte, 1, io->io_ctx);
        if (bytes_read == 0) {
            break;
        }
        if (bytes_read == -1) {
            return -1;
        }
        if (byte == '\r' || byte == '\n') {
            if (byte == '\r') {
                bytes_read = io->read(&byte, 1, io->io_ctx);
                if (bytes_read == 0 || bytes_read == -1 || byte != '\n')
                    return -1;
            }
            ctx->num_spaces = POR_LINE_LENGTH - ctx->pos;
            ctx->pos = 0;
            continue;
        } else if (ctx->pos == POR_LINE_LENGTH) {
            return -1;
        }
        *dst_pos++ = byte;
        ctx->pos++;
    }
    
    return (int)(dst_pos - (char *)dst);
}

static uint16_t read_tag(por_ctx_t *ctx) {
    unsigned char tag;
    if (read_bytes(ctx, &tag, 1) != 1) {
        return -1;
    }
    return ctx->byte2unicode[tag];
}

static readstat_error_t read_double_with_peek(por_ctx_t *ctx, double *out_double, 
        unsigned char peek) {
    readstat_error_t retval = READSTAT_OK;
    double value = NAN;
    unsigned char buffer[100];
    char utf8_buffer[300];
    char error_buf[1024];
    int64_t len = 0;
    ssize_t bytes_read = 0;

    buffer[0] = peek;

    bytes_read = read_bytes(ctx, &buffer[1], 1);
    if (bytes_read != 1)
        return READSTAT_ERROR_PARSE;

    if (ctx->byte2unicode[buffer[0]] == '*' && 
            ctx->byte2unicode[buffer[1]] == '.') {
        if (out_double)
            *out_double = NAN;
        return READSTAT_OK;
    }
    int64_t i=2;
    while (i<sizeof(buffer) && ctx->byte2unicode[buffer[i-1]] != '/') {
        bytes_read = read_bytes(ctx, &buffer[i], 1);
        if (bytes_read != 1)
            return READSTAT_ERROR_PARSE;
        i++;
    }

    if (i == sizeof(buffer)) {
        return READSTAT_ERROR_PARSE;
    }

    len = por_utf8_encode(buffer, i, utf8_buffer, sizeof(utf8_buffer), ctx->byte2unicode);
    if (len == -1) {
        if (ctx->error_handler) {
            snprintf(error_buf, sizeof(error_buf), "Error converting double string (length=%" PRId64 "): %*s", 
                    i, (int)i, buffer);
            ctx->error_handler(error_buf, ctx->user_ctx);
        }
        retval = READSTAT_ERROR_CONVERT;
        goto cleanup;
    }
    
    bytes_read = readstat_por_parse_double(utf8_buffer, len, &value, ctx->error_handler, ctx->user_ctx);
    if (bytes_read == -1) {
        if (ctx->error_handler) {
            snprintf(error_buf, sizeof(error_buf), "Error parsing double string (length=%" PRId64 "): %*s [%s]", 
                    len, (int)len, utf8_buffer, buffer);
            ctx->error_handler(error_buf, ctx->user_ctx);
        }
        retval = READSTAT_ERROR_PARSE;
        goto cleanup;
    }
    
cleanup:
    if (out_double)
        *out_double = value;

    return retval;
}

static readstat_error_t read_double(por_ctx_t *ctx, double *out_double) {
    unsigned char peek;
    size_t bytes_read = read_bytes(ctx, &peek, 1);
    if (bytes_read != 1)
        return READSTAT_ERROR_PARSE;
    return read_double_with_peek(ctx, out_double, peek);
}

static readstat_error_t maybe_read_double(por_ctx_t *ctx, double *out_double, int *out_finished) {
    unsigned char peek;
    size_t bytes_read = read_bytes(ctx, &peek, 1);
    if (bytes_read != 1)
        return READSTAT_ERROR_PARSE;

    if (ctx->byte2unicode[peek] == 'Z') {
        if (out_double)
            *out_double = NAN;
        if (out_finished)
            *out_finished = 1;
        return READSTAT_OK;
    }

    if (out_finished)
        *out_finished = 0;

    return read_double_with_peek(ctx, out_double, peek);
}

static readstat_error_t maybe_read_string(por_ctx_t *ctx, char *data, size_t len, int *out_finished) {
    readstat_error_t retval = READSTAT_OK;
    double value;
    int finished = 0;
    char error_buf[1024];
    retval = maybe_read_double(ctx, &value, &finished);
    if (retval != READSTAT_OK || finished) {
        if (out_finished)
            *out_finished = finished;

        return retval;
    }
    
    size_t string_length = (size_t)value;
    if (string_length <= 0 || string_length > 20000) {
        retval = READSTAT_ERROR_PARSE;
        goto cleanup;
    }
    
    if (string_length > ctx->string_buffer_len) {
        ctx->string_buffer_len = string_length;
        ctx->string_buffer = realloc(ctx->string_buffer, ctx->string_buffer_len);
    }
    
    if (read_bytes(ctx, ctx->string_buffer, string_length) == -1) {
        retval = READSTAT_ERROR_READ;
        goto cleanup;
    }
    size_t bytes_encoded = por_utf8_encode(ctx->string_buffer, string_length, 
            data, len - 1, ctx->byte2unicode);
    if (bytes_encoded == -1) {
        if (ctx->error_handler) {
            snprintf(error_buf, sizeof(error_buf), "Error converting string: %*s", 
                    (int)string_length, ctx->string_buffer);
            ctx->error_handler(error_buf, ctx->user_ctx);
        }
        retval = READSTAT_ERROR_CONVERT;
        goto cleanup;
    }
    
    data[bytes_encoded] = '\0';
    if (out_finished)
        *out_finished = 0;

cleanup:
    return retval;
}

static readstat_error_t read_string(por_ctx_t *ctx, char *data, size_t len) {
    int finished = 0;
    readstat_error_t retval = maybe_read_string(ctx, data, len, &finished);
    if (retval == READSTAT_OK && finished) {
        return READSTAT_ERROR_PARSE;
    }
    return retval;
}

static readstat_error_t read_variable_count_record(por_ctx_t *ctx) {
    double value;
    readstat_error_t retval = READSTAT_OK;
    if ((retval = read_double(ctx, &value)) != READSTAT_OK) {
        goto cleanup;
    }
    ctx->var_count = (int)value;
    ctx->variables = calloc(ctx->var_count, sizeof(readstat_variable_t *));
    ctx->varinfo = calloc(ctx->var_count, sizeof(spss_varinfo_t));
    if (ctx->info_handler) {
        if (ctx->info_handler(-1, ctx->var_count, ctx->user_ctx) != READSTAT_HANDLER_OK) {
            retval = READSTAT_ERROR_USER_ABORT;
            goto cleanup;
        }
    }
cleanup:
    return retval;
}

static readstat_error_t read_precision_record(por_ctx_t *ctx) {
    double precision = NAN;
    readstat_error_t error = read_double(ctx, &precision);
    if (error == READSTAT_OK)
        ctx->base30_precision = precision;
    return error;
}

static readstat_error_t read_case_weight_record(por_ctx_t *ctx) {
    return read_string(ctx, ctx->fweight_name, sizeof(ctx->fweight_name));
}

static readstat_error_t read_variable_record(por_ctx_t *ctx) {
    readstat_error_t retval = READSTAT_OK;
    double value;
    int i;
    ctx->var_offset++;

    spss_varinfo_t *varinfo = &ctx->varinfo[ctx->var_offset];
    spss_format_t *formats[2] = { &varinfo->print_format, &varinfo->write_format };

    if ((retval = read_double(ctx, &value)) != READSTAT_OK) {
        goto cleanup;
    }
    varinfo->labels_index = -1;
    varinfo->width = (int)value;
    if (varinfo->width == 0) {
        varinfo->type = READSTAT_TYPE_DOUBLE;
    } else {
        varinfo->type = READSTAT_TYPE_STRING;
    }
    if ((retval = read_string(ctx, varinfo->name, sizeof(varinfo->name))) != READSTAT_OK) {
        goto cleanup;
    }
    ck_str_hash_insert(varinfo->name, varinfo, ctx->var_dict);

    for (i=0; i<sizeof(formats)/sizeof(spss_format_t *); i++) {
        spss_format_t *format = formats[i];
        if ((retval = read_double(ctx, &value)) != READSTAT_OK) {
            goto cleanup;
        }
        format->type = (int)value;

        if ((retval = read_double(ctx, &value)) != READSTAT_OK) {
            goto cleanup;
        }
        format->width = (int)value;

        if ((retval = read_double(ctx, &value)) != READSTAT_OK) {
            goto cleanup;
        }
        format->decimal_places = (int)value;
    }

cleanup:
    return retval;
}

static readstat_error_t read_missing_value_record(por_ctx_t *ctx) {
    readstat_error_t retval = READSTAT_OK;
    double value;
    char string[256];
    if (ctx->varinfo[ctx->var_offset].type == READSTAT_TYPE_DOUBLE) {
        if ((retval = read_double(ctx, &value)) != READSTAT_OK) {
            goto cleanup;
        }
        ctx->varinfo[ctx->var_offset].missing_values[ctx->varinfo[ctx->var_offset].n_missing_values++] = value;
        if (ctx->varinfo[ctx->var_offset].n_missing_values > 3) {
            retval = READSTAT_ERROR_PARSE;
            goto cleanup;
        }
    } else {
        if ((retval = read_string(ctx, string, sizeof(string))) != READSTAT_OK) {
            goto cleanup;
        }
    }
cleanup:
    return retval;
}

static readstat_error_t read_missing_value_range_record(por_ctx_t *ctx) {
    readstat_error_t retval = READSTAT_OK;
    double value;
    char string[256];
    if (ctx->varinfo[ctx->var_offset].type == READSTAT_TYPE_DOUBLE) {
        ctx->varinfo[ctx->var_offset].missing_range = 1;
        if ((retval = read_double(ctx, &value)) != READSTAT_OK) {
            goto cleanup;
        }
        ctx->varinfo[ctx->var_offset].missing_values[0] = value;
        if ((retval = read_double(ctx, &value)) != READSTAT_OK) {
            goto cleanup;
        }
        ctx->varinfo[ctx->var_offset].missing_values[1] = value;
        ctx->varinfo[ctx->var_offset].n_missing_values = 2;
    } else {
        if ((retval = read_string(ctx, string, sizeof(string))) != READSTAT_OK) {
            goto cleanup;
        }
        if ((retval = read_string(ctx, string, sizeof(string))) != READSTAT_OK) {
            goto cleanup;
        }
    }
cleanup:
    return retval;
}

static readstat_error_t read_missing_value_lo_range_record(por_ctx_t *ctx) {
    readstat_error_t retval = READSTAT_OK;
    double value;
    char string[256];
    if (ctx->varinfo[ctx->var_offset].type == READSTAT_TYPE_DOUBLE) {
        ctx->varinfo[ctx->var_offset].missing_range = 1;
        if ((retval = read_double(ctx, &value)) != READSTAT_OK) {
            goto cleanup;
        }
        ctx->varinfo[ctx->var_offset].missing_values[0] = -HUGE_VAL;
        ctx->varinfo[ctx->var_offset].missing_values[1] = value;
        ctx->varinfo[ctx->var_offset].n_missing_values = 2;
    } else {
        if ((retval = read_string(ctx, string, sizeof(string))) != READSTAT_OK) {
            goto cleanup;
        }
    }
cleanup:
    return retval;
}

static readstat_error_t read_missing_value_hi_range_record(por_ctx_t *ctx) {
    readstat_error_t retval = READSTAT_OK;
    double value;
    char string[256];
    if (ctx->varinfo[ctx->var_offset].type == READSTAT_TYPE_DOUBLE) {
        ctx->varinfo[ctx->var_offset].missing_range = 1;
        if ((retval = read_double(ctx, &value)) != READSTAT_OK) {
            goto cleanup;
        }
        ctx->varinfo[ctx->var_offset].missing_values[0] = value;
        ctx->varinfo[ctx->var_offset].missing_values[1] = HUGE_VAL;
        ctx->varinfo[ctx->var_offset].n_missing_values = 2;
    } else {
        if ((retval = read_string(ctx, string, sizeof(string))) != READSTAT_OK) {
            goto cleanup;
        }
    }
cleanup:
    return retval;
}

static readstat_error_t read_document_record(por_ctx_t *ctx) {
    readstat_error_t retval = READSTAT_OK;
    double value;
    char string[256];
    int i;
    int line_count = 0;
    if ((retval = read_double(ctx, &value)) != READSTAT_OK) {
        goto cleanup;
    }
    line_count = (int)value;
    for (i=0; i<line_count; i++) {
        if ((retval = read_string(ctx, string, sizeof(string))) != READSTAT_OK) {
            goto cleanup;
        }
        if (ctx->note_handler) {
            if (ctx->note_handler(i, string, ctx->user_ctx) != READSTAT_OK) {
                retval = READSTAT_ERROR_USER_ABORT;
                goto cleanup;
            }
        }
    }
cleanup:
    return retval;
}

static readstat_error_t read_variable_label_record(por_ctx_t *ctx) {
    readstat_error_t retval = READSTAT_OK;
    char string[256];
    if ((retval = read_string(ctx, string, sizeof(string))) != READSTAT_OK) {
        goto cleanup;
    }

    ctx->varinfo[ctx->var_offset].label = malloc(strlen(string) + 1);
    strcpy(ctx->varinfo[ctx->var_offset].label, string);

cleanup:
    return retval;
}

static readstat_error_t read_value_label_record(por_ctx_t *ctx) {
    readstat_error_t retval = READSTAT_OK;
    double dval;
    int i;
    char string[256];
    int count = 0, label_count = 0;
    char label_name_buf[256];
    char label_buf[256];
    snprintf(label_name_buf, sizeof(label_name_buf), POR_LABEL_NAME_PREFIX "%d", ctx->labels_offset);
    readstat_type_t value_type = READSTAT_TYPE_DOUBLE;
    if ((retval = read_double(ctx, &dval)) != READSTAT_OK) {
        goto cleanup;
    }
    count = (int)dval;
    for (i=0; i<count; i++) {
        if ((retval = read_string(ctx, string, sizeof(string))) != READSTAT_OK) {
            goto cleanup;
        }
        spss_varinfo_t *info = (spss_varinfo_t *)ck_str_hash_lookup(string, ctx->var_dict);
        if (info) {
            value_type = info->type;
            info->labels_index = ctx->labels_offset;
        }
    }
    if ((retval = read_double(ctx, &dval)) != READSTAT_OK) {
        goto cleanup;
    }
    label_count = (int)dval;
    for (i=0; i<label_count; i++) {
        readstat_value_t value = { .type = value_type };
        if (value_type == READSTAT_TYPE_STRING) {
            if ((retval = read_string(ctx, string, sizeof(string))) != READSTAT_OK) {
                goto cleanup;
            }
            if ((retval = read_string(ctx, label_buf, sizeof(label_buf))) != READSTAT_OK) {
                goto cleanup;
            }
            value.v.string_value = string;
        } else {
            if ((retval = read_double(ctx, &dval)) != READSTAT_OK) {
                goto cleanup;
            }
            if ((retval = read_string(ctx, label_buf, sizeof(label_buf))) != READSTAT_OK) {
                goto cleanup;
            }
            value.v.double_value = dval;
        }
        if (ctx->value_label_handler(label_name_buf, value, label_buf, ctx->user_ctx) != READSTAT_HANDLER_OK) {
            retval = READSTAT_ERROR_USER_ABORT;
            goto cleanup;
        }
    }
    ctx->labels_offset++;

cleanup:
    return retval;
}

static readstat_error_t read_por_file_data(por_ctx_t *ctx) {
    int i;
    char input_string[256];
    char output_string[4*256+1];
    char error_buf[1024];
    readstat_error_t rs_retval = READSTAT_OK;

    if (ctx->var_count == 0)
        return READSTAT_OK;

    while (1) {
        int finished = 0;
        for (i=0; i<ctx->var_count; i++) {
            spss_varinfo_t *info = &ctx->varinfo[i];
            readstat_value_t value = { .type = info->type };

            if (info->type == READSTAT_TYPE_STRING) {
                rs_retval = maybe_read_string(ctx, input_string, sizeof(input_string), &finished);
                if (rs_retval != READSTAT_OK) {
                    if (ctx->error_handler) {
                        snprintf(error_buf, sizeof(error_buf), "Error in %s (row=%d)", 
                                info->name, ctx->obs_count+1);
                        ctx->error_handler(error_buf, ctx->user_ctx);
                    }
                    goto cleanup;
                } else if (finished) {
                    if (i != 0)
                        rs_retval = READSTAT_ERROR_PARSE;
                    goto cleanup;
                }
                rs_retval = readstat_convert(output_string, sizeof(output_string),
                        input_string, strlen(input_string), ctx->converter);
                if (rs_retval != READSTAT_OK) {
                    goto cleanup;
                }
                value.v.string_value = output_string;
            } else if (info->type == READSTAT_TYPE_DOUBLE) {
                rs_retval = maybe_read_double(ctx, &value.v.double_value, &finished);
                if (rs_retval != READSTAT_OK) {
                    if (ctx->error_handler) {
                        snprintf(error_buf, sizeof(error_buf), "Error in %s (row=%d)", 
                                info->name, ctx->obs_count+1);
                        ctx->error_handler(error_buf, ctx->user_ctx);
                    }
                    goto cleanup;
                } else if (finished) {
                    if (i != 0)
                        rs_retval = READSTAT_ERROR_PARSE;
                    goto cleanup;
                }
                value.is_system_missing = isnan(value.v.double_value);
            }
            if (ctx->value_handler && !ctx->variables[i]->skip) {
                if (ctx->value_handler(ctx->obs_count, ctx->variables[i], value, ctx->user_ctx) != READSTAT_HANDLER_OK) {
                    rs_retval = READSTAT_ERROR_USER_ABORT;
                    goto cleanup;
                }
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

readstat_error_t read_version_and_timestamp(por_ctx_t *ctx) {
    readstat_error_t retval = READSTAT_OK;
    char string[256];
    struct tm timestamp = { .tm_isdst = -1 };
    unsigned char version;

    if (read_bytes(ctx, &version, sizeof(version)) != sizeof(version)) {
        retval = READSTAT_ERROR_READ;
        goto cleanup;
    }
    if ((retval = read_string(ctx, string, sizeof(string))) != READSTAT_OK) { /* creation date */
        goto cleanup;
    }
    if (sscanf(string, "%04d%02d%02d", &timestamp.tm_year, &timestamp.tm_mon, &timestamp.tm_mday) != 3) {
        retval = READSTAT_ERROR_BAD_TIMESTAMP;
        goto cleanup;
    }
    if ((retval = read_string(ctx, string, sizeof(string))) != READSTAT_OK) { /* creation time */
        goto cleanup;
    }
    if (sscanf(string, "%02d%02d%02d", &timestamp.tm_hour, &timestamp.tm_min, &timestamp.tm_sec) != 3) {
        retval = READSTAT_ERROR_BAD_TIMESTAMP;
        goto cleanup;
    }

    timestamp.tm_year -= 1900;
    timestamp.tm_mon--;

    ctx->timestamp = mktime(&timestamp);
    ctx->version = ctx->byte2unicode[version] - 'A';

cleanup:
    return retval;
}

readstat_error_t handle_variables(por_ctx_t *ctx) {
    readstat_error_t retval = READSTAT_OK;
    int i;
    int index_after_skipping = 0;
    for (i=0; i<ctx->var_count; i++) {
        char label_name_buf[256];
        spss_varinfo_t *info = &ctx->varinfo[i];
        info->index = i;

        ctx->variables[i] = spss_init_variable_for_info(info, index_after_skipping);

        snprintf(label_name_buf, sizeof(label_name_buf), POR_LABEL_NAME_PREFIX "%d", info->labels_index);

        int cb_retval = READSTAT_HANDLER_OK;

        if (ctx->variable_handler) {
            cb_retval = ctx->variable_handler(i, ctx->variables[i],
                    info->labels_index == -1 ? NULL : label_name_buf,
                    ctx->user_ctx);
        }

        if (cb_retval == READSTAT_HANDLER_ABORT) {
            retval = READSTAT_ERROR_USER_ABORT;
            goto cleanup;
        }

        if (cb_retval == READSTAT_HANDLER_SKIP_VARIABLE) {
            ctx->variables[i]->skip = 1;
        } else {
            index_after_skipping++;
        }
    }
    if (ctx->fweight_handler && ctx->fweight_name[0]) {
        for (i=0; i<ctx->var_count; i++) {
            spss_varinfo_t *info = &ctx->varinfo[i];
            if (strcmp(info->name, ctx->fweight_name) == 0) {
                if (ctx->fweight_handler(ctx->variables[i], ctx->user_ctx) != READSTAT_HANDLER_OK) {
                    retval = READSTAT_ERROR_USER_ABORT;
                    goto cleanup;
                }
                break;
            }
        }
    }
cleanup:
    return retval;
}

readstat_error_t readstat_parse_por(readstat_parser_t *parser, const char *path, void *user_ctx) {
    readstat_error_t retval = READSTAT_OK;
    readstat_io_t *io = parser->io;
    unsigned char reverse_lookup[256];
    char vanity[5][40];
    char file_label[21];
    char error_buf[1024];

    por_ctx_t *ctx = por_ctx_init();
    
    ctx->info_handler = parser->info_handler;
    ctx->metadata_handler = parser->metadata_handler;
    ctx->note_handler = parser->note_handler;
    ctx->fweight_handler = parser->fweight_handler;
    ctx->variable_handler = parser->variable_handler;
    ctx->value_handler = parser->value_handler;
    ctx->value_label_handler = parser->value_label_handler;
    ctx->error_handler = parser->error_handler;
    ctx->progress_handler = parser->progress_handler;
    ctx->user_ctx = user_ctx;
    ctx->io = io;
    ctx->row_limit = parser->row_limit;

    if (parser->output_encoding) {
        if (strcmp(parser->output_encoding, "UTF-8") != 0)
            ctx->converter = iconv_open(parser->output_encoding, "UTF-8");

        if (ctx->converter == (iconv_t)-1) {
            ctx->converter = NULL;
            retval = READSTAT_ERROR_UNSUPPORTED_CHARSET;
            goto cleanup;
        }
    }
    
    if (io->open(path, io->io_ctx) == -1) {
        retval = READSTAT_ERROR_OPEN;
        goto cleanup;
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

    readstat_convert(file_label, sizeof(file_label), vanity[1] + 20, 20, NULL);
    
    if (read_bytes(ctx, reverse_lookup, sizeof(reverse_lookup)) != sizeof(reverse_lookup)) {
        retval = READSTAT_ERROR_READ;
        goto cleanup;
    }
    
    ctx->space = reverse_lookup[126];

    int i;

    for (i=0; i<256; i++) {
        if (por_ascii_lookup[i]) {
            ctx->byte2unicode[reverse_lookup[i]] = por_ascii_lookup[i];
        } else if (por_unicode_lookup[i]) {
            ctx->byte2unicode[reverse_lookup[i]] = por_unicode_lookup[i];
        }
    }

    ctx->byte2unicode[reverse_lookup[64]] = por_unicode_lookup[64];

    unsigned char check[8];
    char tr_check[8];
    
    if (read_bytes(ctx, check, sizeof(check)) != sizeof(check)) {
        retval = READSTAT_ERROR_READ;
        goto cleanup;
    }
    
    if (por_utf8_encode(check, sizeof(check), tr_check, sizeof(tr_check), ctx->byte2unicode) == -1) {
        if (ctx->error_handler) {
            snprintf(error_buf, sizeof(error_buf), "Error converting check string: %*s\n", (int)sizeof(check), check);
            ctx->error_handler(error_buf, ctx->user_ctx);
        }
        retval = READSTAT_ERROR_CONVERT;
        goto cleanup;
    }

    if (strncmp("SPSSPORT", tr_check, sizeof(tr_check)) != 0) {
        retval = READSTAT_ERROR_PARSE;
        goto cleanup;
    }
    
    ctx->var_offset = -1;
    
    char string[256];

    retval = read_version_and_timestamp(ctx);
    if (retval != READSTAT_OK)
        goto cleanup;

    if (ctx->metadata_handler) {
        if (ctx->metadata_handler(file_label, NULL, ctx->timestamp, ctx->version, ctx->user_ctx) != READSTAT_HANDLER_OK) {
            retval = READSTAT_ERROR_USER_ABORT;
            goto cleanup;
        }
    }
        
    while (1) {
        uint16_t tr_tag = read_tag(ctx);
        switch (tr_tag) {
            case '1': /* product ID */
            case '2': /* author ID */
            case '3': /* sub-product ID */
                retval = read_string(ctx, string, sizeof(string));
                break;
            case '4': /* variable count */
                retval = read_variable_count_record(ctx);
                break;
            case '5': /* precision */
                retval = read_precision_record(ctx);
                break;
            case '6': /* case weight */
                retval = read_case_weight_record(ctx);
                break;
            case '7': /* variable */
                retval = read_variable_record(ctx);
                break;
            case '8': /* missing value */
                retval = read_missing_value_record(ctx);
                break;
            case 'B': /* missing value range */
                retval = read_missing_value_range_record(ctx);
                break;
            case '9': /* LO THRU x */
                retval = read_missing_value_lo_range_record(ctx);
                break;
            case 'A': /* x THRU HI */
                retval = read_missing_value_hi_range_record(ctx);
                break;
            case 'C': /* variable label */
                retval = read_variable_label_record(ctx);
                break;
            case 'D': /* value label */
                retval = read_value_label_record(ctx);
                break;
            case 'E': /* document record */
                retval = read_document_record(ctx);
                break;
            case 'F': /* file data */
                if (ctx->var_offset != ctx->var_count - 1) {
                    retval = READSTAT_ERROR_COLUMN_COUNT_MISMATCH;
                    goto cleanup;
                }

                retval = handle_variables(ctx);
                if (retval != READSTAT_OK)
                    goto cleanup;

                if (ctx->value_handler) {
                    retval = read_por_file_data(ctx);
                }
                goto cleanup;
            default:
                retval = READSTAT_ERROR_PARSE;
                goto cleanup;
        }
        if (retval != READSTAT_OK)
            break;
    }

cleanup:
    io->close(io->io_ctx);
    por_ctx_free(ctx);
    
    return retval;
}
