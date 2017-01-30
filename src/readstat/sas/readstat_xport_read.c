#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <stdint.h>
#include <time.h>

#include "../readstat.h"
#include "../readstat_iconv.h"
#include "../readstat_convert.h"
#include "readstat_sas.h"
#include "readstat_xport.h"
#include "ieee.h"

#define LINE_LEN        80

typedef struct xport_ctx_s {
    readstat_info_handler           info_handler;
    readstat_metadata_handler       metadata_handler;
    readstat_note_handler           note_handler;
    readstat_variable_handler       variable_handler;
    readstat_fweight_handler        fweight_handler;
    readstat_value_handler          value_handler;
    readstat_value_label_handler    value_label_handler;
    readstat_error_handler          error_handler;
    readstat_progress_handler       progress_handler;
    size_t                          file_size;
    void                           *user_ctx;

    readstat_io_t *io;
    time_t         timestamp;

    int            obs_count;
    int            var_count;
    int            row_limit;
    size_t         row_length;
    int            parsed_row_count;

    readstat_variable_t **variables;

    int            version;
} xport_ctx_t;

static readstat_error_t xport_update_progress(xport_ctx_t *ctx) {
    readstat_io_t *io = ctx->io;
    return io->update(ctx->file_size, ctx->progress_handler, ctx->user_ctx, io->io_ctx);
}

static xport_ctx_t *xport_ctx_init() {
    xport_ctx_t *ctx = calloc(1, sizeof(xport_ctx_t));
    return ctx;
}

static void xport_ctx_free(xport_ctx_t *ctx) {
    if (ctx->variables) {
        int i;
        for (i=0; i<ctx->var_count; i++) {
            if (ctx->variables[i])
                free(ctx->variables[i]);
        }
        free(ctx->variables);
    }

    free(ctx);
}

static ssize_t read_bytes(xport_ctx_t *ctx, void *dst, size_t dst_len) {
    readstat_io_t *io = (readstat_io_t *)ctx->io;
    return io->read(dst, dst_len, io->io_ctx);
}

static readstat_error_t xport_skip_record(xport_ctx_t *ctx) {
    readstat_io_t *io = (readstat_io_t *)ctx->io;
    if (io->seek(LINE_LEN, READSTAT_SEEK_CUR, io->io_ctx) == -1)
        return READSTAT_ERROR_SEEK;

    return READSTAT_OK;
}

static readstat_error_t xport_skip_rest_of_record(xport_ctx_t *ctx) {
    readstat_io_t *io = (readstat_io_t *)ctx->io;
    off_t pos = io->seek(0, READSTAT_SEEK_CUR, io->io_ctx);
    if (pos == -1)
        return READSTAT_ERROR_SEEK;

    if (pos % LINE_LEN) {
        if (io->seek(LINE_LEN - (pos % LINE_LEN), READSTAT_SEEK_CUR, io->io_ctx) == -1)
            return READSTAT_ERROR_SEEK;
    }

    return READSTAT_OK;
}

static readstat_error_t xport_read_record(xport_ctx_t *ctx, char *record) {
    ssize_t bytes_read = read_bytes(ctx, record, LINE_LEN);
    if (bytes_read < LINE_LEN)
        return READSTAT_ERROR_READ;

    record[LINE_LEN] = '\0';

    return READSTAT_OK;
}

static readstat_error_t xport_read_header_record(xport_ctx_t *ctx, xport_header_record_t *xrecord) {
    char line[LINE_LEN+1];
    readstat_error_t retval = READSTAT_OK;

    retval = xport_read_record(ctx, line);
    if (retval != READSTAT_OK)
        return retval;

    memset(xrecord, 0, sizeof(xport_header_record_t));
    int matches = sscanf(line,
            "HEADER RECORD*******%8s HEADER RECORD!!!!!!!"
            "%05d%05d%05d" "%05d%05d%05d", xrecord->name,
            &xrecord->num1, &xrecord->num2, &xrecord->num3,
            &xrecord->num4, &xrecord->num5, &xrecord->num6);

    if (matches < 2) {
        return READSTAT_ERROR_PARSE;
    }

    return READSTAT_OK;
}

static readstat_error_t xport_expect_header_record(xport_ctx_t *ctx, 
        const char *v5_name, const char *v8_name) {
    readstat_error_t retval = READSTAT_OK;
    xport_header_record_t xrecord;

    retval = xport_read_header_record(ctx, &xrecord);
    if (retval != READSTAT_OK)
        goto cleanup;

    if (ctx->version == 5 && strcmp(xrecord.name, v5_name) != 0) {
        retval = READSTAT_ERROR_PARSE;
        goto cleanup;
    } else if (ctx->version == 8 && strcmp(xrecord.name, v8_name) != 0) {
        retval = READSTAT_ERROR_PARSE;
        goto cleanup;
    }

cleanup:
    return retval;
}

static readstat_error_t xport_read_file_label_record(xport_ctx_t *ctx) {
    char line[LINE_LEN+1];
    char label[40*4+1];
    readstat_error_t retval = READSTAT_OK;

    retval = xport_read_record(ctx, line);
    if (retval != READSTAT_OK)
        goto cleanup;

    retval = readstat_convert(label, sizeof(label), &line[32], 40, NULL);
    if (retval != READSTAT_OK)
        goto cleanup;

    if (ctx->metadata_handler) {
        if (ctx->metadata_handler(label, ctx->timestamp, ctx->version, ctx->user_ctx) != READSTAT_HANDLER_OK) {
            retval = READSTAT_ERROR_USER_ABORT;
            goto cleanup;
        }
    }

cleanup:
    return retval;
}

static readstat_error_t xport_read_library_record(xport_ctx_t *ctx) {
    xport_header_record_t xrecord;
    readstat_error_t retval = xport_read_header_record(ctx, &xrecord);
    if (retval != READSTAT_OK)
        goto cleanup;

    if (strcmp(xrecord.name, "LIBRARY") == 0) {
        ctx->version = 5;
    } else if (strcmp(xrecord.name, "LIBV8") == 0) {
        ctx->version = 8;
    } else {
        retval = READSTAT_ERROR_UNSUPPORTED_FILE_FORMAT_VERSION;
        goto cleanup;
    }

cleanup:
    return retval;
}

static readstat_error_t xport_read_timestamp_record(xport_ctx_t *ctx) {
    char line[LINE_LEN+1];
    readstat_error_t retval = READSTAT_OK;
    struct tm ts = { .tm_isdst = -1 };
    char month[4];
    int i;

    retval = xport_read_record(ctx, line);
    if (retval != READSTAT_OK)
        goto cleanup;

    sscanf(line,
            "%02d%3s%02d:%02d:%02d:%02d",
            &ts.tm_mday, month, &ts.tm_year, &ts.tm_hour, &ts.tm_min, &ts.tm_sec);

    for (i=0; i<sizeof(_xport_months)/sizeof(_xport_months[0]); i++) {
        if (strcmp(month, _xport_months[i]) == 0) {
            ts.tm_mon = i;
            break;
        }
    }

    if (ts.tm_year < 60) {
        ts.tm_year += 100;
    }

    ctx->timestamp = mktime(&ts);

cleanup:
    return retval;
}

static readstat_error_t xport_read_namestr_header_record(xport_ctx_t *ctx) {
    xport_header_record_t xrecord;
    readstat_error_t retval = READSTAT_OK;

    retval = xport_read_header_record(ctx, &xrecord);
    if (retval != READSTAT_OK)
        goto cleanup;

    if (ctx->version == 5 && strcmp(xrecord.name, "NAMESTR") != 0) {
        retval = READSTAT_ERROR_PARSE;
        goto cleanup;
    } else if (ctx->version == 8 && strcmp(xrecord.name, "NAMSTV8") != 0) {
        retval = READSTAT_ERROR_PARSE;
        goto cleanup;
    }

    ctx->var_count = xrecord.num2;
    ctx->variables = calloc(ctx->var_count, sizeof(readstat_variable_t *));

    if (ctx->info_handler) {
        if (ctx->info_handler(-1, ctx->var_count, ctx->user_ctx) != READSTAT_HANDLER_OK) {
            retval = READSTAT_ERROR_USER_ABORT;
            goto cleanup;
        }
    }

cleanup:
    return retval;
}

static readstat_error_t xport_read_obs_header_record(xport_ctx_t *ctx) {
    return xport_expect_header_record(ctx, "OBS", "OBSV8");
}

static readstat_error_t xport_construct_format(char *dst, size_t dst_len,
        const char *src, size_t src_len, int width, int decimals) {
    char format[4*src_len+1];
    readstat_error_t retval = readstat_convert(format, sizeof(format), src, src_len, NULL);

    if (decimals) {
        snprintf(dst, dst_len, "%s%d.%d",
                format, width, decimals);
    } else if (width) {
        snprintf(dst, dst_len, "%s%d",
                format, width);
    } else {
        strcpy(dst, format);
    }

    return retval;
}

static readstat_error_t xport_read_labels_v8(xport_ctx_t *ctx, int label_count) {
    readstat_error_t retval = READSTAT_OK;
    uint16_t labeldef[3];
    int i;
    for (i=0; i<label_count; i++) {
        int index, name_len, label_len;
        if (read_bytes(ctx, labeldef, sizeof(labeldef)) != sizeof(labeldef)) {
            retval = READSTAT_ERROR_READ;
            goto cleanup;
        }

        if (machine_is_little_endian()) {
            index = byteswap2(labeldef[0]);
            name_len = byteswap2(labeldef[1]);
            label_len = byteswap2(labeldef[2]);
        } else {
            index = labeldef[0];
            name_len = labeldef[1];
            label_len = labeldef[2];
        }

        if (index >= ctx->var_count) {
            retval = READSTAT_ERROR_PARSE;
            goto cleanup;
        }

        char name[name_len];
        char label[label_len];
        readstat_variable_t *variable = ctx->variables[index];

        if (read_bytes(ctx, name, name_len) != name_len ||
                read_bytes(ctx, label, label_len) != label_len) {
            retval = READSTAT_ERROR_READ;
            goto cleanup;
        }

        retval = readstat_convert(variable->name, sizeof(variable->name),
                name, name_len,  NULL);
        if (retval != READSTAT_OK)
            goto cleanup;

        retval = readstat_convert(variable->label, sizeof(variable->label),
                label, label_len,  NULL);
        if (retval != READSTAT_OK)
            goto cleanup;
    }

    retval = xport_skip_rest_of_record(ctx);
    if (retval != READSTAT_OK)
        goto cleanup;

    retval = xport_read_obs_header_record(ctx);
    if (retval != READSTAT_OK)
        goto cleanup;

cleanup:
    return retval;
}

static readstat_error_t xport_read_labels_v9(xport_ctx_t *ctx, int label_count) {
    readstat_error_t retval = READSTAT_OK;
    uint16_t labeldef[5];
    int i;
    for (i=0; i<label_count; i++) {
        int index, name_len, format_len, informat_len, label_len;
        if (read_bytes(ctx, labeldef, sizeof(labeldef)) != sizeof(labeldef)) {
            retval = READSTAT_ERROR_READ;
            goto cleanup;
        }

        if (machine_is_little_endian()) {
            index = byteswap2(labeldef[0]);
            name_len = byteswap2(labeldef[1]);
            format_len = byteswap2(labeldef[2]);
            informat_len = byteswap2(labeldef[3]);
            label_len = byteswap2(labeldef[4]);
        } else {
            index = labeldef[0];
            name_len = labeldef[1];
            format_len = labeldef[2];
            informat_len = labeldef[3];
            label_len = labeldef[4];
        }

        if (index >= ctx->var_count) {
            retval = READSTAT_ERROR_PARSE;
            goto cleanup;
        }

        char name[name_len];
        char format[format_len];
        char informat[informat_len];
        char label[label_len];

        readstat_variable_t *variable = ctx->variables[index];

        if (read_bytes(ctx, name, name_len) != name_len ||
                read_bytes(ctx, format, format_len) != format_len ||
                read_bytes(ctx, informat, informat_len) != informat_len ||
                read_bytes(ctx, label, label_len) != label_len) {
            retval = READSTAT_ERROR_READ;
            goto cleanup;
        }

        retval = readstat_convert(variable->name, sizeof(variable->name),
                name, name_len,  NULL);
        if (retval != READSTAT_OK)
            goto cleanup;

        retval = readstat_convert(variable->label, sizeof(variable->label),
                label, label_len,  NULL);
        if (retval != READSTAT_OK)
            goto cleanup;

        retval = xport_construct_format(variable->format, sizeof(variable->format),
                format, format_len, variable->display_width, variable->decimals);
        if (retval != READSTAT_OK)
            goto cleanup;
    }

    retval = xport_skip_rest_of_record(ctx);
    if (retval != READSTAT_OK)
        goto cleanup;

    retval = xport_read_obs_header_record(ctx);
    if (retval != READSTAT_OK)
        goto cleanup;

cleanup:
    return retval;
}

static readstat_error_t xport_read_variables(xport_ctx_t *ctx) {
    int i;
    readstat_error_t retval = READSTAT_OK;
    for (i=0; i<ctx->var_count; i++) {
        xport_namestr_t namestr;
        ssize_t bytes_read = read_bytes(ctx, &namestr, sizeof(xport_namestr_t));
        if (bytes_read < sizeof(xport_namestr_t)) {
            retval = READSTAT_ERROR_READ;
            goto cleanup;
        }
        xport_namestr_bswap(&namestr);

        readstat_variable_t *variable = calloc(1, sizeof(readstat_variable_t));

        variable->index = i;
        variable->type = namestr.ntype == SAS_COLUMN_TYPE_CHR ? READSTAT_TYPE_STRING : READSTAT_TYPE_DOUBLE;
        variable->storage_width = namestr.nlng;
        variable->display_width = namestr.nfl;
        variable->decimals = namestr.nfd;
        variable->alignment = namestr.nfj ? READSTAT_ALIGNMENT_RIGHT : READSTAT_ALIGNMENT_LEFT;

        readstat_convert(variable->name, sizeof(variable->name),
                namestr.nname, sizeof(namestr.nname), NULL);
        if (retval != READSTAT_OK)
            goto cleanup;

        readstat_convert(variable->label, sizeof(variable->label),
                namestr.nlabel, sizeof(namestr.nlabel), NULL);
        if (retval != READSTAT_OK)
            goto cleanup;

        xport_construct_format(variable->format, sizeof(variable->format),
                namestr.nform, sizeof(namestr.nform),
                variable->display_width, variable->decimals);
        if (retval != READSTAT_OK)
            goto cleanup;

        ctx->variables[i] = variable;
    }

    retval = xport_skip_rest_of_record(ctx);
    if (retval != READSTAT_OK)
        goto cleanup;

    if (ctx->version == 5) {
        retval = xport_read_obs_header_record(ctx);
        if (retval != READSTAT_OK)
            goto cleanup;
    } else {
        xport_header_record_t xrecord;
        retval = xport_read_header_record(ctx, &xrecord);
        if (retval != READSTAT_OK)
            goto cleanup;

        if (strcmp(xrecord.name, "OBSV8") == 0) {
            /* void */
        } else if (strcmp(xrecord.name, "LABELV8") == 0) {
            retval = xport_read_labels_v8(ctx, xrecord.num1);
        } else if (strcmp(xrecord.name, "LABELV9") == 0) {
            retval = xport_read_labels_v9(ctx, xrecord.num1);
        }
        if (retval != READSTAT_OK)
            goto cleanup;
    }

    ctx->row_length = 0;

    int index_after_skipping = 0;

    for (i=0; i<ctx->var_count; i++) {
        readstat_variable_t *variable = ctx->variables[i];
        variable->index_after_skipping = index_after_skipping;
        
        int cb_retval = READSTAT_HANDLER_OK;
        if (ctx->variable_handler) {
            cb_retval = ctx->variable_handler(i, variable, variable->format, ctx->user_ctx);
        }
        if (cb_retval == READSTAT_HANDLER_ABORT) {
            retval = READSTAT_ERROR_USER_ABORT;
            goto cleanup;
        }
        if (cb_retval == READSTAT_HANDLER_SKIP_VARIABLE) {
            variable->skip = 1;
        } else {
            index_after_skipping++;
        }

        ctx->row_length += variable->storage_width;
    }

cleanup:
    return retval;
}

static readstat_error_t xport_process_row(xport_ctx_t *ctx, const char *row, size_t row_length) {
    readstat_error_t retval = READSTAT_OK;
    int i;
    off_t pos = 0;
    char *string = NULL;
    for (i=0; i<ctx->var_count; i++) {
        readstat_variable_t *variable = ctx->variables[i];
        readstat_value_t value = { .type = variable->type };

        if (variable->type == READSTAT_TYPE_STRING) {
            string = realloc(string, 4*variable->storage_width+1);
            retval = readstat_convert(string, 4*variable->storage_width+1,
                    &row[pos], variable->storage_width, NULL);
            if (retval != READSTAT_OK)
                goto cleanup;

            value.v.string_value = string;
        } else {
            double dval = NAN;
            if (variable->storage_width <= XPORT_MAX_DOUBLE_SIZE &&
                    variable->storage_width >= XPORT_MIN_DOUBLE_SIZE) {
                char full_value[8] = { 0 };
                if (memcmp(&full_value[1], &row[pos+1], variable->storage_width - 1) == 0 &&
                        (row[pos] == '_' || row[pos] == '.' || (row[pos] >= 'A' && row[pos] <= 'Z'))) {
                    if (row[pos] == '.') {
                        value.is_system_missing = 1;
                    } else {
                        value.tag = row[pos];
                        value.is_tagged_missing = 1;
                    }
                } else {
                    memcpy(full_value, &row[pos], variable->storage_width);
                    int rc = cnxptiee(full_value, CN_TYPE_XPORT, &dval, CN_TYPE_NATIVE);
                    if (rc != 0) {
                        retval = READSTAT_ERROR_CONVERT;
                        goto cleanup;
                    }
                }
            }

            value.v.double_value = dval;
        }
        pos += variable->storage_width;

        if (ctx->value_handler(ctx->parsed_row_count, variable, value, ctx->user_ctx) != READSTAT_HANDLER_OK) {
            retval = READSTAT_ERROR_USER_ABORT;
            goto cleanup;
        }
    }

cleanup:
    free(string);
    return retval;
}

static readstat_error_t xport_read_data(xport_ctx_t *ctx) {
    if (!ctx->row_length)
        return READSTAT_OK;

    if (!ctx->value_handler)
        return READSTAT_OK;

    readstat_error_t retval = READSTAT_OK;
    char *row = malloc(ctx->row_length);
    char *blank_row = malloc(ctx->row_length);
    memset(blank_row, ' ', ctx->row_length);
    int num_blank_rows = 0;
    while (1) {
        ssize_t bytes_read = read_bytes(ctx, row, ctx->row_length);
        if (bytes_read == -1) {
            retval = READSTAT_ERROR_READ;
            goto cleanup;
        } else if (bytes_read < ctx->row_length) {
            break;
        }

        off_t pos = 0;

        int row_is_blank = 1;

        for (pos=0; pos<ctx->row_length; pos++) {
            if (row[pos] != ' ') {
                row_is_blank = 0;
                break;
            }
        }

        if (row_is_blank) {
            num_blank_rows++;
            continue;
        }

        while (num_blank_rows) {
            retval = xport_process_row(ctx, blank_row, ctx->row_length);
            if (retval != READSTAT_OK)
                goto cleanup;

            if (++(ctx->parsed_row_count) == ctx->row_limit)
                goto cleanup;

            num_blank_rows--;
        }

        retval = xport_process_row(ctx, row, ctx->row_length);
        if (retval != READSTAT_OK)
            goto cleanup;

        retval = xport_update_progress(ctx);
        if (retval != READSTAT_OK)
            goto cleanup;

        if (++(ctx->parsed_row_count) == ctx->row_limit)
            break;
    }

cleanup:
    free(row);
    return retval;
}

readstat_error_t readstat_parse_xport(readstat_parser_t *parser, const char *path, void *user_ctx) {
    readstat_error_t retval = READSTAT_OK;
    readstat_io_t *io = parser->io;

    xport_ctx_t *ctx = xport_ctx_init();
    ctx->info_handler = parser->info_handler;
    ctx->metadata_handler = parser->metadata_handler;
    ctx->note_handler = parser->note_handler;
    ctx->variable_handler = parser->variable_handler;
    ctx->value_handler = parser->value_handler;
    ctx->value_label_handler = parser->value_label_handler;
    ctx->error_handler = parser->error_handler;
    ctx->progress_handler = parser->progress_handler;
    ctx->user_ctx = user_ctx;
    ctx->io = io;
    ctx->row_limit = parser->row_limit;

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

    retval = xport_read_library_record(ctx);
    if (retval != READSTAT_OK)
        goto cleanup;

    retval = xport_skip_record(ctx);
    if (retval != READSTAT_OK)
        goto cleanup;

    retval = xport_read_timestamp_record(ctx);
    if (retval != READSTAT_OK)
        goto cleanup;

    retval = xport_expect_header_record(ctx, "MEMBER", "MEMBV8");
    if (retval != READSTAT_OK)
        goto cleanup;

    retval = xport_expect_header_record(ctx, "DSCRPTR", "DSCPTV8");
    if (retval != READSTAT_OK)
        goto cleanup;

    retval = xport_skip_record(ctx);
    if (retval != READSTAT_OK)
        goto cleanup;

    retval = xport_read_file_label_record(ctx);
    if (retval != READSTAT_OK)
        goto cleanup;

    retval = xport_read_namestr_header_record(ctx);
    if (retval != READSTAT_OK)
        goto cleanup;

    retval = xport_read_variables(ctx);
    if (retval != READSTAT_OK)
        goto cleanup;

    if (ctx->row_length) {
        retval = xport_read_data(ctx);
        if (retval != READSTAT_OK)
            goto cleanup;
    }

cleanup:
    io->close(io->io_ctx);
    xport_ctx_free(ctx);

    return retval;
}

