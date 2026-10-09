#define _XOPEN_SOURCE 700 /* for localtime_r */
#include <stdlib.h>
#include <math.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>

#include "../readstat.h"
#include "../readstat_bits.h"
#include "../readstat_iconv.h"
#include "../readstat_writer.h"

#include "readstat_dta.h"

#define DTA_DEFAULT_DISPLAY_WIDTH_BYTE   8
#define DTA_DEFAULT_DISPLAY_WIDTH_INT16  8
#define DTA_DEFAULT_DISPLAY_WIDTH_INT32  12
#define DTA_DEFAULT_DISPLAY_WIDTH_FLOAT  9
#define DTA_DEFAULT_DISPLAY_WIDTH_DOUBLE 10
#define DTA_DEFAULT_DISPLAY_WIDTH_STRING 9

#define DTA_FILE_VERSION_MIN     104
#define DTA_FILE_VERSION_MAX     119
#define DTA_FILE_VERSION_DEFAULT 118

#define DTA_OLD_MAX_WIDTH    128
#define DTA_111_MAX_WIDTH    244
#define DTA_117_MAX_WIDTH   2045

/* Stata's name limits. Formats below 118 store names in the system code page
 * and allow 8 or 32 bytes; 118 and later store UTF-8 and allow 32 characters,
 * which may occupy up to 128 bytes. The on-disk fields are one byte wider so
 * that every name keeps its terminator. */
#define DTA_OLD_MAX_NAME_LEN       8
#define DTA_110_MAX_NAME_LEN      32
#define DTA_118_MAX_NAME_LEN     128
#define DTA_118_MAX_NAME_CHARS    32

/* Data set and variable labels are 80 characters in every format that has a
 * full-width label field (32 bytes in formats before 108); 118 and later
 * store them as UTF-8 in a 321-byte field. */
#define DTA_MAX_LABEL_CHARS       80

/* Stata refuses notes longer than this many bytes */
#define DTA_MAX_NOTE_LEN       67784

/* Stata's smallest non-missing integers. The largest are in readstat_dta.h;
 * the most negative bit patterns are not valid Stata values. */
#define DTA_MIN_INT8              -127
#define DTA_MIN_INT16           -32767
#define DTA_MIN_INT32      -2147483647

static readstat_error_t dta_113_write_missing_numeric(void *row, const readstat_variable_t *var);

static size_t dta_utf8_char_count(const char *string) {
    size_t count = 0;
    const unsigned char *p = (const unsigned char *)string;
    for (; *p; p++) {
        if ((*p & 0xC0) != 0x80)
            count++;
    }
    return count;
}

static readstat_error_t dta_write_tag(readstat_writer_t *writer, dta_ctx_t *ctx, const char *tag) {
    if (!ctx->file_is_xmlish)
        return READSTAT_OK;

    return readstat_write_string(writer, tag);
}

static readstat_error_t dta_write_chunk(readstat_writer_t *writer, dta_ctx_t *ctx, 
        const char *start_tag,
        const void *bytes, size_t len,
        const char *end_tag) {
    readstat_error_t error = READSTAT_OK;

    if ((error = dta_write_tag(writer, ctx, start_tag)) != READSTAT_OK)
        goto cleanup;

    if ((error = readstat_write_bytes(writer, bytes, len)) != READSTAT_OK)
        goto cleanup;

    if ((error = dta_write_tag(writer, ctx, end_tag)) != READSTAT_OK)
        goto cleanup;

cleanup:
    return error;
}


static readstat_error_t dta_emit_header_data_label(readstat_writer_t *writer, dta_ctx_t *ctx) {
    readstat_error_t error = READSTAT_OK;
    char *data_label = NULL;
    if ((error = dta_write_tag(writer, ctx, "<label>")) != READSTAT_OK)
        goto cleanup;

    if (ctx->data_label_len_len) {
        if (ctx->data_label_len_len == 1) {
            uint8_t len  = strlen(writer->file_label);
            if ((error = readstat_write_bytes(writer, &len, sizeof(uint8_t))) != READSTAT_OK)
                goto cleanup;
        } else if (ctx->data_label_len_len == 2) {
            uint16_t len  = strlen(writer->file_label);
            if ((error = readstat_write_bytes(writer, &len, sizeof(uint16_t))) != READSTAT_OK)
                goto cleanup;
        }
        if ((error = readstat_write_string(writer, writer->file_label)) != READSTAT_OK)
            goto cleanup;
    } else {
        /* The label was checked against the field width in dta_validate_metadata,
         * so the copy always keeps its terminator */
        data_label = calloc(1, ctx->data_label_len);
        if (data_label == NULL) {
            error = READSTAT_ERROR_MALLOC;
            goto cleanup;
        }
        strncpy(data_label, writer->file_label, ctx->data_label_len - 1);

        if ((error = readstat_write_bytes(writer, data_label, ctx->data_label_len)) != READSTAT_OK)
            goto cleanup;
    }

    if ((error = dta_write_tag(writer, ctx, "</label>")) != READSTAT_OK)
        goto cleanup;

cleanup:
    if (data_label)
        free(data_label);

    return error;
}

static readstat_error_t dta_emit_header_time_stamp(readstat_writer_t *writer, dta_ctx_t *ctx) {
    if (!ctx->timestamp_len)
        return READSTAT_OK;

    readstat_error_t error = READSTAT_OK;
    char *timestamp = calloc(1, ctx->timestamp_len);
    uint8_t actual_timestamp_len = 0;
    /* There are locale/portability issues with strftime so hack something up */
    char months[][4] = {
        "Jan", "Feb", "Mar", "Apr", "May", "Jun",
        "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };

    if (!timestamp) {
        error = READSTAT_ERROR_MALLOC;
        goto cleanup;
    }

    /* A timestamp of zero means "no timestamp": Stata writes a blank stamp
     * (all NULs in the old formats, a zero-length string in 117 and later)
     * rather than the epoch in the local time zone. */
    if (writer->timestamp != 0) {
        time_t now = writer->timestamp;
        struct tm time_buf;
#if !defined _MSC_VER
        struct tm *time_s = localtime_r(&now, &time_buf);
#else
        errno_t time_err = localtime_s(&time_buf, &now);
        struct tm *time_s = (time_err == 0) ? &time_buf : NULL;
#endif

        if (!time_s || time_s->tm_mon < 0 || time_s->tm_mon > 11) {
            error = READSTAT_ERROR_BAD_TIMESTAMP_VALUE;
            goto cleanup;
        }

        int len = snprintf(timestamp, ctx->timestamp_len, "%02d %3s %04d %02d:%02d",
                time_s->tm_mday, months[time_s->tm_mon], time_s->tm_year + 1900,
                time_s->tm_hour, time_s->tm_min);
        if (len <= 0 || (size_t)len >= ctx->timestamp_len) {
            error = READSTAT_ERROR_BAD_TIMESTAMP_VALUE;
            goto cleanup;
        }
        actual_timestamp_len = len;
    }

    if (ctx->file_is_xmlish) {
        if ((error = dta_write_tag(writer, ctx, "<timestamp>")) != READSTAT_OK)
            goto cleanup;

        if ((error = readstat_write_bytes(writer, &actual_timestamp_len, sizeof(uint8_t))) != READSTAT_OK)
            goto cleanup;

        if ((error = readstat_write_bytes(writer, timestamp, actual_timestamp_len)) != READSTAT_OK)
            goto cleanup;

        if ((error = dta_write_tag(writer, ctx, "</timestamp>")) != READSTAT_OK)
            goto cleanup;
    } else {
        error = readstat_write_bytes(writer, timestamp, ctx->timestamp_len);
    }

cleanup:
    free(timestamp);
    return error;
}

static readstat_error_t dta_111_typecode_for_variable(readstat_variable_t *r_variable,
        uint16_t *out_typecode) {
    readstat_error_t retval = READSTAT_OK;
    size_t max_len = r_variable->storage_width;
    uint16_t typecode = 0;
    switch (r_variable->type) {
        case READSTAT_TYPE_INT8: 
            typecode = DTA_111_TYPE_CODE_INT8; break;
        case READSTAT_TYPE_INT16:
            typecode = DTA_111_TYPE_CODE_INT16; break;
        case READSTAT_TYPE_INT32:
            typecode = DTA_111_TYPE_CODE_INT32; break;
        case READSTAT_TYPE_FLOAT:
            typecode = DTA_111_TYPE_CODE_FLOAT; break;
        case READSTAT_TYPE_DOUBLE:
            typecode = DTA_111_TYPE_CODE_DOUBLE; break;
        case READSTAT_TYPE_STRING:
            typecode = max_len; break;
        case READSTAT_TYPE_STRING_REF:
            retval = READSTAT_ERROR_STRING_REFS_NOT_SUPPORTED; break;
    }
    if (out_typecode && retval == READSTAT_OK)
        *out_typecode = typecode;

    return retval;
}

static readstat_error_t dta_117_typecode_for_variable(readstat_variable_t *r_variable, uint16_t *out_typecode) {
    readstat_error_t retval = READSTAT_OK;
    size_t max_len = r_variable->storage_width;
    uint16_t typecode = 0;
    switch (r_variable->type) {
        case READSTAT_TYPE_INT8: 
            typecode = DTA_117_TYPE_CODE_INT8; break;
        case READSTAT_TYPE_INT16:
            typecode = DTA_117_TYPE_CODE_INT16; break;
        case READSTAT_TYPE_INT32:
            typecode = DTA_117_TYPE_CODE_INT32; break;
        case READSTAT_TYPE_FLOAT:
            typecode = DTA_117_TYPE_CODE_FLOAT; break;
        case READSTAT_TYPE_DOUBLE:
            typecode = DTA_117_TYPE_CODE_DOUBLE; break;
        case READSTAT_TYPE_STRING:
            typecode = max_len; break;
        case READSTAT_TYPE_STRING_REF:
            typecode = DTA_117_TYPE_CODE_STRL; break;
    }
    if (out_typecode)
        *out_typecode = typecode;

    return retval;
}

static readstat_error_t dta_old_typecode_for_variable(readstat_variable_t *r_variable, uint16_t *out_typecode) {
    readstat_error_t retval = READSTAT_OK;
    size_t max_len = r_variable->storage_width;
    uint16_t typecode = 0;
    switch (r_variable->type) {
        case READSTAT_TYPE_INT8: 
            typecode = DTA_OLD_TYPE_CODE_INT8; break;
        case READSTAT_TYPE_INT16:
            typecode = DTA_OLD_TYPE_CODE_INT16; break;
        case READSTAT_TYPE_INT32:
            typecode = DTA_OLD_TYPE_CODE_INT32; break;
        case READSTAT_TYPE_FLOAT:
            typecode = DTA_OLD_TYPE_CODE_FLOAT; break;
        case READSTAT_TYPE_DOUBLE:
            typecode = DTA_OLD_TYPE_CODE_DOUBLE; break;
        case READSTAT_TYPE_STRING:
            typecode = max_len + 0x7F; break;
        case READSTAT_TYPE_STRING_REF:
            retval = READSTAT_ERROR_STRING_REFS_NOT_SUPPORTED; break;
    }
    if (out_typecode && retval == READSTAT_OK)
        *out_typecode = typecode;

    return retval;
}

static readstat_error_t dta_typecode_for_variable(readstat_variable_t *r_variable, 
        int typlist_version, uint16_t *typecode) {
    if (typlist_version == 111) {
        return dta_111_typecode_for_variable(r_variable, typecode);
    }
    if (typlist_version == 117) {
        return dta_117_typecode_for_variable(r_variable, typecode);
    } 
    return dta_old_typecode_for_variable(r_variable, typecode);
}

static readstat_error_t dta_emit_typlist(readstat_writer_t *writer, dta_ctx_t *ctx) {
    readstat_error_t error = READSTAT_OK;
    int i;

    if ((error = dta_write_tag(writer, ctx, "<variable_types>")) != READSTAT_OK)
        goto cleanup;

    for (i=0; i<ctx->nvar; i++) {
        readstat_variable_t *r_variable = readstat_get_variable(writer, i);
        uint16_t typecode = 0;
        error = dta_typecode_for_variable(r_variable, ctx->typlist_version, &typecode);
        if (error != READSTAT_OK)
            goto cleanup;

        ctx->typlist[i] = typecode;
    }

    for (i=0; i<ctx->nvar; i++) {
        if (ctx->typlist_entry_len == 1) {
            uint8_t byte = ctx->typlist[i];
            error = readstat_write_bytes(writer, &byte, sizeof(uint8_t));
        } else if (ctx->typlist_entry_len == 2) {
            uint16_t val = ctx->typlist[i];
            error = readstat_write_bytes(writer, &val, sizeof(uint16_t));
        }
        if (error != READSTAT_OK)
            goto cleanup;
    }

    if ((error = dta_write_tag(writer, ctx, "</variable_types>")) != READSTAT_OK)
        goto cleanup;

cleanup:
    return error;
}

static readstat_error_t dta_validate_name_chars(const char *name, int unicode) {
    /* TODO check Unicode class */
    int j;
    for (j=0; name[j]; j++) {
        if (((unsigned char)name[j] < 0x80 || !unicode) && name[j] != '_' &&
                !(name[j] >= 'a' && name[j] <= 'z') &&
                !(name[j] >= 'A' && name[j] <= 'Z') &&
                !(name[j] >= '0' && name[j] <= '9')) {
            return READSTAT_ERROR_NAME_CONTAINS_ILLEGAL_CHARACTER;
        }
    }
    char first_char = name[0];
    if (((unsigned char)first_char < 0x80 || !unicode) && first_char != '_' &&
            !(first_char >= 'a' && first_char <= 'z') &&
            !(first_char >= 'A' && first_char <= 'Z')) {
        return READSTAT_ERROR_NAME_BEGINS_WITH_ILLEGAL_CHARACTER;
    }
    return READSTAT_OK;
}

static readstat_error_t dta_validate_name_unreserved(const char *name) {
    if (strcmp(name, "_all") == 0 || strcmp(name, "_b") == 0 ||
            strcmp(name, "byte") == 0 || strcmp(name, "_coef") == 0 ||
            strcmp(name, "_cons") == 0 || strcmp(name, "double") == 0 ||
            strcmp(name, "float") == 0 || strcmp(name, "if") == 0 ||
            strcmp(name, "in") == 0 || strcmp(name, "int") == 0 ||
            strcmp(name, "long") == 0 || strcmp(name, "_n") == 0 ||
            strcmp(name, "_N") == 0 || strcmp(name, "_pi") == 0 ||
            strcmp(name, "_pred") == 0 || strcmp(name, "_rc") == 0 ||
            strcmp(name, "_skip") == 0 || strcmp(name, "strL") == 0 ||
            strcmp(name, "using") == 0 || strcmp(name, "with") == 0) {
        return READSTAT_ERROR_NAME_IS_RESERVED_WORD;
    }
    /* Only the storage types str1 through str2045 are reserved; names such
     * as str1abc or str3000 are legal, so the whole name must be consumed */
    int len = 0, consumed = 0;
    if (sscanf(name, "str%d%n", &len, &consumed) == 1 && name[consumed] == '\0'
            && len >= 1 && len <= DTA_117_MAX_WIDTH)
        return READSTAT_ERROR_NAME_IS_RESERVED_WORD;

    return READSTAT_OK;
}

/* Length and character checks shared by variable names and value-label names.
 * `unicode' selects the 118+ rules: at most DTA_118_MAX_NAME_CHARS UTF-8
 * characters within max_len bytes. */
static readstat_error_t dta_validate_name_length_and_chars(const char *name, int unicode, size_t max_len) {
    size_t len = strlen(name);

    if (len == 0)
        return READSTAT_ERROR_NAME_IS_ZERO_LENGTH;

    if (len > max_len)
        return READSTAT_ERROR_NAME_IS_TOO_LONG;

    if (unicode && dta_utf8_char_count(name) > DTA_118_MAX_NAME_CHARS)
        return READSTAT_ERROR_NAME_IS_TOO_LONG;

    return dta_validate_name_chars(name, unicode);
}

static readstat_error_t dta_validate_name(const char *name, int unicode, size_t max_len) {
    readstat_error_t error = READSTAT_OK;

    if ((error = dta_validate_name_length_and_chars(name, unicode, max_len)) != READSTAT_OK)
        return error;

    return dta_validate_name_unreserved(name);
}

/* Use variable->name rather than readstat_variable_get_name(), which returns
 * NULL for an empty name */
static readstat_error_t dta_variable_ok(const readstat_variable_t *variable,
        int unicode, size_t max_name_len, size_t max_string_width) {
    readstat_error_t error = dta_validate_name(variable->name, unicode, max_name_len);
    if (error != READSTAT_OK)
        return error;

    if (variable->type == READSTAT_TYPE_STRING && variable->user_width > max_string_width)
        return READSTAT_ERROR_BAD_STRING_WIDTH;

    return READSTAT_OK;
}

static readstat_error_t dta_old_variable_ok(const readstat_variable_t *variable) {
    return dta_variable_ok(variable, 0, DTA_OLD_MAX_NAME_LEN, DTA_OLD_MAX_WIDTH);
}

static readstat_error_t dta_110_variable_ok(const readstat_variable_t *variable) {
    return dta_variable_ok(variable, 0, DTA_110_MAX_NAME_LEN, DTA_OLD_MAX_WIDTH);
}

static readstat_error_t dta_111_variable_ok(const readstat_variable_t *variable) {
    return dta_variable_ok(variable, 0, DTA_110_MAX_NAME_LEN, DTA_111_MAX_WIDTH);
}

static readstat_error_t dta_117_variable_ok(const readstat_variable_t *variable) {
    return dta_variable_ok(variable, 0, DTA_110_MAX_NAME_LEN, DTA_117_MAX_WIDTH);
}

static readstat_error_t dta_118_variable_ok(const readstat_variable_t *variable) {
    return dta_variable_ok(variable, 1, DTA_118_MAX_NAME_LEN, DTA_117_MAX_WIDTH);
}

static readstat_error_t dta_emit_varlist(readstat_writer_t *writer, dta_ctx_t *ctx) {
    readstat_error_t error = READSTAT_OK;
    int i;

    if ((error = dta_write_tag(writer, ctx, "<varnames>")) != READSTAT_OK)
        goto cleanup;

    for (i=0; i<ctx->nvar; i++) {
        readstat_variable_t *r_variable = readstat_get_variable(writer, i);

        strncpy(&ctx->varlist[ctx->variable_name_len*i], 
                r_variable->name, ctx->variable_name_len);
    }

    if ((error = readstat_write_bytes(writer, ctx->varlist, ctx->varlist_len)) != READSTAT_OK)
        goto cleanup;

    if ((error = dta_write_tag(writer, ctx, "</varnames>")) != READSTAT_OK)
        goto cleanup;

cleanup:
    return error;
}

static readstat_error_t dta_emit_srtlist(readstat_writer_t *writer, dta_ctx_t *ctx) {
    readstat_error_t error = READSTAT_OK;

    if ((error = dta_write_tag(writer, ctx, "<sortlist>")) != READSTAT_OK)
        goto cleanup;

    memset(ctx->srtlist, '\0', ctx->srtlist_len);

    if ((error = readstat_write_bytes(writer, ctx->srtlist, ctx->srtlist_len)) != READSTAT_OK)
        goto cleanup;

    if ((error = dta_write_tag(writer, ctx, "</sortlist>")) != READSTAT_OK)
        goto cleanup;

cleanup:
    return error;
}

static readstat_error_t dta_emit_fmtlist(readstat_writer_t *writer, dta_ctx_t *ctx) {
    readstat_error_t error = READSTAT_OK;
    int i;

    if ((error = dta_write_tag(writer, ctx, "<formats>")) != READSTAT_OK)
        goto cleanup;

    for (i=0; i<ctx->nvar; i++) {
        readstat_variable_t *r_variable = readstat_get_variable(writer, i);

        if (r_variable->format[0]) {
            strncpy(&ctx->fmtlist[ctx->fmtlist_entry_len*i],
                    r_variable->format, ctx->fmtlist_entry_len);
        } else {
            char format_letter = 'g';
            int display_width = r_variable->display_width;
            if (readstat_type_class(r_variable->type) == READSTAT_TYPE_CLASS_STRING) {
                format_letter = 's';
            }
            if (!display_width) {
                if (r_variable->type == READSTAT_TYPE_INT8) {
                    display_width = DTA_DEFAULT_DISPLAY_WIDTH_BYTE;
                } else if (r_variable->type == READSTAT_TYPE_INT16) {
                    display_width = DTA_DEFAULT_DISPLAY_WIDTH_INT16;
                } else if (r_variable->type == READSTAT_TYPE_INT32) {
                    display_width = DTA_DEFAULT_DISPLAY_WIDTH_INT32;
                } else if (r_variable->type == READSTAT_TYPE_FLOAT) {
                    display_width = DTA_DEFAULT_DISPLAY_WIDTH_FLOAT;
                } else if (r_variable->type == READSTAT_TYPE_DOUBLE) {
                    display_width = DTA_DEFAULT_DISPLAY_WIDTH_DOUBLE;
                } else {
                    display_width = DTA_DEFAULT_DISPLAY_WIDTH_STRING;
                }
            }
            char format[64];
            if (format_letter == 'g') {
                snprintf(format, sizeof(format), "%%%s%d.0g", r_variable->alignment == READSTAT_ALIGNMENT_LEFT ? "-" : "",
                        display_width);
            } else {
                snprintf(format, sizeof(format), "%%%s%ds",
                        r_variable->alignment == READSTAT_ALIGNMENT_LEFT ? "-" : "",
                        display_width);
            }
            strncpy(&ctx->fmtlist[ctx->fmtlist_entry_len*i],
                    format, ctx->fmtlist_entry_len);
        }
    }

    if ((error = readstat_write_bytes(writer, ctx->fmtlist, ctx->fmtlist_len)) != READSTAT_OK)
        goto cleanup;

    if ((error = dta_write_tag(writer, ctx, "</formats>")) != READSTAT_OK)
        goto cleanup;

cleanup:
    return error;
}

static readstat_error_t dta_emit_lbllist(readstat_writer_t *writer, dta_ctx_t *ctx) {
    readstat_error_t error = READSTAT_OK;
    int i;

    if ((error = dta_write_tag(writer, ctx, "<value_label_names>")) != READSTAT_OK)
        goto cleanup;

    for (i=0; i<ctx->nvar; i++) {
        readstat_variable_t *r_variable = readstat_get_variable(writer, i);

        if (r_variable->label_set) {
            strncpy(&ctx->lbllist[ctx->lbllist_entry_len*i], 
                    r_variable->label_set->name, ctx->lbllist_entry_len);
        } else {
            memset(&ctx->lbllist[ctx->lbllist_entry_len*i], '\0', ctx->lbllist_entry_len);
        }
    }
    if ((error = readstat_write_bytes(writer, ctx->lbllist, ctx->lbllist_len)) != READSTAT_OK)
        goto cleanup;

    if ((error = dta_write_tag(writer, ctx, "</value_label_names>")) != READSTAT_OK)
        goto cleanup;

cleanup:
    return error;
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
    readstat_error_t error = READSTAT_OK;
    int i;

    if ((error = dta_write_tag(writer, ctx, "<variable_labels>")) != READSTAT_OK)
        goto cleanup;

    for (i=0; i<ctx->nvar; i++) {
        readstat_variable_t *r_variable = readstat_get_variable(writer, i);

        strncpy(&ctx->variable_labels[ctx->variable_labels_entry_len*i], 
                r_variable->label, ctx->variable_labels_entry_len);
    }
    if ((error = readstat_write_bytes(writer, ctx->variable_labels, ctx->variable_labels_len)) != READSTAT_OK)
        goto cleanup;

    if ((error = dta_write_tag(writer, ctx, "</variable_labels>")) != READSTAT_OK)
        goto cleanup;

cleanup:
    return error;
}

/* Size in bytes of the expansion field holding one _dta[name] characteristic */
static size_t dta_characteristic_payload_len(dta_ctx_t *ctx, size_t contents_len) {
    return 2 * ctx->ch_metadata_len + contents_len + 1;
}

/* Notes are stored as the characteristics _dta[note0] = "<count>",
 * _dta[note1] = "<first note>", and so on; Stata's `notes' command
 * enumerates from note0, so it must be present. */
static void dta_note_count_string(readstat_writer_t *writer, char *buffer, size_t buffer_len) {
    snprintf(buffer, buffer_len, "%ld", writer->notes_count);
}

static readstat_error_t dta_emit_characteristic(readstat_writer_t *writer, dta_ctx_t *ctx,
        char *buffer, const char *name, const char *contents) {
    readstat_error_t error = READSTAT_OK;
    size_t len = strlen(contents);

    if (ctx->file_is_xmlish) {
        error = dta_write_tag(writer, ctx, "<ch>");
    } else {
        char data_type = 1;
        error = readstat_write_bytes(writer, &data_type, 1);
    }
    if (error != READSTAT_OK)
        goto cleanup;

    if (ctx->expansion_len_len == 2) {
        uint16_t len16 = dta_characteristic_payload_len(ctx, len);
        error = readstat_write_bytes(writer, &len16, sizeof(len16));
    } else if (ctx->expansion_len_len == 4) {
        uint32_t len32 = dta_characteristic_payload_len(ctx, len);
        error = readstat_write_bytes(writer, &len32, sizeof(len32));
    }
    if (error != READSTAT_OK)
        goto cleanup;

    memset(buffer, '\0', ctx->ch_metadata_len);
    strncpy(buffer, "_dta", ctx->ch_metadata_len - 1);

    error = readstat_write_bytes(writer, buffer, ctx->ch_metadata_len);
    if (error != READSTAT_OK)
        goto cleanup;

    memset(buffer, '\0', ctx->ch_metadata_len);
    strncpy(buffer, name, ctx->ch_metadata_len - 1);

    error = readstat_write_bytes(writer, buffer, ctx->ch_metadata_len);
    if (error != READSTAT_OK)
        goto cleanup;

    error = readstat_write_bytes(writer, contents, len + 1);
    if (error != READSTAT_OK)
        goto cleanup;

    error = dta_write_tag(writer, ctx, "</ch>");

cleanup:
    return error;
}

static readstat_error_t dta_emit_characteristics(readstat_writer_t *writer, dta_ctx_t *ctx) {
    readstat_error_t error = READSTAT_OK;
    int i;
    char *buffer = NULL;
    char name[32];
    char count[32];

    if (ctx->expansion_len_len == 0)
        return READSTAT_OK;

    if ((error = dta_write_tag(writer, ctx, "<characteristics>")) != READSTAT_OK)
        return error;

    if ((buffer = malloc(ctx->ch_metadata_len)) == NULL)
        return READSTAT_ERROR_MALLOC;

    if (writer->notes_count > 0) {
        dta_note_count_string(writer, count, sizeof(count));
        if ((error = dta_emit_characteristic(writer, ctx, buffer, "note0", count)) != READSTAT_OK)
            goto cleanup;
    }

    for (i=0; i<writer->notes_count; i++) {
        snprintf(name, sizeof(name), "note%d", i+1);
        if ((error = dta_emit_characteristic(writer, ctx, buffer, name, writer->notes[i])) != READSTAT_OK)
            goto cleanup;
    }

    if (ctx->file_is_xmlish) {
        error = dta_write_tag(writer, ctx, "</characteristics>");
    } else {
        error = readstat_write_zeros(writer, 1 + ctx->expansion_len_len);
    }
    if (error != READSTAT_OK)
        goto cleanup;

cleanup:
    free(buffer);
    return error;
}

static readstat_error_t dta_117_emit_strl_header(readstat_writer_t *writer, readstat_string_ref_t *ref) {
    dta_117_strl_header_t header = {
        .v = ref->first_v,
        .o = ref->first_o,
        .type = DTA_GSO_TYPE_ASCII,
        .len = ref->len
    };

    return readstat_write_bytes(writer, &header, SIZEOF_DTA_117_STRL_HEADER_T);
}

static readstat_error_t dta_118_emit_strl_header(readstat_writer_t *writer, readstat_string_ref_t *ref) {
    dta_118_strl_header_t header = {
        .v = ref->first_v,
        .o = ref->first_o,
        .type = DTA_GSO_TYPE_ASCII,
        .len = ref->len
    };

    return readstat_write_bytes(writer, &header, SIZEOF_DTA_118_STRL_HEADER_T);
}

static readstat_error_t dta_emit_strls(readstat_writer_t *writer, dta_ctx_t *ctx) {
    if (!ctx->file_is_xmlish)
        return READSTAT_OK;

    readstat_error_t retval = READSTAT_OK;

    retval = readstat_write_string(writer, "<strls>");
    if (retval != READSTAT_OK)
        goto cleanup;

    int i;
    for (i=0; i<writer->string_refs_count; i++) {
        readstat_string_ref_t *ref = writer->string_refs[i];

        /* Only references that appear in the data may be defined, and the
         * <map> written before the data already accounts for every
         * registered ref, so a never-inserted ref cannot simply be skipped */
        if (ref->first_v == -1) {
            retval = READSTAT_ERROR_UNUSED_STRING_REF;
            goto cleanup;
        }

        retval = readstat_write_string(writer, "GSO");
        if (retval != READSTAT_OK)
            goto cleanup;

        if (ctx->strl_o_len > 4) {
            retval = dta_118_emit_strl_header(writer, ref);
        } else {
            retval = dta_117_emit_strl_header(writer, ref);
        }
        if (retval != READSTAT_OK)
            goto cleanup;

        retval = readstat_write_bytes(writer, &ref->data[0], ref->len);
        if (retval != READSTAT_OK)
            goto cleanup;
    }

    retval = readstat_write_string(writer, "</strls>");
    if (retval != READSTAT_OK)
        goto cleanup;

cleanup:
    return retval;
}

/* Formats 105 and earlier: n (int16), 9-byte label name, 1 byte of padding,
 * n int16 codes, then n 8-byte labels (the layout pandas and R foreign use). */
static readstat_error_t dta_old_emit_value_labels(readstat_writer_t *writer, dta_ctx_t *ctx) {
    readstat_error_t retval = READSTAT_OK;
    int i, j;
    char labname[9+1];
    char *table_buffer = NULL;
    for (i=0; i<writer->label_sets_count; i++) {
        readstat_label_set_t *r_label_set = readstat_get_label_set(writer, i);
        int32_t n = r_label_set->value_labels_count;
        size_t table_len = 10 * n;
        if (n > INT16_MAX) {
            retval = READSTAT_ERROR_TOO_MANY_COLUMNS;
            goto cleanup;
        }
        for (j=0; j<n; j++) {
            readstat_value_label_t *value_label = readstat_get_value_label(r_label_set, j);
            if (value_label->tag) {
                retval = READSTAT_ERROR_TAGGED_VALUES_NOT_SUPPORTED;
                goto cleanup;
            }
            if (value_label->int32_key < INT16_MIN || value_label->int32_key > INT16_MAX) {
                retval = READSTAT_ERROR_NUMERIC_VALUE_IS_OUT_OF_RANGE;
                goto cleanup;
            }
        }
        int16_t n16 = n;
        retval = readstat_write_bytes(writer, &n16, sizeof(int16_t));
        if (retval != READSTAT_OK)
            goto cleanup;

        memset(labname, 0, sizeof(labname));
        strncpy(labname, r_label_set->name, ctx->value_label_table_labname_len - 1);

        retval = readstat_write_bytes(writer, labname, ctx->value_label_table_labname_len
                + ctx->value_label_table_padding_len);
        if (retval != READSTAT_OK)
            goto cleanup;

        if (n == 0)
            continue;

        char *new_table_buffer = realloc(table_buffer, table_len);
        if (new_table_buffer == NULL) {
            retval = READSTAT_ERROR_MALLOC;
            goto cleanup;
        }
        table_buffer = new_table_buffer;
        memset(table_buffer, 0, table_len);

        for (j=0; j<n; j++) {
            readstat_value_label_t *value_label = readstat_get_value_label(r_label_set, j);
            int16_t code = value_label->int32_key;
            size_t len = value_label->label_len;
            if (len > 8)
                len = 8;
            memcpy(&table_buffer[2*j], &code, sizeof(int16_t));
            memcpy(&table_buffer[2*n + 8*j], value_label->label, len);
        }

        retval = readstat_write_bytes(writer, table_buffer, table_len);
        if (retval != READSTAT_OK)
            goto cleanup;
    }
cleanup:
    if (table_buffer)
        free(table_buffer);

    return retval;
}

static int dta_compare_value_labels(const readstat_value_label_t *vl1, const readstat_value_label_t *vl2) {
    if (vl1->tag) {
        if (vl2->tag) {
            return vl1->tag - vl2->tag;
        }
        return 1;
    }
    if (vl2->tag) {
        return -1;
    }
    /* Stata binary-searches val[], so it must be strictly ascending; a plain
     * subtraction overflows for keys of opposite sign near the int32 limits */
    return (vl1->int32_key > vl2->int32_key) - (vl1->int32_key < vl2->int32_key);
}

static readstat_error_t dta_emit_value_labels(readstat_writer_t *writer, dta_ctx_t *ctx) {
    if (ctx->value_label_table_len_len == 2)
        return dta_old_emit_value_labels(writer, ctx);

    readstat_error_t retval = READSTAT_OK;
    int i, j;
    int32_t *off = NULL;
    int32_t *val = NULL;
    char *txt = NULL;
    char *labname = calloc(1, ctx->value_label_table_labname_len + ctx->value_label_table_padding_len);
    if (labname == NULL) {
        retval = READSTAT_ERROR_MALLOC;
        goto cleanup;
    }

    retval = dta_write_tag(writer, ctx, "<value_labels>");
    if (retval != READSTAT_OK)
        goto cleanup;

    for (i=0; i<writer->label_sets_count; i++) {
        readstat_label_set_t *r_label_set = readstat_get_label_set(writer, i);
        int32_t n = r_label_set->value_labels_count;
        int64_t txtlen64 = 0;
        for (j=0; j<n; j++) {
            readstat_value_label_t *value_label = readstat_get_value_label(r_label_set, j);
            txtlen64 += value_label->label_len + 1;
        }
        if (8 + 8LL*n + txtlen64 > INT32_MAX) {
            /* The table length and text length are stored as int32 */
            retval = READSTAT_ERROR_STRING_VALUE_IS_TOO_LONG;
            goto cleanup;
        }
        int32_t txtlen = txtlen64;

        retval = dta_write_tag(writer, ctx, "<lbl>");
        if (retval != READSTAT_OK)
            goto cleanup;

        int32_t table_len = 8 + 8*n + txtlen;
        retval = readstat_write_bytes(writer, &table_len, sizeof(int32_t));
        if (retval != READSTAT_OK)
            goto cleanup;

        strncpy(labname, r_label_set->name, ctx->value_label_table_labname_len);

        retval = readstat_write_bytes(writer, labname, ctx->value_label_table_labname_len 
                + ctx->value_label_table_padding_len);
        if (retval != READSTAT_OK)
            goto cleanup;

        if (txtlen == 0) {
            retval = readstat_write_bytes(writer, &txtlen, sizeof(int32_t));
            if (retval != READSTAT_OK)
                goto cleanup;

            retval = readstat_write_bytes(writer, &txtlen, sizeof(int32_t));
            if (retval != READSTAT_OK)
                goto cleanup;

            retval = dta_write_tag(writer, ctx, "</lbl>");
            if (retval != READSTAT_OK)
                goto cleanup;

            continue;
        }

        void *new_buf = NULL;
        if ((new_buf = realloc(off, 4*n)) == NULL) {
            retval = READSTAT_ERROR_MALLOC;
            goto cleanup;
        }
        off = new_buf;
        if ((new_buf = realloc(val, 4*n)) == NULL) {
            retval = READSTAT_ERROR_MALLOC;
            goto cleanup;
        }
        val = new_buf;
        if ((new_buf = realloc(txt, txtlen)) == NULL) {
            retval = READSTAT_ERROR_MALLOC;
            goto cleanup;
        }
        txt = new_buf;

        readstat_off_t offset = 0;

        readstat_sort_label_set(r_label_set, &dta_compare_value_labels);

        for (j=0; j<n; j++) {
            readstat_value_label_t *value_label = readstat_get_value_label(r_label_set, j);
            const char *label = value_label->label;
            size_t label_data_len = value_label->label_len;
            off[j] = offset;
            if (value_label->tag) {
                if (writer->version < 113) {
                    retval = READSTAT_ERROR_TAGGED_VALUES_NOT_SUPPORTED;
                    goto cleanup;
                }
                val[j] = DTA_113_MISSING_INT32_A + (value_label->tag - 'a');
            } else {
                val[j] = value_label->int32_key;
            }
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

        retval = dta_write_tag(writer, ctx, "</lbl>");
        if (retval != READSTAT_OK)
            goto cleanup;
    }

    retval = dta_write_tag(writer, ctx, "</value_labels>");
    if (retval != READSTAT_OK)
        goto cleanup;

cleanup:
    if (off)
        free(off);
    if (val)
        free(val);
    if (txt)
        free(txt);
    if (labname)
        free(labname);

    return retval;
}

static size_t dta_numeric_variable_width(readstat_type_t type, size_t user_width) {
    size_t len = 0;
    if (type == READSTAT_TYPE_DOUBLE) {
        len = 8;
    } else if (type == READSTAT_TYPE_FLOAT) {
        len = 4;
    } else if (type == READSTAT_TYPE_INT32) {
        len = 4;
    } else if (type == READSTAT_TYPE_INT16) {
        len = 2;
    } else if (type == READSTAT_TYPE_INT8) {
        len = 1;
    }
    return len;
}

static size_t dta_111_variable_width(readstat_type_t type, size_t user_width) {
    if (type == READSTAT_TYPE_STRING) {
        if (user_width > DTA_111_MAX_WIDTH || user_width == 0)
            user_width = DTA_111_MAX_WIDTH;
        return user_width;
    }
    return dta_numeric_variable_width(type, user_width);
}

static size_t dta_117_variable_width(readstat_type_t type, size_t user_width) {
    if (type == READSTAT_TYPE_STRING) {
        if (user_width > DTA_117_MAX_WIDTH || user_width == 0)
            user_width = DTA_117_MAX_WIDTH;
        return user_width;
    }

    if (type == READSTAT_TYPE_STRING_REF)
        return 8;

    return dta_numeric_variable_width(type, user_width);
}

static size_t dta_old_variable_width(readstat_type_t type, size_t user_width) {
    if (type == READSTAT_TYPE_STRING) {
        if (user_width > DTA_OLD_MAX_WIDTH || user_width == 0)
            user_width = DTA_OLD_MAX_WIDTH;
        return user_width;
    }
    return dta_numeric_variable_width(type, user_width);
}

static readstat_error_t dta_emit_xmlish_header(readstat_writer_t *writer, dta_ctx_t *ctx) {
    readstat_error_t error = READSTAT_OK;

    if ((error = dta_write_tag(writer, ctx, "<stata_dta>")) != READSTAT_OK)
        goto cleanup;

    if ((error = dta_write_tag(writer, ctx, "<header>")) != READSTAT_OK)
        goto cleanup;

    char release[128];
    snprintf(release, sizeof(release), "<release>%ld</release>", writer->version);
    if ((error = readstat_write_string(writer, release)) != READSTAT_OK)
        goto cleanup;

    error = dta_write_chunk(writer, ctx, "<byteorder>",
            machine_is_little_endian() ? "LSF" : "MSF", sizeof("MSF")-1,
            "</byteorder>");
    if (error != READSTAT_OK)
        goto cleanup;

    if (writer->version >= 119) {
        uint32_t nvar = writer->variables_count;
        error = dta_write_chunk(writer, ctx, "<K>", &nvar, sizeof(uint32_t), "</K>");
        if (error != READSTAT_OK)
            goto cleanup;
    } else {
        /* K is a 2-byte field; Stata requires format 119 above 32,767 variables */
        if (writer->variables_count > 32767) {
            error = READSTAT_ERROR_TOO_MANY_COLUMNS;
            goto cleanup;
        }
        uint16_t nvar = writer->variables_count;
        error = dta_write_chunk(writer, ctx, "<K>", &nvar, sizeof(uint16_t), "</K>");
        if (error != READSTAT_OK)
            goto cleanup;
    }

    if (writer->version >= 118) {
        uint64_t nobs = writer->row_count;
        error = dta_write_chunk(writer, ctx, "<N>", &nobs, sizeof(uint64_t), "</N>");
        if (error != READSTAT_OK)
            goto cleanup;
    } else {
        uint32_t nobs = writer->row_count;
        error = dta_write_chunk(writer, ctx, "<N>", &nobs, sizeof(uint32_t), "</N>");
        if (error != READSTAT_OK)
            goto cleanup;
    }

    error = dta_emit_header_data_label(writer, ctx);
    if (error != READSTAT_OK)
        goto cleanup;

    error = dta_emit_header_time_stamp(writer, ctx);
    if (error != READSTAT_OK)
        goto cleanup;

    if ((error = dta_write_tag(writer, ctx, "</header>")) != READSTAT_OK)
        goto cleanup;

cleanup:
    return error;
}

static readstat_error_t dta_emit_header(readstat_writer_t *writer, dta_ctx_t *ctx) {
    if (ctx->file_is_xmlish) {
        return dta_emit_xmlish_header(writer, ctx);
    }
    readstat_error_t error = READSTAT_OK;
    dta_header_t header = {0};

    header.ds_format = writer->version;
    header.byteorder = machine_is_little_endian() ? DTA_LOHI : DTA_HILO;
    header.filetype  = 0x01;
    header.unused    = 0x00;
    header.nvar      = writer->variables_count;
    header.nobs      = writer->row_count;

    if (writer->variables_count > 32767) {
        error = READSTAT_ERROR_TOO_MANY_COLUMNS;
        goto cleanup;
    }

    if ((error = readstat_write_bytes(writer, &header, sizeof(dta_header_t))) != READSTAT_OK)
        goto cleanup;

    if ((error = dta_emit_header_data_label(writer, ctx)) != READSTAT_OK)
        goto cleanup;

    if ((error = dta_emit_header_time_stamp(writer, ctx)) != READSTAT_OK)
        goto cleanup;

cleanup:
    return error;
}

static size_t dta_measure_tag(dta_ctx_t *ctx, const char *tag) {
    if (!ctx->file_is_xmlish)
        return 0;

    return strlen(tag);
}

static size_t dta_measure_map(dta_ctx_t *ctx) {
    return (dta_measure_tag(ctx, "<map>") 
            + 14 * sizeof(uint64_t) 
            + dta_measure_tag(ctx, "</map>"));
}

static size_t dta_measure_typlist(dta_ctx_t *ctx) {
    return (dta_measure_tag(ctx, "<variable_types>") 
            + ctx->typlist_entry_len * ctx->nvar
            + dta_measure_tag(ctx, "</variable_types>"));
}

static size_t dta_measure_varlist(dta_ctx_t *ctx) {
    return (dta_measure_tag(ctx, "<varnames>") 
            + ctx->varlist_len
            + dta_measure_tag(ctx, "</varnames>"));
}

static size_t dta_measure_srtlist(dta_ctx_t *ctx) {
    return (dta_measure_tag(ctx, "<sortlist>")
            + ctx->srtlist_len
            + dta_measure_tag(ctx, "</sortlist>"));
}

static size_t dta_measure_fmtlist(dta_ctx_t *ctx) {
    return (dta_measure_tag(ctx, "<formats>")
            + ctx->fmtlist_len
            + dta_measure_tag(ctx, "</formats>"));
}

static size_t dta_measure_lbllist(dta_ctx_t *ctx) {
    return (dta_measure_tag(ctx, "<value_label_names>")
            + ctx->lbllist_len
            + dta_measure_tag(ctx, "</value_label_names>"));
}

static size_t dta_measure_variable_labels(dta_ctx_t *ctx) {
    return (dta_measure_tag(ctx, "<variable_labels>")
            + ctx->variable_labels_len
            + dta_measure_tag(ctx, "</variable_labels>"));
}

static size_t dta_measure_characteristic(dta_ctx_t *ctx, size_t contents_len) {
    return (dta_measure_tag(ctx, "<ch>")
            + ctx->expansion_len_len
            + dta_characteristic_payload_len(ctx, contents_len)
            + dta_measure_tag(ctx, "</ch>"));
}

static size_t dta_measure_characteristics(readstat_writer_t *writer, dta_ctx_t *ctx) {
    size_t characteristics_len = 0;
    int i;
    if (writer->notes_count > 0) {
        char count[32];
        dta_note_count_string(writer, count, sizeof(count));
        characteristics_len += dta_measure_characteristic(ctx, strlen(count));
    }
    for (i=0; i<writer->notes_count; i++) {
        characteristics_len += dta_measure_characteristic(ctx, strlen(writer->notes[i]));
    }
    return (dta_measure_tag(ctx, "<characteristics>")
            + characteristics_len
            + dta_measure_tag(ctx, "</characteristics>"));
}

static size_t dta_measure_data(readstat_writer_t *writer, dta_ctx_t *ctx) {
    int i;
    for (i=0; i<ctx->nvar; i++) {
        size_t      max_len = 0;
        readstat_variable_t *r_variable = readstat_get_variable(writer, i);
        uint16_t typecode = 0;
        dta_typecode_for_variable(r_variable, ctx->typlist_version, &typecode);
        if (dta_type_info(typecode, ctx, &max_len, NULL) == READSTAT_OK)
            ctx->record_len += max_len;
    }
    return (dta_measure_tag(ctx, "<data>")
            + ctx->record_len * ctx->nobs
            + dta_measure_tag(ctx, "</data>"));
}

static size_t dta_measure_strls(readstat_writer_t *writer, dta_ctx_t *ctx) {
    int i;
    size_t strls_len = 0;

    /* Measured before any row is written, so every registered ref counts;
     * dta_emit_strls refuses refs that were never inserted */
    for (i=0; i<writer->string_refs_count; i++) {
        readstat_string_ref_t *ref = writer->string_refs[i];
        if (ctx->strl_o_len > 4) {
            strls_len += sizeof("GSO") - 1 + SIZEOF_DTA_118_STRL_HEADER_T + ref->len;
        } else {
            strls_len += sizeof("GSO") - 1 + SIZEOF_DTA_117_STRL_HEADER_T + ref->len;
        }
    }

    return (dta_measure_tag(ctx, "<strls>")
            + strls_len
            + dta_measure_tag(ctx, "</strls>"));
}

static size_t dta_measure_value_labels(readstat_writer_t *writer, dta_ctx_t *ctx) {
    size_t len = dta_measure_tag(ctx, "<value_labels>");

    int i, j;
    for (i=0; i<writer->label_sets_count; i++) {
        readstat_label_set_t *r_label_set = readstat_get_label_set(writer, i);
        int32_t n = r_label_set->value_labels_count;
        int32_t txtlen = 0;
        for (j=0; j<n; j++) {
            readstat_value_label_t *value_label = readstat_get_value_label(r_label_set, j);
            txtlen += value_label->label_len + 1;
        }
        len += dta_measure_tag(ctx, "<lbl>");
        len += sizeof(int32_t);
        len += ctx->value_label_table_labname_len;
        len += ctx->value_label_table_padding_len;
        len += 8 + 8*n + txtlen;
        len += dta_measure_tag(ctx, "</lbl>");
    }
    len += dta_measure_tag(ctx, "</value_labels>");
    return len;
}

static readstat_error_t dta_emit_map(readstat_writer_t *writer, dta_ctx_t *ctx) {
    if (!ctx->file_is_xmlish)
        return READSTAT_OK;

    uint64_t map[14];

    map[0] = 0;                                         /* <stata_dta> */
    map[1] = writer->bytes_written;                     /* <map> */
    map[2] = map[1] + dta_measure_map(ctx);             /* <variable_types> */
    map[3] = map[2] + dta_measure_typlist(ctx);         /* <varnames> */
    map[4] = map[3] + dta_measure_varlist(ctx);         /* <sortlist> */
    map[5] = map[4] + dta_measure_srtlist(ctx);         /* <formats> */
    map[6] = map[5] + dta_measure_fmtlist(ctx);         /* <value_label_names> */
    map[7] = map[6] + dta_measure_lbllist(ctx);         /* <variable_labels> */
    map[8] = map[7] + dta_measure_variable_labels(ctx); /* <characteristics> */
    map[9] = map[8] + dta_measure_characteristics(writer, ctx); /* <data> */
    map[10]= map[9] + dta_measure_data(writer, ctx);    /* <strls> */
    map[11]= map[10]+ dta_measure_strls(writer, ctx);   /* <value_labels> */
    map[12]= map[11]+ dta_measure_value_labels(writer, ctx);    /* </stata_dta> */
    map[13]= map[12]+ dta_measure_tag(ctx, "</stata_dta>");

    return dta_write_chunk(writer, ctx, "<map>", map, sizeof(map), "</map>");
}

/* A data set or variable label must fit in its field with a terminator; 118
 * and later also cap it at 80 UTF-8 characters */
static readstat_error_t dta_validate_label(const char *label, size_t field_len, int unicode) {
    if (strlen(label) >= field_len)
        return READSTAT_ERROR_LABEL_IS_TOO_LONG;

    if (unicode && dta_utf8_char_count(label) > DTA_MAX_LABEL_CHARS)
        return READSTAT_ERROR_LABEL_IS_TOO_LONG;

    return READSTAT_OK;
}

static readstat_error_t dta_validate_label_set(readstat_writer_t *writer, dta_ctx_t *ctx,
        readstat_label_set_t *r_label_set) {
    readstat_error_t error = READSTAT_OK;
    int unicode = (writer->version >= 118);
    size_t max_name_len = unicode ? DTA_118_MAX_NAME_LEN :
        (writer->version >= 110 ? DTA_110_MAX_NAME_LEN : DTA_OLD_MAX_NAME_LEN);
    int j;

    /* Value-label names follow the variable-name rules (but are not subject
     * to the reserved-word list) */
    error = dta_validate_name_length_and_chars(r_label_set->name, unicode, max_name_len);
    if (error != READSTAT_OK)
        return error;

    /* Stata value labels map integers (and extended missing values) to text */
    if (readstat_type_class(r_label_set->type) == READSTAT_TYPE_CLASS_STRING)
        return READSTAT_ERROR_VALUE_TYPE_MISMATCH;

    for (j=0; j<r_label_set->value_labels_count; j++) {
        readstat_value_label_t *value_label = readstat_get_value_label(r_label_set, j);
        if (value_label->tag) {
            if (!ctx->supports_tagged_missing)
                return READSTAT_ERROR_TAGGED_VALUES_NOT_SUPPORTED;
            if (value_label->tag < 'a' || value_label->tag > 'z')
                return READSTAT_ERROR_TAGGED_VALUE_IS_OUT_OF_RANGE;
        } else if (r_label_set->type == READSTAT_TYPE_DOUBLE || r_label_set->type == READSTAT_TYPE_FLOAT) {
            double key = value_label->double_key;
            if (isnan(key) || key != floor(key))
                return READSTAT_ERROR_VALUE_TYPE_MISMATCH;
            if (key < DTA_MIN_INT32 || key > INT32_MAX)
                return READSTAT_ERROR_NUMERIC_VALUE_IS_OUT_OF_RANGE;
        }
    }

    /* Stata looks values up by binary search, so sort the set now and refuse
     * keys that appear more than once */
    readstat_sort_label_set(r_label_set, &dta_compare_value_labels);

    for (j=1; j<r_label_set->value_labels_count; j++) {
        if (dta_compare_value_labels(readstat_get_value_label(r_label_set, j-1),
                    readstat_get_value_label(r_label_set, j)) == 0)
            return READSTAT_ERROR_DUPLICATE_VALUE_LABEL;
    }

    return READSTAT_OK;
}

static readstat_error_t dta_validate_note(dta_ctx_t *ctx, const char *note) {
    size_t len = strlen(note);
    if (len > DTA_MAX_NOTE_LEN)
        return READSTAT_ERROR_NOTE_IS_TOO_LONG;

    /* Formats 105 through 109 store the expansion-field length in two bytes */
    if (ctx->expansion_len_len == 2 && dta_characteristic_payload_len(ctx, len) > INT16_MAX)
        return READSTAT_ERROR_NOTE_IS_TOO_LONG;

    return READSTAT_OK;
}

/* Everything that goes into a fixed-width field or a size-prefixed table is
 * checked here, before any bytes are written, so that no field is ever
 * truncated or left without its terminator */
static readstat_error_t dta_validate_metadata(readstat_writer_t *writer, dta_ctx_t *ctx) {
    readstat_error_t error = READSTAT_OK;
    int unicode = (writer->version >= 118);
    int i;

    error = dta_validate_label(writer->file_label, ctx->data_label_len, unicode);
    if (error != READSTAT_OK)
        return error;

    for (i=0; i<writer->variables_count; i++) {
        readstat_variable_t *r_variable = readstat_get_variable(writer, i);

        error = dta_validate_label(r_variable->label, ctx->variable_labels_entry_len, unicode);
        if (error != READSTAT_OK)
            return error;

        if (strlen(r_variable->format) >= ctx->fmtlist_entry_len)
            return READSTAT_ERROR_BAD_FORMAT_STRING;

        if (r_variable->label_set &&
                readstat_type_class(r_variable->type) == READSTAT_TYPE_CLASS_STRING)
            return READSTAT_ERROR_VALUE_TYPE_MISMATCH;
    }

    for (i=0; i<writer->label_sets_count; i++) {
        error = dta_validate_label_set(writer, ctx, readstat_get_label_set(writer, i));
        if (error != READSTAT_OK)
            return error;
    }

    for (i=0; i<writer->notes_count; i++) {
        error = dta_validate_note(ctx, writer->notes[i]);
        if (error != READSTAT_OK)
            return error;
    }

    return READSTAT_OK;
}

static readstat_error_t dta_begin_data(void *writer_ctx) {
    readstat_writer_t *writer = (readstat_writer_t *)writer_ctx;
    readstat_error_t error = READSTAT_OK;
    if (!writer->initialized)
        return READSTAT_ERROR_WRITER_NOT_INITIALIZED;

    dta_ctx_t *ctx = dta_ctx_alloc(NULL);
    if (ctx == NULL)
        return READSTAT_ERROR_MALLOC;

    error = dta_ctx_init(ctx, writer->variables_count, writer->row_count,
            machine_is_little_endian() ? DTA_LOHI : DTA_HILO, writer->version, NULL, NULL);
    if (error != READSTAT_OK)
        goto cleanup;

    error = dta_validate_metadata(writer, ctx);
    if (error != READSTAT_OK)
        goto cleanup;

    error = dta_emit_header(writer, ctx);
    if (error != READSTAT_OK)
        goto cleanup;

    error = dta_emit_map(writer, ctx);
    if (error != READSTAT_OK)
        goto cleanup;
    
    error = dta_emit_descriptors(writer, ctx);
    if (error != READSTAT_OK)
        goto cleanup;

    error = dta_emit_variable_labels(writer, ctx);
    if (error != READSTAT_OK)
        goto cleanup;

    error = dta_emit_characteristics(writer, ctx);
    if (error != READSTAT_OK)
        goto cleanup;

    error = dta_write_tag(writer, ctx, "<data>");
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

static readstat_error_t dta_write_raw_int8(void *row, int8_t value) {
    memcpy(row, &value, sizeof(char));
    return READSTAT_OK;
}

static readstat_error_t dta_write_raw_int16(void *row, int16_t value) {
    memcpy(row, &value, sizeof(int16_t));
    return READSTAT_OK;
}

static readstat_error_t dta_write_raw_int32(void *row, int32_t value) {
    memcpy(row, &value, sizeof(int32_t));
    return READSTAT_OK;
}

static readstat_error_t dta_write_raw_int64(void *row, int64_t value) {
    memcpy(row, &value, sizeof(int64_t));
    return READSTAT_OK;
}

static readstat_error_t dta_write_raw_float(void *row, float value) {
    memcpy(row, &value, sizeof(float));
    return READSTAT_OK;
}

static readstat_error_t dta_write_raw_double(void *row, double value) {
    memcpy(row, &value, sizeof(double));
    return READSTAT_OK;
}

static readstat_error_t dta_113_write_int8(void *row, const readstat_variable_t *var, int8_t value) {
    if (value > DTA_113_MAX_INT8 || value < DTA_MIN_INT8) {
        return READSTAT_ERROR_NUMERIC_VALUE_IS_OUT_OF_RANGE;
    }
    return dta_write_raw_int8(row, value);
}

static readstat_error_t dta_old_write_int8(void *row, const readstat_variable_t *var, int8_t value) {
    if (value > DTA_OLD_MAX_INT8 || value < DTA_MIN_INT8) {
        return READSTAT_ERROR_NUMERIC_VALUE_IS_OUT_OF_RANGE;
    }
    return dta_write_raw_int8(row, value);
}

static readstat_error_t dta_113_write_int16(void *row, const readstat_variable_t *var, int16_t value) {
    if (value > DTA_113_MAX_INT16 || value < DTA_MIN_INT16) {
        return READSTAT_ERROR_NUMERIC_VALUE_IS_OUT_OF_RANGE;
    }
    return dta_write_raw_int16(row, value);
}

static readstat_error_t dta_old_write_int16(void *row, const readstat_variable_t *var, int16_t value) {
    if (value > DTA_OLD_MAX_INT16 || value < DTA_MIN_INT16) {
        return READSTAT_ERROR_NUMERIC_VALUE_IS_OUT_OF_RANGE;
    }
    return dta_write_raw_int16(row, value);
}

static readstat_error_t dta_113_write_int32(void *row, const readstat_variable_t *var, int32_t value) {
    if (value > DTA_113_MAX_INT32 || value < DTA_MIN_INT32) {
        return READSTAT_ERROR_NUMERIC_VALUE_IS_OUT_OF_RANGE;
    }
    return dta_write_raw_int32(row, value);
}

static readstat_error_t dta_old_write_int32(void *row, const readstat_variable_t *var, int32_t value) {
    if (value > DTA_OLD_MAX_INT32 || value < DTA_MIN_INT32) {
        return READSTAT_ERROR_NUMERIC_VALUE_IS_OUT_OF_RANGE;
    }
    return dta_write_raw_int32(row, value);
}

/* Stata's floating-point ranges are symmetric: +-1.701e38 and +-8.988e307.
 * Infinities are outside them; NaN is written as system-missing. */
static readstat_error_t dta_write_float(void *row, const readstat_variable_t *var, float value) {
    int32_t max_flt_i32 = DTA_113_MAX_FLOAT;
    float max_flt;
    memcpy(&max_flt, &max_flt_i32, sizeof(float));
    if (isnan(value)) {
        return dta_113_write_missing_numeric(row, var);
    } else if (isinf(value) || value > max_flt || value < -max_flt) {
        return READSTAT_ERROR_NUMERIC_VALUE_IS_OUT_OF_RANGE;
    }
    return dta_write_raw_float(row, value);
}

static readstat_error_t dta_write_double(void *row, const readstat_variable_t *var, double value) {
    int64_t max_dbl_i64 = DTA_113_MAX_DOUBLE;
    double max_dbl;
    memcpy(&max_dbl, &max_dbl_i64, sizeof(double));
    if (isnan(value)) {
        return dta_113_write_missing_numeric(row, var);
    } else if (isinf(value) || value > max_dbl || value < -max_dbl) {
        return READSTAT_ERROR_NUMERIC_VALUE_IS_OUT_OF_RANGE;
    }
    return dta_write_raw_double(row, value);
}

static readstat_error_t dta_write_string(void *row, const readstat_variable_t *var, const char *value) {
    size_t max_len = var->storage_width;
    if (value == NULL || value[0] == '\0') {
        memset(row, '\0', max_len);
    } else {
        size_t value_len = strlen(value);
        if (value_len > max_len)
            return READSTAT_ERROR_STRING_VALUE_IS_TOO_LONG;

        strncpy((char *)row, value, max_len);
    }
    return READSTAT_OK;
}

/* A missing strL is the empty reference (v,o) = (0,0): eight zero bytes in
 * every layout */
static readstat_error_t dta_write_empty_string_ref(void *row) {
    memset(row, '\0', 8);
    return READSTAT_OK;
}

static readstat_error_t dta_118_write_string_ref(void *row, const readstat_variable_t *var, readstat_string_ref_t *ref) {
    if (ref == NULL)
        return dta_write_empty_string_ref(row);

    int16_t v = ref->first_v;
    int64_t o = ref->first_o;
    char *row_bytes = (char *)row;
    memcpy(&row_bytes[0], &v, sizeof(int16_t));
    if (!machine_is_little_endian()) {
        o <<= 16;
    }
    memcpy(&row_bytes[2], &o, 6);
    return READSTAT_OK;
}

static readstat_error_t dta_119_write_string_ref(void *row, const readstat_variable_t *var, readstat_string_ref_t *ref) {
    if (ref == NULL)
        return dta_write_empty_string_ref(row);

    /* 3-byte v followed by 5-byte o, in machine byte order */
    uint32_t v = ref->first_v;
    uint64_t o = ref->first_o;
    unsigned char *row_bytes = (unsigned char *)row;
    int i;
    if (machine_is_little_endian()) {
        for (i=0; i<3; i++)
            row_bytes[i] = (v >> (8*i)) & 0xFF;
        for (i=0; i<5; i++)
            row_bytes[3+i] = (o >> (8*i)) & 0xFF;
    } else {
        for (i=0; i<3; i++)
            row_bytes[i] = (v >> (8*(2-i))) & 0xFF;
        for (i=0; i<5; i++)
            row_bytes[3+i] = (o >> (8*(4-i))) & 0xFF;
    }
    return READSTAT_OK;
}

static readstat_error_t dta_117_write_string_ref(void *row, const readstat_variable_t *var, readstat_string_ref_t *ref) {
    if (ref == NULL)
        return dta_write_empty_string_ref(row);

    int32_t v = ref->first_v;
    int32_t o = ref->first_o;
    char *row_bytes = (char *)row;
    memcpy(&row_bytes[0], &v, sizeof(int32_t));
    memcpy(&row_bytes[4], &o, sizeof(int32_t));
    return READSTAT_OK;
}

static readstat_error_t dta_113_write_missing_numeric(void *row, const readstat_variable_t *var) {
    readstat_error_t retval = READSTAT_OK;
    if (var->type == READSTAT_TYPE_INT8) {
        retval = dta_write_raw_int8(row, DTA_113_MISSING_INT8);
    } else if (var->type == READSTAT_TYPE_INT16) {
        retval = dta_write_raw_int16(row, DTA_113_MISSING_INT16);
    } else if (var->type == READSTAT_TYPE_INT32) {
        retval = dta_write_raw_int32(row, DTA_113_MISSING_INT32);
    } else if (var->type == READSTAT_TYPE_FLOAT) {
        retval = dta_write_raw_int32(row, DTA_113_MISSING_FLOAT);
    } else if (var->type == READSTAT_TYPE_DOUBLE) {
        retval = dta_write_raw_int64(row, DTA_113_MISSING_DOUBLE);
    }
    return retval;
}

static readstat_error_t dta_old_write_missing_numeric(void *row, const readstat_variable_t *var) {
    readstat_error_t retval = READSTAT_OK;
    if (var->type == READSTAT_TYPE_INT8) {
        retval = dta_write_raw_int8(row, DTA_OLD_MISSING_INT8);
    } else if (var->type == READSTAT_TYPE_INT16) {
        retval = dta_write_raw_int16(row, DTA_OLD_MISSING_INT16);
    } else if (var->type == READSTAT_TYPE_INT32) {
        retval = dta_write_raw_int32(row, DTA_OLD_MISSING_INT32);
    } else if (var->type == READSTAT_TYPE_FLOAT) {
        retval = dta_write_raw_int32(row, DTA_OLD_MISSING_FLOAT);
    } else if (var->type == READSTAT_TYPE_DOUBLE) {
        retval = dta_write_raw_int64(row, DTA_OLD_MISSING_DOUBLE);
    }
    return retval;
}

static readstat_error_t dta_write_missing_string(void *row, const readstat_variable_t *var) {
    return dta_write_string(row, var, NULL);
}

static readstat_error_t dta_113_write_missing_tagged(void *row, const readstat_variable_t *var, char tag) {
    readstat_error_t retval = READSTAT_OK;
    if (tag < 'a' || tag > 'z')
        return READSTAT_ERROR_TAGGED_VALUE_IS_OUT_OF_RANGE;

    if (var->type == READSTAT_TYPE_INT8) {
        retval = dta_write_raw_int8(row, DTA_113_MISSING_INT8_A + (tag - 'a'));
    } else if (var->type == READSTAT_TYPE_INT16) {
        retval = dta_write_raw_int16(row, DTA_113_MISSING_INT16_A + (tag - 'a'));
    } else if (var->type == READSTAT_TYPE_INT32) {
        retval = dta_write_raw_int32(row, DTA_113_MISSING_INT32_A + (tag - 'a'));
    } else if (var->type == READSTAT_TYPE_FLOAT) {
        retval = dta_write_raw_int32(row, DTA_113_MISSING_FLOAT_A + ((tag - 'a') << 11));
    } else if (var->type == READSTAT_TYPE_DOUBLE) {
        retval = dta_write_raw_int64(row, DTA_113_MISSING_DOUBLE_A + ((int64_t)(tag - 'a') << 40));
    } else {
        retval = READSTAT_ERROR_TAGGED_VALUES_NOT_SUPPORTED;
    }
    return retval;
}

static readstat_error_t dta_end_data(void *writer_ctx) {
    readstat_writer_t *writer = (readstat_writer_t *)writer_ctx;
    dta_ctx_t *ctx = writer->module_ctx;
    readstat_error_t error = READSTAT_OK;

    /* module_ctx is NULL when dta_begin_data failed */
    if (!writer->initialized || ctx == NULL)
        return READSTAT_ERROR_WRITER_NOT_INITIALIZED;

    error = dta_write_tag(writer, ctx, "</data>");
    if (error != READSTAT_OK)
        goto cleanup;

    error = dta_emit_strls(writer, ctx);
    if (error != READSTAT_OK)
        goto cleanup;

    error = dta_emit_value_labels(writer, ctx);
    if (error != READSTAT_OK)
        goto cleanup;

    error = dta_write_tag(writer, ctx, "</stata_dta>");
    if (error != READSTAT_OK)
        goto cleanup;

cleanup:
    return error;
}

/* Rows may only follow a header: refuse them when dta_begin_data failed */
static readstat_error_t dta_write_row(void *writer_ctx, void *bytes, size_t len) {
    readstat_writer_t *writer = (readstat_writer_t *)writer_ctx;
    if (writer->module_ctx == NULL)
        return READSTAT_ERROR_WRITER_NOT_INITIALIZED;

    return readstat_write_bytes(writer, bytes, len);
}

static void dta_module_ctx_free(void *module_ctx) {
    dta_ctx_free(module_ctx);
}

readstat_error_t dta_metadata_ok(void *writer_ctx) {
    readstat_writer_t *writer = (readstat_writer_t *)writer_ctx;

    if (writer->compression != READSTAT_COMPRESS_NONE)
        return READSTAT_ERROR_UNSUPPORTED_COMPRESSION;

    if (writer->version > DTA_FILE_VERSION_MAX || writer->version < DTA_FILE_VERSION_MIN)
        return READSTAT_ERROR_UNSUPPORTED_FILE_FORMAT_VERSION;

    /* The variable count is a 2-byte field before format 119 */
    if (writer->version < 119 && writer->variables_count > 32767)
        return READSTAT_ERROR_TOO_MANY_COLUMNS;

    return READSTAT_OK;
}

readstat_error_t readstat_begin_writing_dta(readstat_writer_t *writer, void *user_ctx, long row_count) {

    if (writer->version == 0)
        writer->version = DTA_FILE_VERSION_DEFAULT;

    writer->callbacks.metadata_ok = &dta_metadata_ok;

    if (writer->version >= 117) {
        writer->callbacks.variable_width = &dta_117_variable_width;
    } else if (writer->version >= 111) {
        writer->callbacks.variable_width = &dta_111_variable_width;
    } else {
        writer->callbacks.variable_width = &dta_old_variable_width;
    }

    if (writer->version >= 118) {
        writer->callbacks.variable_ok = &dta_118_variable_ok;
    } else if (writer->version >= 117) {
        writer->callbacks.variable_ok = &dta_117_variable_ok;
    } else if (writer->version >= 111) {
        writer->callbacks.variable_ok = &dta_111_variable_ok;
    } else if (writer->version >= 110) {
        writer->callbacks.variable_ok = &dta_110_variable_ok;
    } else {
        writer->callbacks.variable_ok = &dta_old_variable_ok;
    }

    if (writer->version >= 119) {
        writer->callbacks.write_string_ref = &dta_119_write_string_ref;
    } else if (writer->version == 118) {
        writer->callbacks.write_string_ref = &dta_118_write_string_ref;
    } else if (writer->version == 117) {
        writer->callbacks.write_string_ref = &dta_117_write_string_ref;
    }

    if (writer->version >= 113) {
        writer->callbacks.write_int8 = &dta_113_write_int8;
        writer->callbacks.write_int16 = &dta_113_write_int16;
        writer->callbacks.write_int32 = &dta_113_write_int32;
        writer->callbacks.write_missing_number = &dta_113_write_missing_numeric;
        writer->callbacks.write_missing_tagged = &dta_113_write_missing_tagged;
    } else {
        writer->callbacks.write_int8 = &dta_old_write_int8;
        writer->callbacks.write_int16 = &dta_old_write_int16;
        writer->callbacks.write_int32 = &dta_old_write_int32;
        writer->callbacks.write_missing_number = &dta_old_write_missing_numeric;
    }

    writer->callbacks.write_float = &dta_write_float;
    writer->callbacks.write_double = &dta_write_double;
    writer->callbacks.write_string = &dta_write_string;
    writer->callbacks.write_missing_string = &dta_write_missing_string;

    writer->callbacks.begin_data = &dta_begin_data;
    writer->callbacks.write_row = &dta_write_row;
    writer->callbacks.end_data = &dta_end_data;
    writer->callbacks.module_ctx_free = &dta_module_ctx_free;

    return readstat_begin_writing_file(writer, user_ctx, row_count);
}
