
#include <stdlib.h>
#include <stdio.h>

#include "../readstat.h"
#include "readstat_spss.h"
#include "readstat_spss_parse.h"

typedef struct spss_type_s {
    int     type;
    char    string[100];
} spss_type_t;

static spss_type_t spss_type_strings[] = {
    { .type = SPSS_FORMAT_TYPE_A,      .string = "A" },
    { .type = SPSS_FORMAT_TYPE_AHEX,   .string = "AHEX" },
    { .type = SPSS_FORMAT_TYPE_COMMA,  .string = "COMMA" },
    { .type = SPSS_FORMAT_TYPE_DOLLAR, .string = "DOLLAR" },
    { .type = SPSS_FORMAT_TYPE_F,      .string = "F" },
    { .type = SPSS_FORMAT_TYPE_IB,     .string = "IB" },
    { .type = SPSS_FORMAT_TYPE_PIBHEX, .string = "PIBHEX" },
    { .type = SPSS_FORMAT_TYPE_P,      .string = "P" },
    { .type = SPSS_FORMAT_TYPE_PIB,    .string = "PIB" },
    { .type = SPSS_FORMAT_TYPE_PK,     .string = "PK" },
    { .type = SPSS_FORMAT_TYPE_RB,     .string = "RB" },
    { .type = SPSS_FORMAT_TYPE_RBHEX,  .string = "RBHEX" },
    { .type = SPSS_FORMAT_TYPE_Z,      .string = "Z" },
    { .type = SPSS_FORMAT_TYPE_N,      .string = "N" },
    { .type = SPSS_FORMAT_TYPE_E,      .string = "E" },
    { .type = SPSS_FORMAT_TYPE_DATE,   .string = "DATE" },
    { .type = SPSS_FORMAT_TYPE_TIME,   .string = "TIME" },
    { .type = SPSS_FORMAT_TYPE_DATETIME,.string = "DATETIME" },
    { .type = SPSS_FORMAT_TYPE_ADATE,  .string = "ADATE" },
    { .type = SPSS_FORMAT_TYPE_JDATE,  .string = "JDATE" },
    { .type = SPSS_FORMAT_TYPE_DTIME,  .string = "DTIME" },
    { .type = SPSS_FORMAT_TYPE_WKDAY,  .string = "WKDAY" },
    { .type = SPSS_FORMAT_TYPE_MONTH,  .string = "MONTH" },
    { .type = SPSS_FORMAT_TYPE_MOYR,   .string = "MOYR" },
    { .type = SPSS_FORMAT_TYPE_QYR,    .string = "QYR" },
    { .type = SPSS_FORMAT_TYPE_WKYR,   .string = "WKYR" },
    { .type = SPSS_FORMAT_TYPE_PCT,    .string = "PCT" },
    { .type = SPSS_FORMAT_TYPE_DOT,    .string = "DOT" },
    { .type = SPSS_FORMAT_TYPE_CCA,    .string = "CCA" },
    { .type = SPSS_FORMAT_TYPE_CCB,    .string = "CCB" },
    { .type = SPSS_FORMAT_TYPE_CCC,    .string = "CCC" },
    { .type = SPSS_FORMAT_TYPE_CCD,    .string = "CCD" },
    { .type = SPSS_FORMAT_TYPE_CCE,    .string = "CCE" },
    { .type = SPSS_FORMAT_TYPE_EDATE,  .string = "EDATE" },
    { .type = SPSS_FORMAT_TYPE_SDATE,  .string = "SDATE" }
};

int spss_format(char *buffer, size_t len, spss_format_t *format) {
    int i;
    for (i=0; i < sizeof(spss_type_strings)/sizeof(spss_type_strings[0]); i++) {
        if (format->type == spss_type_strings[i].type) {
            if (format->decimal_places || format->type == SPSS_FORMAT_TYPE_F) {
                snprintf(buffer, len, "%s%d.%d", spss_type_strings[i].string, format->width, format->decimal_places);
            } else if (format->width) {
                snprintf(buffer, len, "%s%d", spss_type_strings[i].string, format->width);
            } else {
                snprintf(buffer, len, "%s", spss_type_strings[i].string);
            }
            return 1;
        }
    }
    return 0;
}

int spss_varinfo_compare(const void *elem1, const void *elem2) {
    int offset = *(int *)elem1;
    const spss_varinfo_t *v = (const spss_varinfo_t *)elem2;
    if (offset < v->offset)
        return -1;
    return (offset > v->offset);
}

static readstat_value_t spss_boxed_value(double fp_value) {
    readstat_value_t value = {
        .type = READSTAT_TYPE_DOUBLE,
        .v = { .double_value = fp_value },
        .is_system_missing = isnan(fp_value)
    };
    return value;
}

uint64_t spss_64bit_value(readstat_value_t value) {
    double dval = readstat_double_value(value);
    uint64_t special_val;
    memcpy(&special_val, &dval, sizeof(double));

    if (isinf(dval)) {
        if (dval < 0.0) {
            special_val = SAV_LOWEST_DOUBLE;
        } else {
            special_val = SAV_HIGHEST_DOUBLE;
        }
    } else if (isnan(dval)) {
        special_val = SAV_MISSING_DOUBLE;
    }
    return special_val;
}

readstat_missingness_t spss_missingness_for_info(spss_varinfo_t *info) {
    readstat_missingness_t missingness;
    memset(&missingness, 0, sizeof(readstat_missingness_t));

    if (info->missing_range) {
        missingness.missing_ranges_count++;
        missingness.missing_ranges[0] = spss_boxed_value(info->missing_values[0]);
        missingness.missing_ranges[1] = spss_boxed_value(info->missing_values[1]);

        if (info->n_missing_values == 3) {
            missingness.missing_ranges_count++;
            missingness.missing_ranges[2] = missingness.missing_ranges[3] = spss_boxed_value(info->missing_values[2]);
        }
    } else if (info->n_missing_values > 0) {
        missingness.missing_ranges_count = info->n_missing_values;
        int i=0;
        for (i=0; i<info->n_missing_values; i++) {
            missingness.missing_ranges[2*i] = missingness.missing_ranges[2*i+1] = spss_boxed_value(info->missing_values[i]);
        }
    }
    return missingness;
}

readstat_variable_t *spss_init_variable_for_info(spss_varinfo_t *info, int index_after_skipping) {
    readstat_variable_t *variable = calloc(1, sizeof(readstat_variable_t));

    variable->index = info->index;
    variable->index_after_skipping = index_after_skipping;
    variable->type = info->type;
    if (info->string_length) {
        variable->storage_width = info->string_length;
    } else {
        variable->storage_width = 8 * info->width;
    }

    if (info->longname[0]) {
        snprintf(variable->name, sizeof(variable->name), "%s", info->longname);
    } else {
        snprintf(variable->name, sizeof(variable->name), "%s", info->name);
    }
    if (info->label) {
        snprintf(variable->label, sizeof(variable->label), "%s", info->label);
    }

    spss_format(variable->format, sizeof(variable->format), &info->print_format);

    variable->missingness = spss_missingness_for_info(info);
    variable->measure = info->measure;
    variable->display_width = info->display_width;

    return variable;
}

int32_t spss_measure_from_readstat_measure(readstat_measure_t measure) {
    int32_t sav_measure = SAV_MEASURE_UNKNOWN;
    if (measure == READSTAT_MEASURE_NOMINAL) {
        sav_measure = SAV_MEASURE_NOMINAL;
    } else if (measure == READSTAT_MEASURE_ORDINAL) {
        sav_measure = SAV_MEASURE_ORDINAL;
    } else if (measure == READSTAT_MEASURE_SCALE) {
        sav_measure = SAV_MEASURE_SCALE;
    }
    return sav_measure;
}

readstat_measure_t spss_measure_to_readstat_measure(int32_t sav_measure) {
    if (sav_measure == SAV_MEASURE_NOMINAL)
        return READSTAT_MEASURE_NOMINAL;
    if (sav_measure == SAV_MEASURE_ORDINAL)
        return READSTAT_MEASURE_ORDINAL;
    if (sav_measure == SAV_MEASURE_SCALE)
        return READSTAT_MEASURE_SCALE;
    return READSTAT_MEASURE_UNKNOWN;
}

int32_t spss_alignment_from_readstat_alignment(readstat_alignment_t alignment) {
    int32_t sav_alignment = 0;
    if (alignment == READSTAT_ALIGNMENT_LEFT) {
        sav_alignment = SAV_ALIGNMENT_LEFT;
    } else if (alignment == READSTAT_ALIGNMENT_CENTER) {
        sav_alignment = SAV_ALIGNMENT_CENTER;
    } else if (alignment == READSTAT_ALIGNMENT_RIGHT) {
        sav_alignment = SAV_ALIGNMENT_RIGHT;
    }
    return sav_alignment;
}

readstat_alignment_t spss_alignment_to_readstat_alignment(int32_t sav_alignment) {
    if (sav_alignment == SAV_ALIGNMENT_LEFT)
        return READSTAT_ALIGNMENT_LEFT;
    if (sav_alignment == SAV_ALIGNMENT_CENTER)
        return READSTAT_ALIGNMENT_CENTER;
    if (sav_alignment == SAV_ALIGNMENT_RIGHT)
        return READSTAT_ALIGNMENT_RIGHT;
    return READSTAT_ALIGNMENT_UNKNOWN;
}

readstat_error_t spss_format_for_variable(readstat_variable_t *r_variable,
        spss_format_t *spss_format) {
    readstat_error_t retval = READSTAT_OK;

    memset(spss_format, 0, sizeof(spss_format_t));

    if (r_variable->type == READSTAT_TYPE_STRING) {
        spss_format->type = SPSS_FORMAT_TYPE_A;
        if (r_variable->user_width) {
            spss_format->width = r_variable->user_width;
        } else {
            spss_format->width = r_variable->storage_width;
        }
    } else {
        spss_format->type = SPSS_FORMAT_TYPE_F;
        spss_format->width = 8;
        if (r_variable->type == READSTAT_TYPE_DOUBLE ||
                r_variable->type == READSTAT_TYPE_FLOAT) {
            spss_format->decimal_places = 2;
        }
    }

    if (r_variable->format[0]) {
        spss_format->decimal_places = 0;
        const char *fmt = r_variable->format;
        if (spss_parse_format(fmt, strlen(fmt), spss_format) != READSTAT_OK) {
            retval = READSTAT_ERROR_BAD_FORMAT_STRING;
            goto cleanup;
        }
    }

cleanup:
    return retval;
}
