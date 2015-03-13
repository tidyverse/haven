
#include <stdlib.h>
#include <math.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>

#include "readstat_dta.h"
#include "readstat_writer.h"
#include "readstat_io.h"

static readstat_error_t dta_emit_header_data_label(readstat_writer_t *writer) {
    char data_label[81];
    memset(data_label, 0, sizeof(data_label));
    snprintf(data_label, sizeof(data_label), "%s", writer->file_label);
    return readstat_write_bytes(writer, data_label, sizeof(data_label));
}

static readstat_error_t dta_emit_header_time_stamp(readstat_writer_t *writer) {
    time_t now = time(NULL);
    struct tm *time_s = localtime(&now);
    char time_stamp[18];
    memset(time_stamp, 0, sizeof(time_stamp));
    strftime(time_stamp, sizeof(time_stamp), "%d %b %Y %H:%M", time_s);
    return readstat_write_bytes(writer, time_stamp, sizeof(time_stamp));
}

static readstat_error_t dta_emit_typlist(readstat_writer_t *writer, dta_ctx_t *ctx) {
    readstat_error_t error = READSTAT_OK;
    int i;

    for (i=0; i<ctx->nvar; i++) {
        readstat_variable_t *r_variable = readstat_get_variable(writer, i);
        unsigned char type = 0x00;
        readstat_types_t user_type = r_variable->type;
        if (user_type == READSTAT_TYPE_CHAR) {
            type = DTA_111_TYPE_CODE_CHAR;
        } else if (user_type == READSTAT_TYPE_INT16) {
            type = DTA_111_TYPE_CODE_INT16;
        } else if (user_type == READSTAT_TYPE_INT32) {
            type = DTA_111_TYPE_CODE_INT32;
        } else if (user_type == READSTAT_TYPE_FLOAT) {
            type = DTA_111_TYPE_CODE_FLOAT;
        } else if (user_type == READSTAT_TYPE_DOUBLE) {
            type = DTA_111_TYPE_CODE_DOUBLE;
        } else if (user_type == READSTAT_TYPE_STRING) {
            size_t max_len = r_variable->width;
            if (max_len > 244)
                max_len = 244;
            
            type = max_len;
        }
        ctx->typlist[i] = type;
    }

    for (i=0; i<ctx->nvar; i++) {
        unsigned char byte = ctx->typlist[i];
        error = readstat_write_bytes(writer, &byte, 1);
        if (error != READSTAT_OK)
            goto cleanup;
    }
cleanup:
    return error;
}

static readstat_error_t dta_emit_varlist(readstat_writer_t *writer, dta_ctx_t *ctx) {
    int i;
    for (i=0; i<ctx->nvar; i++) {
        readstat_variable_t *r_variable = readstat_get_variable(writer, i);

        strncpy(&ctx->varlist[ctx->variable_name_len*i], 
                r_variable->name, ctx->variable_name_len);
    }
    return readstat_write_bytes(writer, ctx->varlist, ctx->varlist_len);
}

static readstat_error_t dta_emit_srtlist(readstat_writer_t *writer, dta_ctx_t *ctx) {
    memset(ctx->srtlist, '\0', ctx->srtlist_len);
    return readstat_write_bytes(writer, ctx->srtlist, ctx->srtlist_len);
}

static readstat_error_t dta_emit_fmtlist(readstat_writer_t *writer, dta_ctx_t *ctx) {
    int i;
    for (i=0; i<ctx->nvar; i++) {
        readstat_variable_t *r_variable = readstat_get_variable(writer, i);

        strncpy(&ctx->fmtlist[ctx->fmtlist_entry_len*i], 
                r_variable->format, ctx->fmtlist_entry_len);
    }
    return readstat_write_bytes(writer, ctx->fmtlist, ctx->fmtlist_len);
}

static readstat_error_t dta_emit_lbllist(readstat_writer_t *writer, dta_ctx_t *ctx) {
    int i;
    for (i=0; i<ctx->nvar; i++) {
        readstat_variable_t *r_variable = readstat_get_variable(writer, i);

        if (r_variable->label_set) {
            strncpy(&ctx->lbllist[ctx->lbllist_entry_len*i], 
                    r_variable->label_set->name, ctx->lbllist_entry_len);
        } else {
            memset(&ctx->lbllist[ctx->lbllist_entry_len*i], '\0', ctx->lbllist_entry_len);
        }
    }
    return readstat_write_bytes(writer, ctx->lbllist, ctx->lbllist_len);
}

static readstat_error_t dta_emit_descriptors(readstat_writer_t *writer, dta_ctx_t *ctx) {
    readstat_error_t error = READSTAT_OK;

    error = dta_emit_typlist(writer, ctx);
    if (error != READSTAT_OK)
        goto cleanup;

    error = dta_emit_varlist(writer, ctx);
    if (error != READSTAT_OK)
        goto cleanup;
    
    error = dta_emit_srtlist(writer, ctx);
    if (error != READSTAT_OK)
        goto cleanup;
    
    error = dta_emit_fmtlist(writer, ctx);
    if (error != READSTAT_OK)
        goto cleanup;
    
    error = dta_emit_lbllist(writer, ctx);
    if (error != READSTAT_OK)
        goto cleanup;

cleanup:
    return error;
}

static readstat_error_t dta_emit_variable_labels(readstat_writer_t *writer, dta_ctx_t *ctx) {
    int i;
    for (i=0; i<ctx->nvar; i++) {
        readstat_variable_t *r_variable = readstat_get_variable(writer, i);

        strncpy(&ctx->variable_labels[ctx->variable_labels_entry_len*i], 
                r_variable->label, ctx->variable_labels_entry_len);
    }
    return readstat_write_bytes(writer, ctx->variable_labels, ctx->variable_labels_len);
}

static readstat_error_t dta_emit_expansion_fields(readstat_writer_t *writer, dta_ctx_t *ctx) {
    /* 5-byte terminator */
    return readstat_write_bytes(writer, "\0\0\0\0", sizeof("\0\0\0\0")); 
}

static readstat_error_t dta_emit_value_labels(readstat_writer_t *writer, dta_ctx_t *ctx) {
    readstat_error_t retval = READSTAT_OK;
    int i, j;
    int32_t *off = NULL;
    int32_t *val = NULL;
    char *txt = NULL;
    for (i=0; i<writer->label_sets_count; i++) {
        readstat_label_set_t *r_label_set = readstat_get_label_set(writer, i);
        int32_t n = r_label_set->value_labels_count;
        int32_t txtlen = 0;
        for (j=0; j<n; j++) {
            readstat_value_label_t *value_label = readstat_get_value_label(r_label_set, j);
            const char *label = value_label->label;
            txtlen += strlen(label) + 1;
        }
        dta_value_label_table_header_t table_header;
        memset(&table_header, 0, sizeof(dta_value_label_table_header_t));

        table_header.len = 8 + 8*n + txtlen;
        strncpy(table_header.labname, r_label_set->name, sizeof(table_header.labname));

        retval = readstat_write_bytes(writer, &table_header, sizeof(dta_value_label_table_header_t));
        if (retval != READSTAT_OK)
            goto cleanup;

        if (txtlen == 0) {
            retval = readstat_write_bytes(writer, &txtlen, sizeof(int32_t));
            if (retval != READSTAT_OK)
                goto cleanup;

            retval = readstat_write_bytes(writer, &txtlen, sizeof(int32_t));
            if (retval != READSTAT_OK)
                goto cleanup;

            continue;
        }

        off = realloc(off, 4*n);
        val = realloc(val, 4*n);
        txt = realloc(txt, txtlen);

        off_t offset = 0;

        for (j=0; j<n; j++) {
            readstat_value_label_t *value_label = readstat_get_value_label(r_label_set, j);
            const char *label = value_label->label;
            size_t label_data_len = strlen(label);
            off[j] = offset;
            val[j] = value_label->int32_key;
            memcpy(txt + offset, label, label_data_len);
            offset += label_data_len;

            txt[offset++] = '\0';                
        }

        retval = readstat_write_bytes(writer, &n, sizeof(int32_t));
        if (retval != READSTAT_OK)
            goto cleanup;

        retval = readstat_write_bytes(writer, &txtlen, sizeof(int32_t));
        if (retval != READSTAT_OK)
            goto cleanup;

        retval = readstat_write_bytes(writer, off, 4*n);
        if (retval != READSTAT_OK)
            goto cleanup;

        retval = readstat_write_bytes(writer, val, 4*n);
        if (retval != READSTAT_OK)
            goto cleanup;

        retval = readstat_write_bytes(writer, txt, txtlen);
        if (retval != READSTAT_OK)
            goto cleanup;
    }

cleanup:
    if (off)
        free(off);
    if (val)
        free(val);
    if (txt)
        free(txt);

    return retval;
}

static size_t dta_variable_width(readstat_types_t type, size_t user_width) {
    size_t len = 0;
    if (type == READSTAT_TYPE_STRING) {
        if (user_width > 244 || user_width == 0)
            user_width = 244;
        len = user_width;
    } else if (type == READSTAT_TYPE_DOUBLE) {
        len = 8;
    } else if (type == READSTAT_TYPE_FLOAT) {
        len = 4;
    } else if (type == READSTAT_TYPE_INT32) {
        len = 4;
    } else if (type == READSTAT_TYPE_INT16) {
        len = 2;
    } else if (type == READSTAT_TYPE_CHAR) {
        len = 1;
    }
    return len;
}

static readstat_error_t dta_begin_data(void *writer_ctx) {
    readstat_writer_t *writer = (readstat_writer_t *)writer_ctx;
    readstat_error_t error = READSTAT_OK;
    
    dta_ctx_t *ctx = NULL;
    dta_header_t header;
    memset(&header, 0, sizeof(dta_header_t));

    header.ds_format = 111;
    header.byteorder = machine_is_little_endian() ? DTA_LOHI : DTA_HILO;
    header.filetype  = 0x01;
    header.unused    = 0x00;
    header.nvar      = writer->variables_count;
    header.nobs      = writer->row_count;

    error = readstat_write_bytes(writer, &header, sizeof(dta_header_t));
    if (error != READSTAT_OK)
        goto cleanup;
    
    ctx = dta_ctx_init(header.nvar, header.nobs, header.byteorder, header.ds_format);
    
    error = dta_emit_header_data_label(writer);
    if (error != READSTAT_OK)
        goto cleanup;

    error = dta_emit_header_time_stamp(writer);
    if (error != READSTAT_OK)
        goto cleanup;

    error = dta_emit_descriptors(writer, ctx);
    if (error != READSTAT_OK)
        goto cleanup;

    error = dta_emit_variable_labels(writer, ctx);
    if (error != READSTAT_OK)
        goto cleanup;

    error = dta_emit_expansion_fields(writer, ctx);
    if (error != READSTAT_OK)
        goto cleanup;

cleanup:
    if (error != READSTAT_OK) {
        dta_ctx_free(ctx);
    } else {
        writer->module_ctx = ctx;
    }
    
    return error;
}

static readstat_error_t dta_write_char(void *row, const readstat_variable_t *var, char value) {
    if (var->type != READSTAT_TYPE_CHAR) {
        return READSTAT_ERROR_VALUE_TYPE_MISMATCH;
    }
    memcpy(row, &value, sizeof(char));
    return READSTAT_OK;
}

static readstat_error_t dta_write_int16(void *row, const readstat_variable_t *var, int16_t value) {
    if (var->type != READSTAT_TYPE_INT16) {
        return READSTAT_ERROR_VALUE_TYPE_MISMATCH;
    }
    memcpy(row, &value, sizeof(int16_t));
    return READSTAT_OK;
}

static readstat_error_t dta_write_int32(void *row, const readstat_variable_t *var, int32_t value) {
    if (var->type != READSTAT_TYPE_INT32) {
        return READSTAT_ERROR_VALUE_TYPE_MISMATCH;
    }
    memcpy(row, &value, sizeof(int32_t));
    return READSTAT_OK;
}

static readstat_error_t dta_write_float(void *row, const readstat_variable_t *var, float value) {
    if (var->type != READSTAT_TYPE_FLOAT) {
        return READSTAT_ERROR_VALUE_TYPE_MISMATCH;
    }
    memcpy(row, &value, sizeof(float));
    return READSTAT_OK;
}

static readstat_error_t dta_write_double(void *row, const readstat_variable_t *var, double value) {
    if (var->type != READSTAT_TYPE_DOUBLE) {
        return READSTAT_ERROR_VALUE_TYPE_MISMATCH;
    }
    memcpy(row, &value, sizeof(double));
    return READSTAT_OK;
}

static readstat_error_t dta_write_string(void *row, const readstat_variable_t *var, const char *value) {
    if (var->type != READSTAT_TYPE_STRING) {
        return READSTAT_ERROR_VALUE_TYPE_MISMATCH;
    }
    size_t value_len = var->width;
    if (value_len > 244)
        value_len = 244;
    if (value == NULL || value[0] == '\0') {
        memset(row, '\0', value_len);
    } else {
        strncpy((char *)row, value, value_len);
    }
    return READSTAT_OK;
}

static readstat_error_t dta_write_missing(void *row, const readstat_variable_t *var) {
    readstat_error_t retval = READSTAT_OK;
    if (var->type == READSTAT_TYPE_CHAR) {
        retval = dta_write_char(row, var, DTA_MISSING_CHAR);
    } else if (var->type == READSTAT_TYPE_INT16) {
        retval = dta_write_int16(row, var, DTA_MISSING_INT16);
    } else if (var->type == READSTAT_TYPE_INT32) {
        retval = dta_write_int32(row, var, DTA_MISSING_INT32);
    } else if (var->type == READSTAT_TYPE_FLOAT) {
        retval = dta_write_float(row, var, DTA_MISSING_FLOAT);
    } else if (var->type == READSTAT_TYPE_DOUBLE) {
        retval = dta_write_double(row, var, DTA_MISSING_DOUBLE);
    } else if (var->type == READSTAT_TYPE_STRING) {
        retval = dta_write_string(row, var, NULL);
    }
    return retval;
}

static readstat_error_t dta_end_data(void *writer_ctx) {
    readstat_writer_t *writer = (readstat_writer_t *)writer_ctx;
    readstat_error_t retval = READSTAT_OK;

    retval = dta_emit_value_labels(writer, writer->module_ctx);

    dta_ctx_free(writer->module_ctx);
    writer->module_ctx = NULL;

    return retval;
}

readstat_error_t readstat_begin_writing_dta(readstat_writer_t *writer, void *user_ctx,
        const char *file_label, long row_count) {
    snprintf(writer->file_label, sizeof(writer->file_label), "%s", file_label);
    writer->row_count = row_count;
    writer->user_ctx = user_ctx;

    writer->callbacks.variable_width = &dta_variable_width;
    writer->callbacks.write_char = &dta_write_char;
    writer->callbacks.write_int16 = &dta_write_int16;
    writer->callbacks.write_int32 = &dta_write_int32;
    writer->callbacks.write_float = &dta_write_float;
    writer->callbacks.write_double = &dta_write_double;
    writer->callbacks.write_string = &dta_write_string;
    writer->callbacks.write_missing = &dta_write_missing;
    writer->callbacks.begin_data = &dta_begin_data;
    writer->callbacks.end_data = &dta_end_data;

    return READSTAT_OK;
}
