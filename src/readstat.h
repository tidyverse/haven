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
    READSTAT_ERROR_ROW_WIDTH_MISMATCH,
    READSTAT_ERROR_BAD_FORMAT_STRING,
    READSTAT_ERROR_VALUE_TYPE_MISMATCH,
    READSTAT_ERROR_WRITE
} readstat_error_t;

const char *readstat_error_message(readstat_error_t error_code);

typedef void * readstat_value_t;

int readstat_value_is_missing(readstat_value_t value);
char readstat_char_value(readstat_value_t value);
int16_t readstat_int16_value(readstat_value_t value);
int32_t readstat_int32_value(readstat_value_t value);
float readstat_float_value(readstat_value_t value);
double readstat_double_value(readstat_value_t value);
char *readstat_string_value(readstat_value_t value);

/* Callbacks should return 0 on success and non-zero to abort */
typedef int (*readstat_info_handler)(int obs_count, int var_count, void *ctx);
typedef int (*readstat_variable_handler)(int index, const char *var_name, 
        const char *var_format, const char *var_label, const char *val_labels, 
        readstat_types_t type, void *ctx);
typedef int (*readstat_value_handler)(int obs_index, int var_index, 
        readstat_value_t value, readstat_types_t type, void *ctx);
typedef int (*readstat_value_label_handler)(const char *val_labels, 
        readstat_value_t value, readstat_types_t type, const char *label, void *ctx);
typedef void (*readstat_error_handler)(const char *error_message);

typedef struct readstat_parser_s {
    readstat_info_handler          info_handler;
    readstat_variable_handler      variable_handler;
    readstat_value_handler         value_handler;
    readstat_value_label_handler   value_label_handler;
    readstat_error_handler         error_handler;
} readstat_parser_t;

readstat_parser_t *readstat_parser_init();
void readstat_parser_free(readstat_parser_t *parser);

readstat_error_t readstat_set_info_handler(readstat_parser_t *parser, readstat_info_handler info_handler);
readstat_error_t readstat_set_variable_handler(readstat_parser_t *parser, readstat_variable_handler variable_handler);
readstat_error_t readstat_set_value_handler(readstat_parser_t *parser, readstat_value_handler value_handler);
readstat_error_t readstat_set_value_label_handler(readstat_parser_t *parser, readstat_value_label_handler value_label_handler);
readstat_error_t readstat_set_error_handler(readstat_parser_t *parser, readstat_error_handler error_handler);

readstat_error_t readstat_parse_dta(readstat_parser_t *parser, const char *filename, void *user_ctx);
readstat_error_t readstat_parse_sav(readstat_parser_t *parser, const char *filename, void *user_ctx);
readstat_error_t readstat_parse_por(readstat_parser_t *parser, const char *filename, void *user_ctx);
readstat_error_t readstat_parse_sas7bdat(readstat_parser_t *parser, const char *filename, void *user_ctx);
readstat_error_t readstat_parse_sas7bcat(readstat_parser_t *parser, const char *filename, void *user_ctx);


/* Internal data structures & module callbacks */
typedef struct readstat_value_label_s {
    double      double_key;
    int32_t     int32_key;
    char        string_key[16];

    char        label[256];
} readstat_value_label_t;

typedef struct readstat_label_set_s {
    readstat_types_t            type;
    char                        name[256];

    readstat_value_label_t     *value_labels;
    long                        value_labels_count;
    long                        value_labels_capacity;

    void                       *variables;
    long                        variables_count;
    long                        variables_capacity;
} readstat_label_set_t;

typedef struct readstat_variable_s {
    readstat_types_t        type;
    char                    name[256];
    char                    format[256];
    char                    label[1024];
    readstat_label_set_t   *label_set;
    off_t                   offset;
    size_t                  width;
    size_t                  user_width;
} readstat_variable_t;

typedef size_t (*readstat_variable_width_callback)(readstat_types_t type, size_t user_width);

typedef readstat_error_t (*readstat_write_char_callback)(void *row_data, const readstat_variable_t *variable, char value);
typedef readstat_error_t (*readstat_write_int16_callback)(void *row_data, const readstat_variable_t *variable, int16_t value);
typedef readstat_error_t (*readstat_write_int32_callback)(void *row_data, const readstat_variable_t *variable, int32_t value);
typedef readstat_error_t (*readstat_write_float_callback)(void *row_data, const readstat_variable_t *variable, float value);
typedef readstat_error_t (*readstat_write_double_callback)(void *row_data, const readstat_variable_t *variable, double value);
typedef readstat_error_t (*readstat_write_string_callback)(void *row_data, const readstat_variable_t *variable, const char *value);
typedef readstat_error_t (*readstat_write_missing_callback)(void *row_data, const readstat_variable_t *variable);

typedef readstat_error_t (*readstat_begin_data_callback)(void *writer);
typedef readstat_error_t (*readstat_end_data_callback)(void *writer);

typedef struct readstat_writer_callbacks_s {
    readstat_variable_width_callback   variable_width;
    readstat_write_char_callback    write_char;
    readstat_write_int16_callback   write_int16;
    readstat_write_int32_callback   write_int32;
    readstat_write_float_callback   write_float;
    readstat_write_double_callback  write_double;
    readstat_write_string_callback  write_string;
    readstat_write_missing_callback write_missing;
    readstat_begin_data_callback    begin_data;
    readstat_end_data_callback      end_data;
} readstat_writer_callbacks_t;

/* You'll need to define one of these to get going. Should return # bytes written,
 * or -1 on error, a la write(2) */
typedef ssize_t (*readstat_data_writer)(const void *data, size_t len, void *ctx);

typedef struct readstat_writer_s {
    readstat_data_writer        data_writer;

    readstat_variable_t       **variables;
    long                        variables_count;
    long                        variables_capacity;

    readstat_label_set_t      **label_sets;
    long                        label_sets_count;
    long                        label_sets_capacity;

    unsigned char              *row;
    size_t                      row_len;

    int                         row_count;
    int                         current_row;
    char                        file_label[80];
    readstat_writer_callbacks_t callbacks;
    void                       *module_ctx;
    void                       *user_ctx;
} readstat_writer_t;

/* Writer API */


// First call this...
readstat_writer_t *readstat_writer_init();

// Then specify a function that will handle the output bytes...
readstat_error_t readstat_set_data_writer(readstat_writer_t *writer, readstat_data_writer data_writer);

// Next define your value labels, if any. Create as many named sets as you'd like.
readstat_label_set_t *readstat_add_label_set(readstat_writer_t *writer, readstat_types_t type, const char *name);
void readstat_label_double_value(readstat_label_set_t *label_set, double value, const char *label);
void readstat_label_int32_value(readstat_label_set_t *label_set, int32_t value, const char *label);
void readstat_label_string_value(readstat_label_set_t *label_set, const char *value, const char *label);

// Now define your variables. Note that `width' is only used for READSTAT_TYPE_STRING variables.
readstat_variable_t *readstat_add_variable(readstat_writer_t *writer, readstat_types_t type, size_t width,
        const char *name, const char *label, const char *format, readstat_label_set_t *label_set);
readstat_variable_t *readstat_get_variable(readstat_writer_t *writer, int index);

// Call one of these at any time before the first invocation of readstat_begin_row
readstat_error_t readstat_begin_writing_sav(readstat_writer_t *writer, void *user_ctx,
        const char *file_label, long row_count);

readstat_error_t readstat_begin_writing_dta(readstat_writer_t *writer, void *user_ctx,
        const char *file_label, long row_count);

// Start a row of data (that is, a case or observation)
readstat_error_t readstat_begin_row(readstat_writer_t *writer);

// Then call one of these for each variable
readstat_error_t readstat_insert_char_value(readstat_writer_t *writer, const readstat_variable_t *variable, char value);
readstat_error_t readstat_insert_int16_value(readstat_writer_t *writer, const readstat_variable_t *variable, int16_t value);
readstat_error_t readstat_insert_int32_value(readstat_writer_t *writer, const readstat_variable_t *variable, int32_t value);
readstat_error_t readstat_insert_float_value(readstat_writer_t *writer, const readstat_variable_t *variable, float value);
readstat_error_t readstat_insert_double_value(readstat_writer_t *writer, const readstat_variable_t *variable, double value);
readstat_error_t readstat_insert_string_value(readstat_writer_t *writer, const readstat_variable_t *variable, const char *value);
readstat_error_t readstat_insert_missing_value(readstat_writer_t *writer, const readstat_variable_t *variable);

// Finally, close out the row
readstat_error_t readstat_end_row(readstat_writer_t *writer);

// Once you've written all the rows, clean up after yourself
readstat_error_t readstat_end_writing(readstat_writer_t *writer);
void readstat_writer_free(readstat_writer_t *writer);

typedef int (*rdata_column_handler)(const char *name, readstat_types_t type, char *format, 
        void *data, long count, void *ctx);
typedef int (*rdata_table_handler)(const char *name, void *ctx);
typedef int (*rdata_text_value_handler)(const char *value, int index, void *ctx);
typedef int (*rdata_column_name_handler)(const char *value, int index, void *ctx);

typedef struct rdata_parser_s {
    rdata_table_handler         table_handler;
    rdata_column_handler        column_handler;
    rdata_column_name_handler   column_name_handler;
    rdata_text_value_handler    text_value_handler;
    rdata_text_value_handler    value_label_handler;
    readstat_error_handler      error_handler;
} rdata_parser_t;

rdata_parser_t *rdata_parser_init();
void rdata_parser_free(rdata_parser_t *parser);

readstat_error_t rdata_set_table_handler(rdata_parser_t *parser, rdata_table_handler table_handler);
readstat_error_t rdata_set_column_handler(rdata_parser_t *parser, rdata_column_handler column_handler);
readstat_error_t rdata_set_column_name_handler(rdata_parser_t *parser, rdata_column_name_handler column_name_handler);
readstat_error_t rdata_set_text_value_handler(rdata_parser_t *parser, rdata_text_value_handler text_value_handler);
readstat_error_t rdata_set_value_label_handler(rdata_parser_t *parser, rdata_text_value_handler value_label_handler);
readstat_error_t rdata_set_error_handler(rdata_parser_t *parser, readstat_error_handler error_handler);
/* rdata_parse works on RData and RDS. The table handler will be called once
 * per data frame in RData files, and zero times on RDS files. */
readstat_error_t rdata_parse(rdata_parser_t *parser, const char *filename, void *user_ctx);

#ifdef __cplusplus
}
#endif

#endif
