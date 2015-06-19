
#include <stdlib.h>
#include "readstat.h"

const char *readstat_variable_get_name(readstat_variable_t *variable) {
    if (variable->name[0])
        return variable->name;

    return NULL;
}

const char *readstat_variable_get_label(readstat_variable_t *variable) {
    if (variable->label[0])
        return variable->label;

    return NULL;
}

const char *readstat_variable_get_format(readstat_variable_t *variable) {
    if (variable->format[0])
        return variable->format;

    return NULL;
}

readstat_types_t readstat_variable_get_type(readstat_variable_t *variable) {
    return variable->type;
}

int readstat_variable_get_missing_ranges_count(readstat_variable_t *variable) {
    return variable->missingness.missing_ranges_count;
}

static readstat_value_t make_blank_value() {
    readstat_value_t value = { .is_system_missing = 1, .v = { .double_value = NAN }, .type = READSTAT_TYPE_DOUBLE };
    return value;
}

readstat_value_t readstat_variable_get_missing_range_lo(readstat_variable_t *variable, int i) {
    if (i < variable->missingness.missing_ranges_count &&
            2*i+1 < sizeof(variable->missingness.missing_ranges)/sizeof(variable->missingness.missing_ranges[0])) {
        return variable->missingness.missing_ranges[2*i];
    }

    return make_blank_value();
}

readstat_value_t readstat_variable_get_missing_range_hi(readstat_variable_t *variable, int i) {
    if (i < variable->missingness.missing_ranges_count &&
            2*i+1 < sizeof(variable->missingness.missing_ranges)/sizeof(variable->missingness.missing_ranges[0])) {
        return variable->missingness.missing_ranges[2*i+1];
    }

    return make_blank_value();
}
