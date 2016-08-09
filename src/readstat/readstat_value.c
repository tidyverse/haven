
#include "readstat.h"

readstat_type_class_t readstat_type_class(readstat_type_t type) {
    if (type == READSTAT_TYPE_STRING || type == READSTAT_TYPE_STRING_REF)
        return READSTAT_TYPE_CLASS_STRING;

    return READSTAT_TYPE_CLASS_NUMERIC;
}

readstat_type_t readstat_value_type(readstat_value_t value) {
    return value.type;
}

readstat_type_class_t readstat_value_type_class(readstat_value_t value) {
    return readstat_type_class(value.type);
}

char readstat_value_tag(readstat_value_t value) {
    return value.tag;
}

int readstat_value_is_missing(readstat_value_t value) {
    return (value.is_system_missing || value.is_tagged_missing || value.is_defined_missing);
}

int readstat_value_is_system_missing(readstat_value_t value) {
    return (value.is_system_missing);
}

int readstat_value_is_tagged_missing(readstat_value_t value) {
    return (value.is_tagged_missing);
}

int readstat_value_is_defined_missing(readstat_value_t value) {
    return (value.is_defined_missing);
}

char readstat_int8_value(readstat_value_t value) {
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
    if (value.type == READSTAT_TYPE_INT8)
        return value.v.i8_value;

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
    if (value.type == READSTAT_TYPE_INT8)
        return value.v.i8_value;

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
    if (value.type == READSTAT_TYPE_INT8)
        return value.v.i8_value;

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
    if (value.type == READSTAT_TYPE_INT8)
        return value.v.i8_value;

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
    if (value.type == READSTAT_TYPE_INT8)
        return value.v.i8_value;

    return NAN;
}

const char *readstat_string_value(readstat_value_t value) {
    if (readstat_value_type(value) == READSTAT_TYPE_STRING)
        return value.v.string_value;

    return NULL;
}
