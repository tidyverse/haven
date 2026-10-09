#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "../readstat.h"
#include "../readstat_malloc.h"
#include "../readstat_iconv.h"
#include "../readstat_convert.h"
#include "readstat_sav.h"
#include "readstat_sav_parse_mr_name.h"

/* Multiple response set records (type 7, subtypes 7 and 19) hold one set per
 * line. See the "Multiple Response Sets Records" section of the PSPP system
 * file format documentation. Each line looks like one of:
 *
 *   $name=C <label_len> <label> <var> <var> ...
 *   $name=D<value_len> <value> <label_len> <label> <var> <var> ...
 *   $name=E <1|11> <value_len> <value> <label_len> <label> <var> <var> ...
 */

static void mr_set_free_contents(mr_set_t *mr) {
    if (mr->name)
        free(mr->name);
    if (mr->label)
        free(mr->label);
    if (mr->counted_string)
        free(mr->counted_string);
    if (mr->subvariables) {
        int i;
        for (i=0; i<mr->num_subvars; i++) {
            if (mr->subvariables[i])
                free(mr->subvariables[i]);
        }
        free(mr->subvariables);
    }
    memset(mr, 0, sizeof(mr_set_t));
}

static readstat_error_t mr_convert(char **out, const char *src, size_t src_len, sav_ctx_t *ctx) {
    size_t dst_len = 4 * src_len + 1; /* UTF-8 expansion: up to 4 bytes per char */
    char *dst = readstat_malloc(dst_len);
    if (dst == NULL)
        return READSTAT_ERROR_MALLOC;
    readstat_error_t retval = readstat_convert(dst, dst_len, src, src_len, ctx->converter);
    if (retval != READSTAT_OK) {
        free(dst);
        return retval;
    }
    *out = dst;
    return READSTAT_OK;
}

/* Parses a decimal count followed by a single space, then that many bytes.
 * On success *pp points at the byte after the string. */
static readstat_error_t mr_read_counted_string(const char **pp, const char *end,
        const char **str, size_t *str_len) {
    const char *p = *pp;
    size_t n = 0;
    if (p == end || *p < '0' || *p > '9')
        return READSTAT_ERROR_BAD_MR_STRING;
    while (p < end && *p >= '0' && *p <= '9') {
        if (n > (size_t)(end - p))
            return READSTAT_ERROR_BAD_MR_STRING;
        n = n * 10 + (*p - '0');
        p++;
    }
    if (p == end || *p != ' ')
        return READSTAT_ERROR_BAD_MR_STRING;
    p++;
    if (n > (size_t)(end - p))
        return READSTAT_ERROR_BAD_MR_STRING;
    *str = p;
    *str_len = n;
    *pp = p + n;
    return READSTAT_OK;
}

static readstat_error_t parse_mr_line(const char *line, size_t line_len, mr_set_t *result, sav_ctx_t *ctx) {
    readstat_error_t retval = READSTAT_OK;
    const char *p = line;
    const char *end = line + line_len;
    const char *str = NULL;
    size_t str_len = 0;

    memset(result, 0, sizeof(mr_set_t));

    /* Name: begins with '$', runs up to '=' */
    if (p == end || *p != '$') {
        retval = READSTAT_ERROR_BAD_MR_STRING;
        goto cleanup;
    }
    str = p;
    while (p < end && *p != '=' && *p != ' ')
        p++;
    if (p == end || *p != '=' || p == str) {
        retval = READSTAT_ERROR_BAD_MR_STRING;
        goto cleanup;
    }
    if ((retval = mr_convert(&result->name, str, p - str, ctx)) != READSTAT_OK)
        goto cleanup;
    p++;

    /* Type */
    if (p == end || (*p != 'C' && *p != 'D' && *p != 'E')) {
        retval = READSTAT_ERROR_BAD_MR_STRING;
        goto cleanup;
    }
    result->type = *p++;
    result->counted_value = -1;

    if (result->type == 'E') {
        /* " 1 " or " 11 ": 11 means LABELSOURCE=VARLABEL */
        long flag = 0;
        if (p == end || *p != ' ') {
            retval = READSTAT_ERROR_BAD_MR_STRING;
            goto cleanup;
        }
        p++;
        if (p == end || *p < '0' || *p > '9') {
            retval = READSTAT_ERROR_BAD_MR_STRING;
            goto cleanup;
        }
        while (p < end && *p >= '0' && *p <= '9' && flag < 1000)
            flag = flag * 10 + (*p++ - '0');
        if (p == end || *p != ' ') {
            retval = READSTAT_ERROR_BAD_MR_STRING;
            goto cleanup;
        }
        p++;
        result->label_from_var_label = (flag == 11);
    }

    if (result->type == 'D' || result->type == 'E') {
        result->is_dichotomy = 1;
        if ((retval = mr_read_counted_string(&p, end, &str, &str_len)) != READSTAT_OK)
            goto cleanup;
        if ((retval = mr_convert(&result->counted_string, str, str_len, ctx)) != READSTAT_OK)
            goto cleanup;
        /* readstat_convert trims trailing spaces; older SPSS pads to 8 bytes */
        if (result->counted_string[0] >= '0' && result->counted_string[0] <= '9') {
            result->counted_value = (int)strtol(result->counted_string, NULL, 10);
        }
        if (p == end || *p != ' ') {
            retval = READSTAT_ERROR_BAD_MR_STRING;
            goto cleanup;
        }
        p++;
    } else {
        if (p == end || *p != ' ') {
            retval = READSTAT_ERROR_BAD_MR_STRING;
            goto cleanup;
        }
        p++;
    }

    /* Label */
    if ((retval = mr_read_counted_string(&p, end, &str, &str_len)) != READSTAT_OK)
        goto cleanup;
    if ((retval = mr_convert(&result->label, str, str_len, ctx)) != READSTAT_OK)
        goto cleanup;

    /* Variables: space-separated; some files have zero or one variable */
    while (p < end) {
        if (*p == ' ') {
            p++;
            continue;
        }
        str = p;
        while (p < end && *p != ' ')
            p++;
        char *subvar = NULL;
        if ((retval = mr_convert(&subvar, str, p - str, ctx)) != READSTAT_OK)
            goto cleanup;
        char **new_subvariables = readstat_realloc(result->subvariables,
                sizeof(char *) * (result->num_subvars + 1));
        if (new_subvariables == NULL) {
            free(subvar);
            retval = READSTAT_ERROR_MALLOC;
            goto cleanup;
        }
        result->subvariables = new_subvariables;
        result->subvariables[result->num_subvars++] = subvar;
    }

cleanup:
    if (retval != READSTAT_OK) {
        mr_set_free_contents(result);
    }
    return retval;
}

readstat_error_t parse_mr_string(const char *data, size_t data_len, mr_set_t **mr_sets, size_t *n_mr_lines, sav_ctx_t *ctx) {
    readstat_error_t retval = READSTAT_OK;
    const char *p = data;
    const char *end = data + data_len;

    while (p < end) {
        const char *line = p;
        while (p < end && *p != '\n' && *p != '\0')
            p++;
        size_t line_len = p - line;
        if (p < end)
            p++; /* consume the line feed */
        if (line_len && line[line_len-1] == '\r')
            line_len--;
        if (line_len == 0)
            continue; /* the spec allows any number of line feeds between sets */

        mr_set_t *new_mr_sets = readstat_realloc(*mr_sets, ((*n_mr_lines) + 1) * sizeof(mr_set_t));
        if (new_mr_sets == NULL) {
            retval = READSTAT_ERROR_MALLOC;
            goto cleanup;
        }
        *mr_sets = new_mr_sets;

        retval = parse_mr_line(line, line_len, &(*mr_sets)[*n_mr_lines], ctx);
        if (retval != READSTAT_OK)
            goto cleanup;

        (*n_mr_lines)++;
    }

cleanup:
    return retval;
}
