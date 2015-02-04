//
//  stat.h
//  Wizard
//
//  Created by Evan Miller on 3/31/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#ifndef INCLUDE_READSTAT_H
#define INCLUDE_READSTAT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <sys/types.h>
#include <string.h>
#include <math.h>
#include <stdio.h>

typedef enum readstat_types_e {
    READSTAT_TYPE_STRING,
    READSTAT_TYPE_CHAR,
    READSTAT_TYPE_INT16,
    READSTAT_TYPE_INT32,
    READSTAT_TYPE_FLOAT,
    READSTAT_TYPE_DOUBLE,
    READSTAT_TYPE_LONG_STRING
} readstat_types_t;

typedef enum readstat_error_e {
    READSTAT_OK,
    READSTAT_ERROR_OPEN = 1,
    READSTAT_ERROR_READ,
    READSTAT_ERROR_MALLOC,
    READSTAT_ERROR_USER_ABORT,
    READSTAT_ERROR_PARSE,
    READSTAT_ERROR_UNSUPPORTED_COMPRESSION,
    READSTAT_ERROR_UNSUPPORTED_CHARSET,
    READSTAT_ERROR_ROW_COUNT_MISMATCH,
    READSTAT_ERROR_ROW_WIDTH_MISMATCH
} readstat_error_t;

typedef void * readstat_value_t;

int readstat_value_is_missing(readstat_value_t value);
char readstat_char_value(readstat_value_t value);
int16_t readstat_int16_value(readstat_value_t value);
int32_t readstat_int32_value(readstat_value_t value);
float readstat_float_value(readstat_value_t value);
double readstat_double_value(readstat_value_t value);
char *readstat_string_value(readstat_value_t value);

/* Callbacks should return 0 on success and non-zero to abort */
typedef int (*readstat_handle_info_callback)(int obs_count, int var_count, void *ctx);
typedef int (*readstat_handle_variable_callback)(int index, const char *var_name, 
        const char *var_format, const char *var_label, const char *val_labels, 
        readstat_types_t type, size_t max_len, void *ctx);
typedef int (*readstat_handle_value_callback)(int obs_index, int var_index, 
        readstat_value_t value, readstat_types_t type, void *ctx);
typedef int (*readstat_handle_value_label_callback)(const char *val_labels, 
        readstat_value_t value, readstat_types_t type, const char *label, void *ctx);

readstat_error_t parse_dta(const char *filename, void *user_ctx, 
        readstat_handle_info_callback info_cb, 
        readstat_handle_variable_callback variable_cb,
        readstat_handle_value_callback value_cb, 
        readstat_handle_value_label_callback value_label_cb);

readstat_error_t parse_por(const char *filename, void *user_ctx,
        readstat_handle_info_callback info_cb, 
        readstat_handle_variable_callback variable_cb,
        readstat_handle_value_callback value_cb, 
        readstat_handle_value_label_callback value_label_cb);

readstat_error_t parse_sav(const char *filename, void *user_ctx,
        readstat_handle_info_callback info_cb, 
        readstat_handle_variable_callback variable_cb,
        readstat_handle_value_callback value_cb, 
        readstat_handle_value_label_callback value_label_cb);

readstat_error_t parse_sas7bdat(const char *filename, void *user_ctx, 
        readstat_handle_info_callback info_cb, 
        readstat_handle_variable_callback variable_cb,
        readstat_handle_value_callback value_cb);

readstat_error_t parse_sas7bcat(const char *filename, void *user_ctx,
        readstat_handle_value_label_callback value_label_cb);

typedef int (*readstat_handle_column_callback)(const char *name, readstat_types_t type, char *format, 
        void *data, long count, void *ctx);
typedef int (*readstat_handle_table_callback)(const char *name, void *ctx);
typedef int (*readstat_handle_text_value_callback)(const char *value, int index, void *ctx);
typedef readstat_handle_text_value_callback readstat_handle_column_name_callback;

readstat_error_t parse_rds(const char *filename, void *user_ctx,
        readstat_handle_column_callback handle_column,
        readstat_handle_column_name_callback handle_column_name,
        readstat_handle_text_value_callback handle_text_value,
        readstat_handle_text_value_callback handle_value_label);
readstat_error_t parse_rdata(const char *filename, void *user_ctx, 
        readstat_handle_table_callback handle_table,
        readstat_handle_column_callback handle_column,
        readstat_handle_column_name_callback handle_column_name,
        readstat_handle_text_value_callback handle_text_value,
        readstat_handle_text_value_callback handle_value_label);

const char *readstat_error_message(readstat_error_t error_code);

#ifdef __cplusplus
}
#endif

#endif
