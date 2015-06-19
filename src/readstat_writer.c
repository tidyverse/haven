
#include <stdlib.h>
#include "readstat.h"
#include "readstat_writer.h"

#define VARIABLES_INITIAL_CAPACITY    50
#define LABEL_SETS_INITIAL_CAPACITY   50
#define VALUE_LABELS_INITIAL_CAPACITY 10
#define LABEL_SET_VARIABLES_INITIAL_CAPACITY 2

readstat_writer_t *readstat_writer_init() {
    readstat_writer_t *writer = calloc(1, sizeof(readstat_writer_t));

    writer->variables = calloc(VARIABLES_INITIAL_CAPACITY, sizeof(readstat_variable_t *));
    writer->variables_capacity = VARIABLES_INITIAL_CAPACITY;

    writer->label_sets = calloc(LABEL_SETS_INITIAL_CAPACITY, sizeof(readstat_label_set_t *));
    writer->label_sets_capacity = LABEL_SETS_INITIAL_CAPACITY;

    return writer;
}

static void readstat_variable_free(readstat_variable_t *variable) {
    free(variable);
}

static void readstat_label_set_free(readstat_label_set_t *label_set) {
    free(label_set->value_labels);
    free(label_set->variables);
    free(label_set);
}

void readstat_writer_free(readstat_writer_t *writer) {
    int i;
    if (writer) {
        if (writer->variables) {
            for (i=0; i<writer->variables_count; i++) {
                readstat_variable_free(writer->variables[i]);
            }
            free(writer->variables);
        }
        if (writer->label_sets) {
            for (i=0; i<writer->label_sets_count; i++) {
                readstat_label_set_free(writer->label_sets[i]);
            }
            free(writer->label_sets);
        }
        if (writer->row) {
            free(writer->row);
        }
        free(writer);
    }
}

readstat_error_t readstat_set_data_writer(readstat_writer_t *writer, readstat_data_writer data_writer) {
    writer->data_writer = data_writer;
    return READSTAT_OK;
}

readstat_error_t readstat_write_bytes(readstat_writer_t *writer, const void *bytes, size_t len) {
    size_t bytes_written = writer->data_writer(bytes, len, writer->user_ctx);
    if (bytes_written < len) {
        return READSTAT_ERROR_WRITE;
    }
    return READSTAT_OK;
}

readstat_label_set_t *readstat_add_label_set(readstat_writer_t *writer, readstat_types_t type, const char *name) {
    if (writer->label_sets_count == writer->label_sets_capacity) {
        writer->label_sets_capacity *= 2;
        writer->label_sets = realloc(writer->label_sets, 
                writer->label_sets_capacity * sizeof(readstat_label_set_t *));
    }
    readstat_label_set_t *new_label_set = calloc(1, sizeof(readstat_label_set_t));
    
    writer->label_sets[writer->label_sets_count++] = new_label_set;

    new_label_set->type = type;
    strncpy(new_label_set->name, name, sizeof(new_label_set->name));

    new_label_set->value_labels = calloc(VALUE_LABELS_INITIAL_CAPACITY, sizeof(readstat_value_label_t));
    new_label_set->value_labels_capacity = VALUE_LABELS_INITIAL_CAPACITY;

    new_label_set->variables = calloc(LABEL_SET_VARIABLES_INITIAL_CAPACITY, sizeof(readstat_variable_t *));
    new_label_set->variables_capacity = LABEL_SET_VARIABLES_INITIAL_CAPACITY;

    return new_label_set;
}

readstat_label_set_t *readstat_get_label_set(readstat_writer_t *writer, int index) {
    if (index < writer->label_sets_count) {
        return writer->label_sets[index];
    }
    return NULL;
}

readstat_value_label_t *readstat_get_value_label(readstat_label_set_t *label_set, int index) {
    if (index < label_set->value_labels_count) {
        return &label_set->value_labels[index];
    }
    return NULL;
}

readstat_variable_t *readstat_get_label_set_variable(readstat_label_set_t *label_set, int index) {
    if (index < label_set->variables_count) {
        return ((readstat_variable_t **)label_set->variables)[index];
    }
    return NULL;
}

void readstat_label_double_value(readstat_label_set_t *label_set, double value, const char *label) {
    if (label_set->value_labels_count == label_set->value_labels_capacity) {
        label_set->value_labels_capacity *= 2;
        label_set->value_labels = realloc(label_set->value_labels, 
                label_set->value_labels_capacity * sizeof(readstat_value_label_t));
    }
    readstat_value_label_t *new_value_label = &label_set->value_labels[label_set->value_labels_count++];
    new_value_label->double_key = value;
    strncpy(new_value_label->label, label, sizeof(new_value_label->label));
}

void readstat_label_int32_value(readstat_label_set_t *label_set, int32_t value, const char *label) {
    if (label_set->value_labels_count == label_set->value_labels_capacity) {
        label_set->value_labels_capacity *= 2;
        label_set->value_labels = realloc(label_set->value_labels, 
                label_set->value_labels_capacity * sizeof(readstat_value_label_t));
    }
    readstat_value_label_t *new_value_label = &label_set->value_labels[label_set->value_labels_count++];
    new_value_label->int32_key = value;
    strncpy(new_value_label->label, label, sizeof(new_value_label->label));
}

void readstat_label_string_value(readstat_label_set_t *label_set, const char *value, const char *label) {
    if (label_set->value_labels_count == label_set->value_labels_capacity) {
        label_set->value_labels_capacity *= 2;
        label_set->value_labels = realloc(label_set->value_labels, 
                label_set->value_labels_capacity * sizeof(readstat_value_label_t));
    }
    readstat_value_label_t *new_value_label = &label_set->value_labels[label_set->value_labels_count++];
    strncpy(new_value_label->string_key, value, sizeof(new_value_label->string_key));
    strncpy(new_value_label->label, label, sizeof(new_value_label->label));
}

readstat_variable_t *readstat_add_variable(readstat_writer_t *writer, readstat_types_t type, size_t width,
        const char *name, const char *label, const char *format, readstat_label_set_t *label_set) {
    if (writer->variables_count == writer->variables_capacity) {
        writer->variables_capacity *= 2;
        writer->variables = realloc(writer->variables,
                writer->variables_capacity * sizeof(readstat_variable_t *));
    }
    readstat_variable_t *new_variable = calloc(1, sizeof(readstat_variable_t));

    new_variable->index = writer->variables_count++;
    
    writer->variables[new_variable->index] = new_variable;

    new_variable->user_width = width;
    new_variable->type = type;

    if (name) {
        snprintf(new_variable->name, sizeof(new_variable->name), "%s", name);
    }

    if (label) {
        snprintf(new_variable->label, sizeof(new_variable->label), "%s", label);
    }

    if (format) {
        snprintf(new_variable->format, sizeof(new_variable->format), "%s", format);
    }

    if (label_set) {
        if (label_set->variables_count == label_set->variables_capacity) {
            label_set->variables_capacity *= 2;
            label_set->variables = realloc(label_set->variables,
                    label_set->variables_capacity * sizeof(readstat_variable_t *));
        }
        new_variable->label_set = label_set;
        ((readstat_variable_t **)label_set->variables)[label_set->variables_count++] = new_variable;
    }
    return new_variable;
}

readstat_variable_t *readstat_get_variable(readstat_writer_t *writer, int index) {
    if (index < writer->variables_count) {
        return writer->variables[index];
    }
    return NULL;
}

void readstat_writer_set_file_label(readstat_writer_t *writer, const char *file_label) {
    snprintf(writer->file_label, sizeof(writer->file_label), "%s", file_label);
}

void readstat_writer_set_fweight_variable(readstat_writer_t *writer, const readstat_variable_t *variable) {
    writer->fweight_variable = variable;
}

readstat_error_t readstat_begin_row(readstat_writer_t *writer) {
    readstat_error_t retval = READSTAT_OK;
    if (writer->current_row == 0) {
        size_t row_len = 0;
        int i;
        for (i=0; i<writer->variables_count; i++) {
            readstat_variable_t *variable = readstat_get_variable(writer, i);
            variable->width = writer->callbacks.variable_width(variable->type, variable->user_width);
            variable->offset = row_len;
            row_len += variable->width;
        }
        if (writer->callbacks.begin_data) {
            retval = writer->callbacks.begin_data(writer);
        }
        writer->row = malloc(row_len);
        writer->row_len = row_len;
    }
    return retval;
}

// Then call one of these for each variable
readstat_error_t readstat_insert_char_value(readstat_writer_t *writer, const readstat_variable_t *variable, char value) {
    return writer->callbacks.write_char(&writer->row[variable->offset], variable, value);
}

readstat_error_t readstat_insert_int16_value(readstat_writer_t *writer, const readstat_variable_t *variable, int16_t value) {
    return writer->callbacks.write_int16(&writer->row[variable->offset], variable, value);
}

readstat_error_t readstat_insert_int32_value(readstat_writer_t *writer, const readstat_variable_t *variable, int32_t value) {
    return writer->callbacks.write_int32(&writer->row[variable->offset], variable, value);
}

readstat_error_t readstat_insert_float_value(readstat_writer_t *writer, const readstat_variable_t *variable, float value) {
    return writer->callbacks.write_float(&writer->row[variable->offset], variable, value);
}

readstat_error_t readstat_insert_double_value(readstat_writer_t *writer, const readstat_variable_t *variable, double value) {
    return writer->callbacks.write_double(&writer->row[variable->offset], variable, value);
}

readstat_error_t readstat_insert_string_value(readstat_writer_t *writer, const readstat_variable_t *variable, const char *value) {
    return writer->callbacks.write_string(&writer->row[variable->offset], variable, value);
}

readstat_error_t readstat_insert_missing_value(readstat_writer_t *writer, const readstat_variable_t *variable) {
    return writer->callbacks.write_missing(&writer->row[variable->offset], variable);
}

readstat_error_t readstat_end_row(readstat_writer_t *writer) {
    writer->current_row++;
    return readstat_write_bytes(writer, writer->row, writer->row_len);
}

readstat_error_t readstat_end_writing(readstat_writer_t *writer) {
    readstat_error_t retval = READSTAT_OK;
    if (writer->current_row != writer->row_count) {
        retval = READSTAT_ERROR_ROW_COUNT_MISMATCH;
    } else if (writer->callbacks.end_data) {
        retval = writer->callbacks.end_data(writer);
    }
    return retval;
}
