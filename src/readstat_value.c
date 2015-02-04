
#include "readstat.h"

int readstat_value_is_missing(readstat_value_t value) {
    return (value == NULL);
}

char readstat_char_value(readstat_value_t value) {
    if (readstat_value_is_missing(value))
        return 0;

    return *(char *)value;
}

int16_t readstat_int16_value(readstat_value_t value) {
    if (readstat_value_is_missing(value))
        return 0;

    return *(int16_t *)value;
}

int32_t readstat_int32_value(readstat_value_t value) {
    if (readstat_value_is_missing(value))
        return 0;

    return *(int32_t *)value;
}

float readstat_float_value(readstat_value_t value) {
    if (readstat_value_is_missing(value))
        return NAN;

    return *(float *)value;
}

double readstat_double_value(readstat_value_t value) {
    if (readstat_value_is_missing(value))
        return NAN;

    return *(double *)value;
}

char *readstat_string_value(readstat_value_t value) {
    return (char *)value;
}
