
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <iconv.h>
#include <inttypes.h>

#include "../readstat.h"
#include "../CKHashTable.h"
#include "../readstat_writer.h"

#include "readstat_spss.h"
#include "readstat_por.h"

/* Significant base-30 digits written for a double. Twelve identify any
 * double uniquely (30^11 > 2^53), so values round-trip exactly through a
 * correctly rounding reader; SPSS itself writes eleven. */
#define POR_BASE30_PRECISION  12
/* Longest number field: sign, digits, radix point or exponent sign, up to
 * three exponent digits, slash, and a terminating NUL */
#define POR_NUMBER_FIELD_WIDTH  (POR_BASE30_PRECISION + 8)
/* Widest string variable the format allows */
#define POR_MAX_STRING_WIDTH  255
/* Longest UTF-8 encoding of a character in the portable character set */
#define POR_MAX_UTF8_CHAR_LEN 3
/* Exact expansion of a denormal needs about 870 trigesimals */
#define POR_MAX_TRIGS         1200

typedef struct por_write_ctx_s {
    unsigned char   *unicode2byte;
    size_t           unicode2byte_len;
} por_write_ctx_t;

static inline char por_encode_base30_digit(uint64_t digit) {
    if (digit < 10)
        return '0' + digit;
    return 'A' + (digit - 10);
}

static int por_write_base30_integer(char *string, size_t string_len, uint64_t integer) {
    int start = 0;
    int end = 0;
    int offset = 0;
    if (integer == 0) {
        string[offset++] = '0';
    }
    while (integer) {
        string[offset++] = por_encode_base30_digit(integer % 30);
        integer /= 30;
    }
    end = offset;
    offset--;
    while (offset > start) {
        char tmp = string[start];
        string[start] = string[offset];
        string[offset] = tmp;
        offset--; start++;
    }
    return end;
}

static readstat_error_t por_finish(readstat_writer_t *writer) {
    return readstat_write_line_padding(writer, 'Z', 80, "\r\n");
}

static readstat_error_t por_write_bytes(readstat_writer_t *writer, const void *bytes, size_t len) {
    return readstat_write_bytes_as_lines(writer, bytes, len, 80, "\r\n");
}

/* Converts a UTF-8 string to the portable character set. Every code point
 * becomes one byte, so the output is never longer than the input. On success
 * the caller owns *out_string. */
static readstat_error_t por_convert_string(readstat_writer_t *writer, por_write_ctx_t *ctx,
        const char *string, size_t input_len, char **out_string, size_t *out_len) {
    char error_buf[1024];
    readstat_error_t retval = READSTAT_OK;
    char *por_string = malloc(input_len ? input_len : 1);
    ssize_t output_len = 0;
    if (por_string == NULL) {
        retval = READSTAT_ERROR_MALLOC;
        goto cleanup;
    }
    output_len = por_utf8_decode(string, input_len, por_string, input_len,
            ctx->unicode2byte, ctx->unicode2byte_len);
    if (output_len == -1) {
        if (writer->error_handler) {
            snprintf(error_buf, sizeof(error_buf), "Error converting string (length=%" PRId64 "): %.*s",
                    (int64_t)input_len, (int)input_len, string);
            writer->error_handler(error_buf, writer->user_ctx);
        }
        retval = READSTAT_ERROR_CONVERT;
        goto cleanup;
    }
    *out_string = por_string;
    *out_len = output_len;
    por_string = NULL;
cleanup:
    if (por_string)
        free(por_string);
    return retval;
}

static readstat_error_t por_write_string_n(readstat_writer_t *writer, por_write_ctx_t *ctx,
        const char *string, size_t input_len) {
    readstat_error_t retval = READSTAT_OK;
    char *por_string = NULL;
    size_t output_len = 0;
    if ((retval = por_convert_string(writer, ctx, string, input_len, &por_string, &output_len)) != READSTAT_OK)
        return retval;
    retval = por_write_bytes(writer, por_string, output_len);
    free(por_string);
    return retval;
}

static readstat_error_t por_write_tag(readstat_writer_t *writer, por_write_ctx_t *ctx, char tag) {
    char string[2];
    string[0] = tag;
    string[1] = '\0';
    return por_write_string_n(writer, ctx, string, 1);
}

/* Rounds the exact expansion in trigs to at most precision significant
 * digits, half to even, and strips trailing zeros. Returns the new count. */
static int por_round_trigs(unsigned char *trigs, int n_trigs, int precision, int *trig_places) {
    if (n_trigs > precision) {
        int round_up = 0;
        int i;
        if (trigs[precision] > 15) {
            round_up = 1;
        } else if (trigs[precision] == 15) {
            for (i=precision+1; i<n_trigs; i++) {
                if (trigs[i]) {
                    round_up = 1;
                    break;
                }
            }
            if (!round_up) /* exactly half: round to even */
                round_up = trigs[precision-1] % 2;
        }
        n_trigs = precision;
        if (round_up) {
            int carry = 1;
            for (i=n_trigs-1; i>=0 && carry; i--) {
                if (trigs[i] == 29) {
                    trigs[i] = 0;
                } else {
                    trigs[i]++;
                    carry = 0;
                }
            }
            if (carry) {
                trigs[0] = 1;
                n_trigs = 1;
                (*trig_places)++;
            }
        }
    }
    while (n_trigs > 1 && trigs[n_trigs-1] == 0)
        n_trigs--;
    return n_trigs;
}

/* Formats value as a portable-file floating-point field ending in '/'
 * (or "*." for missing). The layout follows PSPP: conventional notation
 * when the radix point falls within a couple of places of the digits,
 * otherwise digits followed by a signed base-30 exponent. Never writes more
 * than buffer_len bytes (including the terminating NUL); returns the number
 * of characters written, or -1 if the field does not fit or cannot be
 * formatted. */
static ssize_t por_write_double_to_buffer(char *string, size_t buffer_len, double value, long precision) {
    char field[POR_BASE30_PRECISION + 24];
    size_t offset = 0;
    if (precision < 1 || precision > POR_BASE30_PRECISION)
        precision = POR_BASE30_PRECISION;

    if (isnan(value)) {
        field[offset++] = '*';
        field[offset++] = '.';
    } else if (isinf(value)) {
        if (value < 0.0) {
            field[offset++] = '-';
        }
        field[offset++] = '1';
        field[offset++] = '+';
        field[offset++] = 'T';
        field[offset++] = 'T';
        field[offset++] = '/';
    } else if (value == 0.0) {
        if (signbit(value)) {
            field[offset++] = '-';
        }
        field[offset++] = '0';
        field[offset++] = '/';
    } else {
        unsigned char trigs[POR_MAX_TRIGS];
        int trig_places = 0;
        int n_trigs = por_double_to_trigs(value, trigs, sizeof(trigs), &trig_places);
        int i;
        if (n_trigs < 1)
            return -1;

        n_trigs = por_round_trigs(trigs, n_trigs, precision, &trig_places);

        if (value < 0.0) {
            field[offset++] = '-';
        }
        if (trig_places >= -1 && trig_places < n_trigs + 3) {
            if (trig_places <= 0) {
                field[offset++] = '0';
                field[offset++] = '.';
                for (i=trig_places; i<0; i++)
                    field[offset++] = '0';
                for (i=0; i<n_trigs; i++)
                    field[offset++] = por_encode_base30_digit(trigs[i]);
            } else {
                for (i=0; i<trig_places; i++)
                    field[offset++] = i < n_trigs ? por_encode_base30_digit(trigs[i]) : '0';
                if (n_trigs > trig_places) {
                    field[offset++] = '.';
                    for (i=trig_places; i<n_trigs; i++)
                        field[offset++] = por_encode_base30_digit(trigs[i]);
                }
            }
        } else {
            long exponent = trig_places - n_trigs;
            for (i=0; i<n_trigs; i++)
                field[offset++] = por_encode_base30_digit(trigs[i]);
            field[offset++] = exponent < 0 ? '-' : '+';
            offset += por_write_base30_integer(&field[offset], sizeof(field) - offset,
                    exponent < 0 ? -exponent : exponent);
        }
        field[offset++] = '/';
    }
    if (offset + 1 > buffer_len)
        return -1;

    memcpy(string, field, offset);
    string[offset] = '\0';
    return offset;
}

static readstat_error_t por_write_double(readstat_writer_t *writer, por_write_ctx_t *ctx, double value) {
    char error_buf[1024];
    char string[256];
    ssize_t bytes_written = por_write_double_to_buffer(string, sizeof(string), value, POR_BASE30_PRECISION);
    if (bytes_written == -1) {
        if (writer->error_handler) {
            snprintf(error_buf, sizeof(error_buf), "Unable to encode number: %lf", value);
            writer->error_handler(error_buf, writer->user_ctx);
        }
        return READSTAT_ERROR_WRITE;
    }

    return por_write_string_n(writer, ctx, string, bytes_written);
}

static readstat_error_t por_write_string_field_n(readstat_writer_t *writer, por_write_ctx_t *ctx,
        const char *string, size_t len) {
    readstat_error_t error = READSTAT_OK;
    char *por_string = NULL;
    size_t output_len = 0;

    /* The length prefix counts characters in the file's character set, not
     * UTF-8 bytes, so convert first */
    if ((error = por_convert_string(writer, ctx, string, len, &por_string, &output_len)) != READSTAT_OK)
        return error;

    if ((error = por_write_double(writer, ctx, output_len)) != READSTAT_OK)
        goto cleanup;

    error = por_write_bytes(writer, por_string, output_len);

cleanup:
    free(por_string);
    return error;
}

static readstat_error_t por_write_string_field(readstat_writer_t *writer, por_write_ctx_t *ctx, const char *string) {
    return por_write_string_field_n(writer, ctx, string, strlen(string));
}

static por_write_ctx_t *por_write_ctx_init(void) {
    por_write_ctx_t *ctx = calloc(1, sizeof(por_write_ctx_t));
    uint16_t max_unicode = 0;
    int i;
    for (i=0; i<sizeof(por_unicode_lookup)/sizeof(por_unicode_lookup[0]); i++) {
        if (por_unicode_lookup[i] > max_unicode)
            max_unicode = por_unicode_lookup[i];
    }
    ctx->unicode2byte = calloc(max_unicode+1, 1);
    ctx->unicode2byte_len = max_unicode+1;

    for (i=0; i<sizeof(por_unicode_lookup)/sizeof(por_unicode_lookup[0]); i++) {
        if (por_unicode_lookup[i]) {
            ctx->unicode2byte[por_unicode_lookup[i]] = por_ascii_lookup[i];
        }
        if (por_ascii_lookup[i]) {
            ctx->unicode2byte[por_ascii_lookup[i]] = por_ascii_lookup[i];
        }
    }
    return ctx;
}

static void por_write_ctx_free(por_write_ctx_t *ctx) {
    if (ctx->unicode2byte)
        free(ctx->unicode2byte);
    free(ctx);
}

static readstat_error_t por_emit_header(readstat_writer_t *writer, por_write_ctx_t *ctx) {
    readstat_error_t retval = READSTAT_OK;

    size_t file_label_len = strlen(writer->file_label);
    char vanity[5][40];
    memset(vanity, '0', sizeof(vanity));
    memset(vanity[1], ' ', sizeof(vanity[1]));

    memcpy(vanity[1], "ASCII SPSS PORT FILE", 20);
    memcpy(vanity[1] + 20, writer->file_label, file_label_len > 20 ? 20 : file_label_len);

    por_write_bytes(writer, vanity, sizeof(vanity));

    char lookup[256];
    int i;
    memset(lookup, '0', sizeof(lookup));
    for (i=0; i<sizeof(lookup); i++) {
        if (por_ascii_lookup[i]) {
            lookup[i] = por_ascii_lookup[i];
        }
    }
    if ((retval = por_write_bytes(writer, lookup, sizeof(lookup))) != READSTAT_OK)
        goto cleanup;

    if ((retval = por_write_string_n(writer, ctx, "SPSSPORT", sizeof("SPSSPORT")-1)) != READSTAT_OK)
        goto cleanup;

cleanup:
    return retval;
}

static readstat_error_t por_emit_version_and_timestamp(readstat_writer_t *writer,
        por_write_ctx_t *ctx) {
    readstat_error_t retval = READSTAT_OK;
    struct tm *timestamp = localtime(&writer->timestamp);

    if (!timestamp) {
        retval = READSTAT_ERROR_BAD_TIMESTAMP_VALUE;
        goto cleanup;
    }

    if ((retval = por_write_tag(writer, ctx, 'A')) != READSTAT_OK)
        goto cleanup;

    char date[9];
    snprintf(date, sizeof(date), "%04d%02d%02d",
            (unsigned int)(timestamp->tm_year + 1900) % 10000,
            (unsigned int)(timestamp->tm_mon + 1) % 100,
            (unsigned int)(timestamp->tm_mday) % 100);
    if ((retval = por_write_string_field(writer, ctx, date)) != READSTAT_OK)
        goto cleanup;

    char time[7];
    snprintf(time, sizeof(time), "%02d%02d%02d",
            (unsigned int)timestamp->tm_hour % 100,
            (unsigned int)timestamp->tm_min % 100,
            (unsigned int)timestamp->tm_sec % 100);
    if ((retval = por_write_string_field(writer, ctx, time)) != READSTAT_OK)
        goto cleanup;

cleanup:
    return retval;
}

static readstat_error_t por_emit_identification_records(readstat_writer_t *writer,
        por_write_ctx_t *ctx) {
    readstat_error_t retval = READSTAT_OK;

    if ((retval = por_write_tag(writer, ctx, '1')) != READSTAT_OK)
        goto cleanup;

    if ((retval = por_write_string_field(writer, ctx, READSTAT_PRODUCT_NAME)) != READSTAT_OK)
        goto cleanup;

    if ((retval = por_write_tag(writer, ctx, '3')) != READSTAT_OK)
        goto cleanup;

    if ((retval = por_write_string_field(writer, ctx, READSTAT_PRODUCT_URL)) != READSTAT_OK)
        goto cleanup;

cleanup:
    return retval;
}

static readstat_error_t por_emit_variable_count_record(readstat_writer_t *writer,
        por_write_ctx_t *ctx) {
    readstat_error_t retval = READSTAT_OK;

    if ((retval = por_write_tag(writer, ctx, '4')) != READSTAT_OK)
        goto cleanup;

    if ((retval = por_write_double(writer, ctx, writer->variables_count)) != READSTAT_OK)
        goto cleanup;

cleanup:
    return retval;
}

static readstat_error_t por_emit_precision_record(readstat_writer_t *writer,
        por_write_ctx_t *ctx) {
    readstat_error_t retval = READSTAT_OK;

    if ((retval = por_write_tag(writer, ctx, '5')) != READSTAT_OK)
        goto cleanup;

    if ((retval = por_write_double(writer, ctx, POR_BASE30_PRECISION)) != READSTAT_OK)
        goto cleanup;

cleanup:
    return retval;
}

static readstat_error_t por_emit_case_weight_variable_record(readstat_writer_t *writer,
        por_write_ctx_t *ctx) {
    if (!writer->fweight_variable)
        return READSTAT_OK;

    readstat_error_t retval = READSTAT_OK;

    if ((retval = por_write_tag(writer, ctx, '6')) != READSTAT_OK)
        goto cleanup;

    if ((retval = por_write_string_field(writer, ctx, 
                    readstat_variable_get_name(writer->fweight_variable))) != READSTAT_OK)
        goto cleanup;

cleanup:
    return retval;
}

static readstat_error_t por_emit_format(readstat_writer_t *writer, por_write_ctx_t *ctx,
        spss_format_t *format) {
    readstat_error_t error = READSTAT_OK;

    if ((error = por_write_double(writer, ctx, format->type)) != READSTAT_OK)
        goto cleanup;

    if ((error = por_write_double(writer, ctx, format->width)) != READSTAT_OK)
        goto cleanup;

    if ((error = por_write_double(writer, ctx, format->decimal_places)) != READSTAT_OK)
        goto cleanup;

cleanup:
    return error;
}

static readstat_error_t validate_variable_name(const char *name) {
    size_t len = strlen(name);
    if (len < 1 || len > 8)
        return READSTAT_ERROR_NAME_IS_TOO_LONG;
    int i;
    for (i=0; name[i]; i++) {
        if (name[i] >= 'A' && name[i] <= 'Z')
            continue;
        if (name[i] >= '0' && name[i] <= '9')
            continue;
        if (name[i] == '@' || name[i] == '#' || name[i] == '$')
            continue;
        if (name[i] == '_' || name[i] == '.')
            continue;

        return READSTAT_ERROR_NAME_CONTAINS_ILLEGAL_CHARACTER;
    }

    if (!(name[0] >= 'A' && name[0] <= 'Z') && name[0] != '@')
        return READSTAT_ERROR_NAME_BEGINS_WITH_ILLEGAL_CHARACTER;

    return READSTAT_OK;
}

static readstat_error_t por_emit_variable_label_record(readstat_writer_t *writer,
        por_write_ctx_t *ctx, readstat_variable_t *r_variable) {
    const char *label = readstat_variable_get_label(r_variable);
    readstat_error_t retval = READSTAT_OK;
    if (!label)
        return READSTAT_OK;

    if ((retval = por_write_tag(writer, ctx, 'C')) != READSTAT_OK)
        goto cleanup;

    if ((retval = por_write_string_field(writer, ctx, label)) != READSTAT_OK)
        goto cleanup;

cleanup:
    return retval;
}

static readstat_error_t por_emit_missing_string_values_records(readstat_writer_t *writer,
        por_write_ctx_t *ctx, readstat_variable_t *r_variable) {
    readstat_error_t retval = READSTAT_OK;
    int n_missing_values = 0;
    int n_missing_ranges = readstat_variable_get_missing_ranges_count(r_variable);
    /* ranges */
    int j;

    for (j=0; j<n_missing_ranges; j++) {
        readstat_value_t lo_value = readstat_variable_get_missing_range_lo(r_variable, j);
        readstat_value_t hi_value = readstat_variable_get_missing_range_hi(r_variable, j);
        const char *lo = readstat_string_value(lo_value);
        const char *hi = readstat_string_value(hi_value);
        if (lo && hi && strcmp(lo, hi) != 0) {
            if ((retval = por_write_tag(writer, ctx, 'B')) != READSTAT_OK)
                goto cleanup;

            if ((retval = por_write_string_field(writer, ctx, lo)) != READSTAT_OK)
                goto cleanup;

            if ((retval = por_write_string_field(writer, ctx, hi)) != READSTAT_OK)
                goto cleanup;

            n_missing_values += 2;
        }
    }
    /* values */
    for (j=0; j<n_missing_ranges; j++) {
        readstat_value_t lo_value = readstat_variable_get_missing_range_lo(r_variable, j);
        readstat_value_t hi_value = readstat_variable_get_missing_range_hi(r_variable, j);
        const char *lo = readstat_string_value(lo_value);
        const char *hi = readstat_string_value(hi_value);
        if (lo && hi && strcmp(lo, hi) == 0) {
            if ((retval = por_write_tag(writer, ctx, '8')) != READSTAT_OK)
                goto cleanup;

            if ((retval = por_write_string_field(writer, ctx, lo)) != READSTAT_OK)
                goto cleanup;

            n_missing_values++;
        }
    }
    if (n_missing_values > 3)
        retval = READSTAT_ERROR_TOO_MANY_MISSING_VALUE_DEFINITIONS;

cleanup:
    return retval;
}

static readstat_error_t por_emit_missing_double_values_records(readstat_writer_t *writer,
        por_write_ctx_t *ctx, readstat_variable_t *r_variable) {
    readstat_error_t retval = READSTAT_OK;
    int n_missing_values = 0;
    int n_missing_ranges = readstat_variable_get_missing_ranges_count(r_variable);
    /* ranges */
    int j;

    for (j=0; j<n_missing_ranges; j++) {
        readstat_value_t lo_value = readstat_variable_get_missing_range_lo(r_variable, j);
        readstat_value_t hi_value = readstat_variable_get_missing_range_hi(r_variable, j);
        double lo = readstat_double_value(lo_value);
        double hi = readstat_double_value(hi_value);
        if (isinf(lo)) {
            if ((retval = por_write_tag(writer, ctx, '9')) != READSTAT_OK)
                goto cleanup;

            if ((retval = por_write_double(writer, ctx, hi)) != READSTAT_OK)
                goto cleanup;

            n_missing_values += 2;
        } else if (isinf(hi)) {
            if ((retval = por_write_tag(writer, ctx, 'A')) != READSTAT_OK)
                goto cleanup;

            if ((retval = por_write_double(writer, ctx, lo)) != READSTAT_OK)
                goto cleanup;

            n_missing_values += 2;
        } else if (lo != hi) {
            if ((retval = por_write_tag(writer, ctx, 'B')) != READSTAT_OK)
                goto cleanup;

            if ((retval = por_write_double(writer, ctx, lo)) != READSTAT_OK)
                goto cleanup;

            if ((retval = por_write_double(writer, ctx, hi)) != READSTAT_OK)
                goto cleanup;

            n_missing_values += 2;
        }
    }
    /* values */
    for (j=0; j<n_missing_ranges; j++) {
        readstat_value_t lo_value = readstat_variable_get_missing_range_lo(r_variable, j);
        readstat_value_t hi_value = readstat_variable_get_missing_range_hi(r_variable, j);
        double lo = readstat_double_value(lo_value);
        double hi = readstat_double_value(hi_value);
        if (lo == hi && !isinf(lo) && !isinf(hi)) {
            if ((retval = por_write_tag(writer, ctx, '8')) != READSTAT_OK)
                goto cleanup;

            if ((retval = por_write_double(writer, ctx, lo)) != READSTAT_OK)
                goto cleanup;

            n_missing_values++;
        }
    }
    if (n_missing_values > 3)
        retval = READSTAT_ERROR_TOO_MANY_MISSING_VALUE_DEFINITIONS;

cleanup:
    return retval;
}

static readstat_error_t por_emit_missing_values_records(readstat_writer_t *writer,
        por_write_ctx_t *ctx, readstat_variable_t *r_variable) {
    if (r_variable->type == READSTAT_TYPE_DOUBLE) {
        return por_emit_missing_double_values_records(writer, ctx, r_variable);
    }
    return por_emit_missing_string_values_records(writer, ctx, r_variable);
}

/* Declared width of a string variable in characters. A zero width (an
 * all-empty column) is written as width one, like the other writers' default
 * widths; empty values are written as a single space. */
static size_t por_string_width(size_t user_width) {
    return user_width == 0 ? 1 : user_width;
}

static readstat_error_t por_emit_variable_records(readstat_writer_t *writer,
        por_write_ctx_t *ctx) {
    readstat_error_t retval = READSTAT_OK;
    int i;
    for (i=0; i<writer->variables_count; i++) {
        readstat_variable_t *r_variable = readstat_get_variable(writer, i);
        const char *variable_name = readstat_variable_get_name(r_variable);
        spss_format_t print_format;

        if ((retval = por_write_tag(writer, ctx, '7')) != READSTAT_OK)
            break;

        retval = por_write_double(writer, ctx,
                (r_variable->type == READSTAT_TYPE_STRING) ?
                por_string_width(r_variable->user_width) : 0);
        if (retval != READSTAT_OK)
            break;

        if ((retval = por_write_string_field(writer, ctx, variable_name)) != READSTAT_OK)
            break;

        if ((retval = spss_format_for_variable(r_variable, &print_format)) != READSTAT_OK)
            break;

        if ((retval = por_emit_format(writer, ctx, &print_format)) != READSTAT_OK)
            break;

        if ((retval = por_emit_format(writer, ctx, &print_format)) != READSTAT_OK)
            break;

        if ((retval = por_emit_missing_values_records(writer, ctx, r_variable)) != READSTAT_OK)
            break;

        if ((retval = por_emit_variable_label_record(writer, ctx, r_variable)) != READSTAT_OK)
            break;
    }
    return retval;
}

static readstat_error_t por_emit_value_label_records(readstat_writer_t *writer,
        por_write_ctx_t *ctx) {
    readstat_error_t retval = READSTAT_OK;
    int i, j;

    for (i=0; i<writer->label_sets_count; i++) {
        readstat_label_set_t *r_label_set = readstat_get_label_set(writer, i);
        readstat_type_t user_type = r_label_set->type;
        if (r_label_set->value_labels_count == 0 || r_label_set->variables_count == 0)
            continue;

        if ((retval = por_write_tag(writer, ctx, 'D')) != READSTAT_OK)
            goto cleanup;

        if ((retval = por_write_double(writer, ctx, r_label_set->variables_count)) != READSTAT_OK)
            goto cleanup;

        for (j=0; j<r_label_set->variables_count; j++) {
            readstat_variable_t *r_variable = readstat_get_label_set_variable(r_label_set, j);

            if ((retval = por_write_string_field(writer, ctx, 
                            readstat_variable_get_name(r_variable))) != READSTAT_OK)
                goto cleanup;
        }

        if ((retval = por_write_double(writer, ctx, r_label_set->value_labels_count)) != READSTAT_OK)
            goto cleanup;

        for (j=0; j<r_label_set->value_labels_count; j++) {
            readstat_value_label_t *r_value_label = readstat_get_value_label(r_label_set, j);

            if (user_type == READSTAT_TYPE_STRING) {
                retval = por_write_string_field_n(writer, ctx, 
                        r_value_label->string_key, r_value_label->string_key_len);
            } else if (user_type == READSTAT_TYPE_DOUBLE) {
                retval = por_write_double(writer, ctx, r_value_label->double_key);
            } else if (user_type == READSTAT_TYPE_INT32) {
                retval = por_write_double(writer, ctx, r_value_label->int32_key);
            }

            if (retval != READSTAT_OK)
                goto cleanup;

            if ((retval = por_write_string_field_n(writer, ctx, 
                            r_value_label->label, r_value_label->label_len)) != READSTAT_OK)
                goto cleanup;
        }
    }

cleanup:
    return retval;
}

static readstat_error_t por_emit_document_record(readstat_writer_t *writer, por_write_ctx_t *ctx) {
    readstat_error_t retval = READSTAT_OK;

    /* The document record is optional; PSPP omits it when there are no
     * document lines */
    if (writer->notes_count == 0)
        return READSTAT_OK;

    if ((retval = por_write_tag(writer, ctx, 'E')) != READSTAT_OK)
        goto cleanup;

    if ((retval = por_write_double(writer, ctx, writer->notes_count)) != READSTAT_OK)
        goto cleanup;

    int i;
    for (i=0; i<writer->notes_count; i++) {
        size_t len = strlen(writer->notes[i]);
        if (len > SPSS_DOC_LINE_SIZE) {
            retval = READSTAT_ERROR_NOTE_IS_TOO_LONG;
            goto cleanup;
        }

        if ((retval = por_write_string_field_n(writer, ctx, writer->notes[i], len)) != READSTAT_OK)
            goto cleanup;
    }

cleanup:
    return retval;
}

static readstat_error_t por_emit_data_tag(readstat_writer_t *writer, por_write_ctx_t *ctx) {
    return por_write_tag(writer, ctx, 'F');
}

static readstat_error_t por_begin_data(void *writer_ctx) {
    readstat_writer_t *writer = (readstat_writer_t *)writer_ctx;
    por_write_ctx_t *ctx = por_write_ctx_init();
    readstat_error_t retval = READSTAT_OK;

    if ((retval = por_emit_header(writer, ctx)) != READSTAT_OK)
        goto cleanup;

    if ((retval = por_emit_version_and_timestamp(writer, ctx)) != READSTAT_OK)
        goto cleanup;

    if ((retval = por_emit_identification_records(writer, ctx)) != READSTAT_OK)
        goto cleanup;

    if ((retval = por_emit_variable_count_record(writer, ctx)) != READSTAT_OK)
        goto cleanup;

    if ((retval = por_emit_precision_record(writer, ctx)) != READSTAT_OK)
        goto cleanup;

    if ((retval = por_emit_case_weight_variable_record(writer, ctx)) != READSTAT_OK)
        goto cleanup;

    if ((retval = por_emit_variable_records(writer, ctx)) != READSTAT_OK)
        goto cleanup;

    if ((retval = por_emit_value_label_records(writer, ctx)) != READSTAT_OK)
        goto cleanup;

    if ((retval = por_emit_document_record(writer, ctx)) != READSTAT_OK)
        goto cleanup;

    if ((retval = por_emit_data_tag(writer, ctx)) != READSTAT_OK)
        goto cleanup;

cleanup:
    if (retval != READSTAT_OK) {
        por_write_ctx_free(ctx);
    } else {
        writer->module_ctx = ctx;
    }

    return retval;
}

static readstat_error_t por_end_data(void *writer_ctx) {
    readstat_writer_t *writer = (readstat_writer_t *)writer_ctx;
    readstat_error_t error = READSTAT_OK;

    if ((error = por_write_tag(writer, writer->module_ctx, 'Z')) != READSTAT_OK)
        goto cleanup;

    if ((error = por_finish(writer)) != READSTAT_OK)
        goto cleanup;

cleanup:
    por_write_ctx_free(writer->module_ctx);
    return error;
}

static size_t por_variable_width(readstat_type_t type, size_t user_width) {
    if (type == READSTAT_TYPE_STRING) {
        /* Length prefix plus the value, which is held as UTF-8 until the row
         * is converted */
        if (user_width > POR_MAX_STRING_WIDTH)
            user_width = POR_MAX_STRING_WIDTH;
        return POR_NUMBER_FIELD_WIDTH + POR_MAX_UTF8_CHAR_LEN * por_string_width(user_width);
    }
    return POR_NUMBER_FIELD_WIDTH;
}

static readstat_error_t por_variable_ok(const readstat_variable_t *variable) {
    if (variable->type == READSTAT_TYPE_STRING && variable->user_width > POR_MAX_STRING_WIDTH)
        return READSTAT_ERROR_STRING_VALUE_IS_TOO_LONG;

    return validate_variable_name(readstat_variable_get_name(variable));
}

static readstat_error_t por_write_double_value(void *row, const readstat_variable_t *var, double value) {
    if (por_write_double_to_buffer(row, POR_NUMBER_FIELD_WIDTH, value, POR_BASE30_PRECISION) == -1) {
        return READSTAT_ERROR_WRITE;
    }

    return READSTAT_OK;
}

static readstat_error_t por_write_int8_value(void *row, const readstat_variable_t *var, int8_t value) {
    return por_write_double_value(row, var, value);
}

static readstat_error_t por_write_int16_value(void *row, const readstat_variable_t *var, int16_t value) {
    return por_write_double_value(row, var, value);
}

static readstat_error_t por_write_int32_value(void *row, const readstat_variable_t *var, int32_t value) {
    return por_write_double_value(row, var, value);
}

static readstat_error_t por_write_float_value(void *row, const readstat_variable_t *var, float value) {
    return por_write_double_value(row, var, value);
}

static readstat_error_t por_write_missing_number(void *row, const readstat_variable_t *var) {
    return por_write_double_value(row, var, NAN);
}

static readstat_error_t por_write_missing_string(void *row, const readstat_variable_t *var) {
    return por_write_double_value(row, var, 0);
}

static readstat_error_t por_write_string_value(void *row, const readstat_variable_t *var, const char *string) {
    size_t len = strlen(string);
    ssize_t char_count = 0;
    ssize_t prefix_len = 0;
    if (len == 0) {
        string = " ";
        len = 1;
    }
    /* The declared width and the length prefix count characters in the
     * file's character set; the UTF-8 bytes stay in the row until
     * por_write_row converts them */
    char_count = por_utf8_count(string, len);
    if (char_count == -1)
        return READSTAT_ERROR_CONVERT_BAD_STRING;
    if (char_count > por_string_width(var->user_width))
        return READSTAT_ERROR_STRING_VALUE_IS_TOO_LONG;

    prefix_len = por_write_double_to_buffer(row, POR_NUMBER_FIELD_WIDTH, char_count, POR_BASE30_PRECISION);
    if (prefix_len == -1)
        return READSTAT_ERROR_WRITE;

    if (prefix_len + len > var->storage_width)
        return READSTAT_ERROR_STRING_VALUE_IS_TOO_LONG;

    memcpy(((char *)row) + prefix_len, string, len);
    return READSTAT_OK;
}

static readstat_error_t por_write_row(void *writer_ctx, void *row, size_t row_len) {
    readstat_writer_t *writer = (readstat_writer_t *)writer_ctx;
    char *row_chars = (char *)row;
    int offset = 0, output = 0;
    for (offset=0; offset<row_len; offset++) {
        if (row_chars[offset]) {
            if (offset != output) {
                row_chars[output] = row_chars[offset];
            }
            output++;
        }
    }
    return por_write_string_n(writer, writer->module_ctx, row_chars, output);
}

static readstat_error_t por_metadata_ok(void *writer_ctx) {
    readstat_writer_t *writer = (readstat_writer_t *)writer_ctx;

    if (writer->compression != READSTAT_COMPRESS_NONE)
        return READSTAT_ERROR_UNSUPPORTED_COMPRESSION;

    return READSTAT_OK;
}

readstat_error_t readstat_begin_writing_por(readstat_writer_t *writer, void *user_ctx, long row_count) {

    writer->callbacks.metadata_ok = &por_metadata_ok;
    writer->callbacks.variable_width = &por_variable_width;
    writer->callbacks.variable_ok = &por_variable_ok;
    writer->callbacks.write_int8 = &por_write_int8_value;
    writer->callbacks.write_int16 = &por_write_int16_value;
    writer->callbacks.write_int32 = &por_write_int32_value;
    writer->callbacks.write_float = &por_write_float_value;
    writer->callbacks.write_double = &por_write_double_value;
    writer->callbacks.write_string = &por_write_string_value;
    writer->callbacks.write_missing_string = &por_write_missing_string;
    writer->callbacks.write_missing_number = &por_write_missing_number;
    writer->callbacks.begin_data = &por_begin_data;
    writer->callbacks.write_row = &por_write_row;
    writer->callbacks.end_data = &por_end_data;

    return readstat_begin_writing_file(writer, user_ctx, row_count);

}
