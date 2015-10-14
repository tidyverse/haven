
#include <stdlib.h>
#include <stdio.h>

#include "readstat.h"
#include "readstat_spss.h"

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

void spss_tag_missing_double(readstat_value_t *value, readstat_missingness_t *missingness) {
    double fp_value = value->v.double_value;
    value->is_system_missing = isnan(fp_value);
    if (missingness) {
        int i;
        for (i=0; i<missingness->missing_ranges_count; i++) {
            double lo = readstat_double_value(missingness->missing_ranges[2*i]);
            double hi = readstat_double_value(missingness->missing_ranges[2*i+1]);
            if (fp_value >= lo && fp_value <= hi) {
                value->is_considered_missing = 1;
                break;
            }
        }
    }
    uint64_t long_value = 0;
    memcpy(&long_value, &fp_value, 8);
    if (long_value == SAV_MISSING_DOUBLE)
        value->is_system_missing = 1;
    if (long_value == SAV_LOWEST_DOUBLE)
        value->is_system_missing = 1;
    if (long_value == SAV_HIGHEST_DOUBLE)
        value->is_system_missing = 1;
}

int spss_varinfo_compare(const void *elem1, const void *elem2) {
    int offset = *(int *)elem1;
    const spss_varinfo_t *v = (const spss_varinfo_t *)elem2;
    if (offset < v->offset)
        return -1;
    return (offset > v->offset);
}

readstat_value_t spss_boxed_value(double fp_value) {
    readstat_value_t value;
    memset(&value, 0, sizeof(readstat_value_t));

    value.v.double_value = fp_value;
    value.type = READSTAT_TYPE_DOUBLE;

    uint64_t long_value = 0;
    memcpy(&long_value, &fp_value, 8);

    if (long_value == SAV_MISSING_DOUBLE)
        value.is_system_missing = 1;
    if (long_value == SAV_LOWEST_DOUBLE)
        value.v.double_value = -HUGE_VAL;
    if (long_value == SAV_HIGHEST_DOUBLE)
        value.v.double_value = HUGE_VAL;

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

readstat_variable_t *spss_init_variable_for_info(spss_varinfo_t *info) {
    readstat_variable_t *variable = calloc(1, sizeof(readstat_variable_t));

    variable->index = info->index;
    variable->type = info->type;

    if (info->longname[0]) {
        snprintf(variable->name, sizeof(variable->name), "%s", info->longname);
    } else {
        snprintf(variable->name, sizeof(variable->name), "%s", info->name);
    }
    if (info->label) {
        snprintf(variable->label, sizeof(variable->label), "%s", info->label);
    }

    spss_format(variable->format, sizeof(variable->format), &info->print_format);

    variable->missingness = info->missingness;

    return variable;
}

void spss_free_variable(readstat_variable_t *variable) {
    free(variable);
}
