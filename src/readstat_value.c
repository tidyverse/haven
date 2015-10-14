
#include "readstat.h"

readstat_types_t readstat_value_type(readstat_value_t value) {
    return value.type;
}

char readstat_value_tag(readstat_value_t value) {
    return value.tag;
}

int readstat_value_is_missing(readstat_value_t value) {
    return (value.is_system_missing || value.is_considered_missing);
}

int readstat_value_is_system_missing(readstat_value_t value) {
    return (value.is_system_missing);
}

int readstat_value_is_considered_missing(readstat_value_t value) {
    return (value.is_considered_missing);
}

char readstat_char_value(readstat_value_t value) {
    if (readstat_value_is_system_missing(value))
        return 0;

    if (value.type == READSTAT_TYPE_DOUBLE)
        return value.v.double_value;
    if (value.type == READSTAT_TYPE_FLOAT)
        return value.v.float_value;
    if (value.type == READSTAT_TYPE_INT32)
        return value.v.i32_value;
    if (value.type == READSTAT_TYPE_INT16)
        return value.v.i16_value;
    if (value.type == READSTAT_TYPE_CHAR)
        return value.v.char_value;

    return 0;
}

int16_t readstat_int16_value(readstat_value_t value) {
    if (readstat_value_is_system_missing(value))
        return 0;

    if (value.type == READSTAT_TYPE_DOUBLE)
        return value.v.double_value;
    if (value.type == READSTAT_TYPE_FLOAT)
        return value.v.float_value;
    if (value.type == READSTAT_TYPE_INT32)
        return value.v.i32_value;
    if (value.type == READSTAT_TYPE_INT16)
        return value.v.i16_value;
    if (value.type == READSTAT_TYPE_CHAR)
        return value.v.char_value;

    return 0;
}

int32_t readstat_int32_value(readstat_value_t value) {
    if (readstat_value_is_system_missing(value))
        return 0;

    if (value.type == READSTAT_TYPE_DOUBLE)
        return value.v.double_value;
    if (value.type == READSTAT_TYPE_FLOAT)
        return value.v.float_value;
    if (value.type == READSTAT_TYPE_INT32)
        return value.v.i32_value;
    if (value.type == READSTAT_TYPE_INT16)
        return value.v.i16_value;
    if (value.type == READSTAT_TYPE_CHAR)
        return value.v.char_value;

    return 0;
}

float readstat_float_value(readstat_value_t value) {
    if (readstat_value_is_system_missing(value))
        return NAN;

    if (value.type == READSTAT_TYPE_DOUBLE)
        return value.v.double_value;
    if (value.type == READSTAT_TYPE_FLOAT)
        return value.v.float_value;
    if (value.type == READSTAT_TYPE_INT32)
        return value.v.i32_value;
    if (value.type == READSTAT_TYPE_INT16)
        return value.v.i16_value;
    if (value.type == READSTAT_TYPE_CHAR)
        return value.v.char_value;

    return value.v.float_value;
}

double readstat_double_value(readstat_value_t value) {
    if (readstat_value_is_system_missing(value))
        return NAN;

    if (value.type == READSTAT_TYPE_DOUBLE)
        return value.v.double_value;
    if (value.type == READSTAT_TYPE_FLOAT)
        return value.v.float_value;
    if (value.type == READSTAT_TYPE_INT32)
        return value.v.i32_value;
    if (value.type == READSTAT_TYPE_INT16)
        return value.v.i16_value;
    if (value.type == READSTAT_TYPE_CHAR)
        return value.v.char_value;

    return NAN;
}

char *readstat_string_value(readstat_value_t value) {
    if (value.type == READSTAT_TYPE_STRING || value.type == READSTAT_TYPE_LONG_STRING)
        return value.v.string_value;

    return NULL;
}
