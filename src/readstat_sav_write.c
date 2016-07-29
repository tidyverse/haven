
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdint.h>
#include <math.h>
#include <float.h>
#include <time.h>

#include "readstat_sav.h"
#include "readstat_spss_parse.h"
#include "readstat_writer.h"

#define MAX_TEXT_SIZE               256
#define MAX_LABEL_SIZE              256

static long readstat_label_set_number_short_variables(readstat_label_set_t *r_label_set) {
    long count = 0;
    int j;
    for (j=0; j<r_label_set->variables_count; j++) {
        readstat_variable_t *r_variable = readstat_get_label_set_variable(r_label_set, j);
        if (r_variable->storage_width <= 8) {
            count++;
        }
    }
    return count;
}

static int readstat_label_set_needs_short_value_labels_record(readstat_label_set_t *r_label_set) {
    return readstat_label_set_number_short_variables(r_label_set) > 0;
}

static int readstat_label_set_needs_long_value_labels_record(readstat_label_set_t *r_label_set) {
    return readstat_label_set_number_short_variables(r_label_set) < r_label_set->variables_count;
}

static int32_t sav_encode_format(spss_format_t *spss_format) {
    return ((spss_format->type << 16) |
            (spss_format->width << 8) |
            spss_format->decimal_places);
}

static readstat_error_t sav_encode_variable_format(int32_t *out_code, 
        readstat_variable_t *r_variable) {
    spss_format_t spss_format;
    readstat_error_t retval = spss_format_for_variable(r_variable, &spss_format);
    if (retval == READSTAT_OK && out_code)
        *out_code = sav_encode_format(&spss_format);

    return retval;
}

static readstat_error_t sav_emit_header(readstat_writer_t *writer) {
    readstat_error_t retval = READSTAT_OK;
    time_t now = writer->timestamp;
    struct tm *time_s = localtime(&now);

    sav_file_header_record_t header;
    memset(&header, 0, sizeof(sav_file_header_record_t));

    memcpy(header.rec_type, "$FL2", sizeof("$FL2")-1);
    memset(header.prod_name, ' ', sizeof(header.prod_name));
    memcpy(header.prod_name,
           "@(#) SPSS DATA FILE - " READSTAT_PRODUCT_URL, 
           sizeof("@(#) SPSS DATA FILE - " READSTAT_PRODUCT_URL)-1);
    header.layout_code = 2;
    header.nominal_case_size = writer->row_len / 8;
    header.compressed = 0; /* TODO */
    if (writer->fweight_variable) {
        int32_t dictionary_index = 1 + writer->fweight_variable->offset / 8;
        header.weight_index = dictionary_index;
    } else {
        header.weight_index = 0;
    }
    header.ncases = writer->row_count;
    header.bias = 100.0;
    
    /* There are portability issues with strftime so hack something up */
    char months[][4] = { 
        "Jan", "Feb", "Mar", "Apr", "May", "Jun",
        "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };

    char creation_date[sizeof(header.creation_date)+1];
    snprintf(creation_date, sizeof(creation_date),
            "%02d %3s %02d", time_s->tm_mday, months[time_s->tm_mon], time_s->tm_year % 100);
    strncpy(header.creation_date, creation_date, sizeof(header.creation_date));

    char creation_time[sizeof(header.creation_time)+1];
    snprintf(creation_time, sizeof(creation_time),
            "%02d:%02d:%02d", time_s->tm_hour, time_s->tm_min, time_s->tm_sec);
    strncpy(header.creation_time, creation_time, sizeof(header.creation_time));
    
    memset(header.file_label, ' ', sizeof(header.file_label));

    size_t file_label_len = strlen(writer->file_label);
    if (file_label_len > sizeof(header.file_label))
        file_label_len = sizeof(header.file_label);

    if (writer->file_label[0])
        memcpy(header.file_label, writer->file_label, file_label_len);
    
    retval = readstat_write_bytes(writer, &header, sizeof(header));
    return retval;
}

static readstat_error_t sav_emit_variable_label(readstat_writer_t *writer, readstat_variable_t *r_variable) {
    readstat_error_t retval = READSTAT_OK;
    const char *title_data = r_variable->label;
    size_t title_data_len = strlen(title_data);
    if (title_data_len > 0) {
        char padded_label[MAX_LABEL_SIZE];
        int32_t label_len = title_data_len;
        if (label_len > sizeof(padded_label))
            label_len = sizeof(padded_label);

        retval = readstat_write_bytes(writer, &label_len, sizeof(label_len));
        if (retval != READSTAT_OK)
            goto cleanup;

        strncpy(padded_label, title_data, (label_len + 3) / 4 * 4);

        retval = readstat_write_bytes(writer, padded_label, (label_len + 3) / 4 * 4);
        if (retval != READSTAT_OK)
            goto cleanup;
    }

cleanup:
    return retval;
}

static int sav_n_missing_values(readstat_variable_t *r_variable) {
    int n_missing_ranges = readstat_variable_get_missing_ranges_count(r_variable);
    int n_missing_values = n_missing_ranges;
    int has_missing_range = 0;
    int j;
    for (j=0; j<n_missing_ranges; j++) {
        readstat_value_t lo = readstat_variable_get_missing_range_lo(r_variable, j);
        readstat_value_t hi = readstat_variable_get_missing_range_hi(r_variable, j);
        if (spss_64bit_value(lo) != spss_64bit_value(hi)) {
            n_missing_values++;
            has_missing_range = 1;
        }
    }
    if (n_missing_values > 3)
        n_missing_values = 3;

    return has_missing_range ? -n_missing_values : n_missing_values;
}

static readstat_error_t sav_emit_variable_missing_values(readstat_writer_t *writer, readstat_variable_t *r_variable) {
    readstat_error_t retval = READSTAT_OK;
    int n_missing_values = 0;
    int n_missing_ranges = readstat_variable_get_missing_ranges_count(r_variable);
    /* ranges */
    int j;

    for (j=0; j<n_missing_ranges; j++) {
        readstat_value_t lo = readstat_variable_get_missing_range_lo(r_variable, j);
        readstat_value_t hi = readstat_variable_get_missing_range_hi(r_variable, j);
        if (spss_64bit_value(lo) != spss_64bit_value(hi)) {
            uint64_t lo_val = spss_64bit_value(lo);
            retval = readstat_write_bytes(writer, &lo_val, sizeof(uint64_t));
            if (retval != READSTAT_OK)
                goto cleanup;

            uint64_t hi_val = spss_64bit_value(hi);
            retval = readstat_write_bytes(writer, &hi_val, sizeof(uint64_t));
            if (retval != READSTAT_OK)
                goto cleanup;

            n_missing_values += 2;

            break;
        }
    }
    /* values */
    for (j=0; j<n_missing_ranges; j++) {
        readstat_value_t lo = readstat_variable_get_missing_range_lo(r_variable, j);
        readstat_value_t hi = readstat_variable_get_missing_range_hi(r_variable, j);
        if (spss_64bit_value(lo) == spss_64bit_value(hi)) {
            uint64_t d_val = spss_64bit_value(lo);
            retval = readstat_write_bytes(writer, &d_val, sizeof(uint64_t));
            if (retval != READSTAT_OK)
                goto cleanup;

            if (++n_missing_values == 3)
                break;
        }
    }
cleanup:
    return retval;
}

static readstat_error_t sav_emit_variable_records(readstat_writer_t *writer) {
    readstat_error_t retval = READSTAT_OK;
    int i, j;
    int32_t rec_type = 0;
    
    for (i=0; i<writer->variables_count; i++) {
        readstat_variable_t *r_variable = readstat_get_variable(writer, i);
        char name_data[9];
        snprintf(name_data, sizeof(name_data), "VAR%d", i);
        size_t name_data_len = strlen(name_data);

        rec_type = SAV_RECORD_TYPE_VARIABLE;
        retval = readstat_write_bytes(writer, &rec_type, sizeof(rec_type));
        if (retval != READSTAT_OK)
            goto cleanup;
        
        sav_variable_record_t variable;
        memset(&variable, 0, sizeof(sav_variable_record_t));

        variable.type = (r_variable->type == READSTAT_TYPE_STRING) ? r_variable->storage_width : 0;
        variable.has_var_label = (r_variable->label[0] != '\0');
        variable.n_missing_values = sav_n_missing_values(r_variable); 

        retval = sav_encode_variable_format(&variable.print, r_variable);
        if (retval != READSTAT_OK)
            goto cleanup;

        variable.write = variable.print;

        memset(variable.name, ' ', sizeof(variable.name));
        if (name_data_len > 0 && name_data_len <= sizeof(variable.name))
            memcpy(variable.name, name_data, name_data_len);
        
        retval = readstat_write_bytes(writer, &variable, sizeof(variable));
        if (retval != READSTAT_OK)
            goto cleanup;
        
        retval = sav_emit_variable_label(writer, r_variable);
        if (retval != READSTAT_OK)
            goto cleanup;

        retval = sav_emit_variable_missing_values(writer, r_variable);
        if (retval != READSTAT_OK)
            goto cleanup;
        
        int extra_fields = r_variable->storage_width / 8 - 1;
        for (j=0; j<extra_fields; j++) {
            retval = readstat_write_bytes(writer, &rec_type, sizeof(rec_type));
            if (retval != READSTAT_OK)
                goto cleanup;

            memset(&variable, '\0', sizeof(variable));
            variable.type = -1;
            retval = readstat_write_bytes(writer, &variable, sizeof(variable));
            if (retval != READSTAT_OK)
                goto cleanup;
        }
    }

cleanup:
    return retval;
}

static readstat_error_t sav_emit_value_label_records(readstat_writer_t *writer) {
    readstat_error_t retval = READSTAT_OK;
    int i, j;
    for (i=0; i<writer->label_sets_count; i++) {
        readstat_label_set_t *r_label_set = readstat_get_label_set(writer, i);
        if (!readstat_label_set_needs_short_value_labels_record(r_label_set))
            continue;

        readstat_type_t user_type = r_label_set->type;
        int32_t label_count = r_label_set->value_labels_count;
        int32_t rec_type = 0;

        if (label_count) {
            rec_type = SAV_RECORD_TYPE_VALUE_LABEL;

            retval = readstat_write_bytes(writer, &rec_type, sizeof(rec_type));
            if (retval != READSTAT_OK)
                goto cleanup;

            retval = readstat_write_bytes(writer, &label_count, sizeof(label_count));
            if (retval != READSTAT_OK)
                goto cleanup;
            
            for (j=0; j<label_count; j++) {
                readstat_value_label_t *r_value_label = readstat_get_value_label(r_label_set, j);
                char value[8];
                if (user_type == READSTAT_TYPE_STRING) {
                    memset(value, ' ', sizeof(value));
                    memcpy(value, r_value_label->string_key, r_value_label->string_key_len);
                } else if (user_type == READSTAT_TYPE_DOUBLE) {
                    double num_val = r_value_label->double_key;
                    memcpy(value, &num_val, sizeof(double));
                } else if (user_type == READSTAT_TYPE_INT32) {
                    double num_val = r_value_label->int32_key;
                    memcpy(value, &num_val, sizeof(double));
                }
                retval = readstat_write_bytes(writer, value, sizeof(value));
                
                const char *label_data = r_value_label->label;
                size_t label_data_len = r_value_label->label_len;
                
                char label_len = label_data_len;
                retval = readstat_write_bytes(writer, &label_len, sizeof(label_len));
                if (retval != READSTAT_OK)
                    goto cleanup;

                char label[255];
                memset(label, ' ', sizeof(label));
                memcpy(label, label_data, label_data_len);
                retval = readstat_write_bytes(writer, label, (label_data_len + 8) / 8 * 8 - 1);
                if (retval != READSTAT_OK)
                    goto cleanup;
            }
            
            rec_type = SAV_RECORD_TYPE_VALUE_LABEL_VARIABLES;
            int32_t var_count = readstat_label_set_number_short_variables(r_label_set);
            
            retval = readstat_write_bytes(writer, &rec_type, sizeof(rec_type));
            if (retval != READSTAT_OK)
                goto cleanup;

            retval = readstat_write_bytes(writer, &var_count, sizeof(var_count));
            if (retval != READSTAT_OK)
                goto cleanup;

            for (j=0; j<r_label_set->variables_count; j++) {
                readstat_variable_t *r_variable = readstat_get_label_set_variable(r_label_set, j);
                if (r_variable->storage_width > 8)
                    continue;

                int32_t dictionary_index = 1 + r_variable->offset / 8;
                retval = readstat_write_bytes(writer, &dictionary_index, sizeof(dictionary_index));
                if (retval != READSTAT_OK)
                    goto cleanup;
            }
        }
    }

cleanup:
    return retval;
}

static readstat_error_t sav_emit_integer_info_record(readstat_writer_t *writer) {
    readstat_error_t retval = READSTAT_OK;

    sav_info_record_t info_header;
    memset(&info_header, 0, sizeof(sav_info_record_t));

    info_header.rec_type = SAV_RECORD_TYPE_HAS_DATA;
    info_header.subtype = SAV_RECORD_SUBTYPE_INTEGER_INFO;
    info_header.size = 4;
    info_header.count = 8;

    retval = readstat_write_bytes(writer, &info_header, sizeof(info_header));
    if (retval != READSTAT_OK)
        goto cleanup;
    
    sav_machine_integer_info_record_t machine_info;
    memset(&machine_info, 0, sizeof(sav_machine_integer_info_record_t));

    machine_info.version_major = 1;
    machine_info.version_minor = 0;
    machine_info.version_revision = 0;
    machine_info.machine_code = -1;
    machine_info.floating_point_rep = SAV_FLOATING_POINT_REP_IEEE;
    machine_info.compression_code = 1;
    machine_info.endianness = machine_is_little_endian() ? SAV_ENDIANNESS_LITTLE : SAV_ENDIANNESS_BIG;
    machine_info.character_code = SAV_CHARSET_UTF8;

    retval = readstat_write_bytes(writer, &machine_info, sizeof(machine_info));

cleanup:
    return retval;
}

static readstat_error_t sav_emit_floating_point_info_record(readstat_writer_t *writer) {
    readstat_error_t retval = READSTAT_OK;

    sav_info_record_t info_header;
    memset(&info_header, 0, sizeof(sav_info_record_t));

    info_header.rec_type = SAV_RECORD_TYPE_HAS_DATA;
    info_header.subtype = SAV_RECORD_SUBTYPE_FP_INFO;
    info_header.size = 8;
    info_header.count = 3;

    retval = readstat_write_bytes(writer, &info_header, sizeof(info_header));
    if (retval != READSTAT_OK)
        goto cleanup;
    
    sav_machine_floating_point_info_record_t fp_info;
    memset(&fp_info, 0, sizeof(sav_machine_floating_point_info_record_t));

    fp_info.sysmis = NAN;
    fp_info.lowest = DBL_MAX;
    fp_info.highest = -DBL_MAX;

    retval = readstat_write_bytes(writer, &fp_info, sizeof(fp_info));
    if (retval != READSTAT_OK)
        goto cleanup;

cleanup:
    return retval;
}

static readstat_error_t sav_emit_variable_display_record(readstat_writer_t *writer) {
    readstat_error_t retval = READSTAT_OK;
    int i;
    sav_info_record_t info_header;
    memset(&info_header, 0, sizeof(sav_info_record_t));

    info_header.rec_type = SAV_RECORD_TYPE_HAS_DATA;
    info_header.subtype = SAV_RECORD_SUBTYPE_VAR_DISPLAY;
    info_header.size = sizeof(int32_t);
    info_header.count = 3 * writer->variables_count;

    retval = readstat_write_bytes(writer, &info_header, sizeof(info_header));
    if (retval != READSTAT_OK)
        goto cleanup;
 
    for (i=0; i<writer->variables_count; i++) {
        readstat_variable_t *r_variable = readstat_get_variable(writer, i);
        readstat_measure_t measure = readstat_variable_get_measure(r_variable);
        int32_t sav_measure = spss_measure_from_readstat_measure(measure);

        retval = readstat_write_bytes(writer, &sav_measure, sizeof(int32_t));
        if (retval != READSTAT_OK)
            goto cleanup;

        int32_t sav_display_width = readstat_variable_get_display_width(r_variable);
        if (sav_display_width <= 0)
            sav_display_width = 8;

        retval = readstat_write_bytes(writer, &sav_display_width, sizeof(int32_t));
        if (retval != READSTAT_OK)
            goto cleanup;

        readstat_alignment_t alignment = readstat_variable_get_alignment(r_variable);
        int32_t sav_alignment = spss_alignment_from_readstat_alignment(alignment);

        retval = readstat_write_bytes(writer, &sav_alignment, sizeof(int32_t));
        if (retval != READSTAT_OK)
            goto cleanup;
    }

cleanup:
    return retval;
}

static readstat_error_t sav_emit_long_var_name_record(readstat_writer_t *writer) {
    readstat_error_t retval = READSTAT_OK;
    int i;
    sav_info_record_t info_header;
    memset(&info_header, 0, sizeof(sav_info_record_t));

    info_header.rec_type = SAV_RECORD_TYPE_HAS_DATA;
    info_header.subtype = SAV_RECORD_SUBTYPE_LONG_VAR_NAME;
    info_header.size = 1;
    info_header.count = 0;
    
    for (i=0; i<writer->variables_count; i++) {
        char name_data[9];
        snprintf(name_data, sizeof(name_data), "VAR%d", i);
        size_t name_data_len = strlen(name_data);
        
        readstat_variable_t *r_variable = readstat_get_variable(writer, i);
        const char *title_data = r_variable->name;
        size_t title_data_len = strlen(title_data);
        if (title_data_len > 0 && name_data_len > 0) {
            if (title_data_len > 64)
                title_data_len = 64;
            
            info_header.count += name_data_len;
            info_header.count += sizeof("=")-1;
            info_header.count += title_data_len;
            info_header.count += sizeof("\x09")-1;
        }
    }
    
    if (info_header.count > 0) {
        info_header.count--; /* no trailing 0x09 */
        
        retval = readstat_write_bytes(writer, &info_header, sizeof(info_header));
        if (retval != READSTAT_OK)
            goto cleanup;
        
        int is_first = 1;
        
        for (i=0; i<writer->variables_count; i++) {
            char name_data[9];
            snprintf(name_data, sizeof(name_data), "VAR%d", i);
            size_t name_data_len = strlen(name_data);

            readstat_variable_t *r_variable = readstat_get_variable(writer, i);
            const char *title_data = r_variable->name;
            size_t title_data_len = strlen(title_data);
            
            char kv_separator = '=';
            char tuple_separator = 0x09;
            
            if (title_data_len > 0) {
                if (title_data_len > 64)
                    title_data_len = 64;

                if (!is_first) {
                    retval = readstat_write_bytes(writer, &tuple_separator, sizeof(tuple_separator));
                    if (retval != READSTAT_OK)
                        goto cleanup;
                }
                
                retval = readstat_write_bytes(writer, name_data, name_data_len);
                if (retval != READSTAT_OK)
                    goto cleanup;

                retval = readstat_write_bytes(writer, &kv_separator, sizeof(kv_separator));
                if (retval != READSTAT_OK)
                    goto cleanup;

                retval = readstat_write_bytes(writer, title_data, title_data_len);
                if (retval != READSTAT_OK)
                    goto cleanup;
                
                is_first = 0;
            }
        }
    }

cleanup:
    return retval;
}

static readstat_error_t sav_emit_long_value_labels_records(readstat_writer_t *writer) {
    readstat_error_t retval = READSTAT_OK;
    int i, j, k;
    char *space_buffer = NULL;

    sav_info_record_t info_header;
    memset(&info_header, 0, sizeof(sav_info_record_t));

    info_header.rec_type = SAV_RECORD_TYPE_HAS_DATA;
    info_header.subtype = SAV_RECORD_SUBTYPE_LONG_VALUE_LABELS;
    info_header.size = 1;
    info_header.count = 0;

    for (i=0; i<writer->label_sets_count; i++) {
        readstat_label_set_t *r_label_set = readstat_get_label_set(writer, i);
        if (!readstat_label_set_needs_long_value_labels_record(r_label_set))
            continue;

        int32_t label_count = r_label_set->value_labels_count;
        int32_t var_count = r_label_set->variables_count;
        
        for (k=0; k<var_count; k++) {
            info_header.count = 0;

            readstat_variable_t *r_variable = readstat_get_label_set_variable(r_label_set, k);
            int32_t name_len = strlen(r_variable->name);
            int32_t storage_width = readstat_variable_get_storage_width(r_variable);
            if (storage_width <= 8)
                continue;

            space_buffer = realloc(space_buffer, storage_width);
            memset(space_buffer, ' ', storage_width);

            info_header.count += sizeof(int32_t); // name length
            info_header.count += name_len;
            info_header.count += sizeof(int32_t); // variable width
            info_header.count += sizeof(int32_t); // label count

            for (j=0; j<label_count; j++) {
                readstat_value_label_t *r_value_label = readstat_get_value_label(r_label_set, j);
                info_header.count += sizeof(int32_t); // value length
                info_header.count += r_value_label->string_key_len;
                info_header.count += sizeof(int32_t); // label length
                info_header.count += r_value_label->label_len;
            }

            retval = readstat_write_bytes(writer, &info_header, sizeof(info_header));
            if (retval != READSTAT_OK)
                goto cleanup;

            retval = readstat_write_bytes(writer, &name_len, sizeof(int32_t));
            if (retval != READSTAT_OK)
                goto cleanup;

            retval = readstat_write_bytes(writer, r_variable->name, name_len);
            if (retval != READSTAT_OK)
                goto cleanup;

            retval = readstat_write_bytes(writer, &storage_width, sizeof(int32_t));
            if (retval != READSTAT_OK)
                goto cleanup;

            retval = readstat_write_bytes(writer, &label_count, sizeof(int32_t));
            if (retval != READSTAT_OK)
                goto cleanup;

            for (j=0; j<label_count; j++) {
                readstat_value_label_t *r_value_label = readstat_get_value_label(r_label_set, j);
                int32_t value_len = r_value_label->string_key_len;
                int32_t label_len = r_value_label->label_len;
                if (label_len > 120)
                    label_len = 120;

                retval = readstat_write_bytes(writer, &storage_width, sizeof(int32_t));
                if (retval != READSTAT_OK)
                    goto cleanup;

                retval = readstat_write_bytes(writer, r_value_label->string_key, value_len);
                if (retval != READSTAT_OK)
                    goto cleanup;

                if (value_len < storage_width) {
                    retval = readstat_write_bytes(writer, space_buffer, storage_width - value_len);
                    if (retval != READSTAT_OK)
                        goto cleanup;
                }

                retval = readstat_write_bytes(writer, &label_len, sizeof(int32_t));
                if (retval != READSTAT_OK)
                    goto cleanup;

                retval = readstat_write_bytes(writer, r_value_label->label, label_len);
                if (retval != READSTAT_OK)
                    goto cleanup;
            }
        }
    }
cleanup:
    if (space_buffer)
        free(space_buffer);

    return retval;
}

static readstat_error_t sav_emit_termination_record(readstat_writer_t *writer) {
    sav_dictionary_termination_record_t termination_record;
    memset(&termination_record, 0, sizeof(sav_dictionary_termination_record_t));

    termination_record.rec_type = SAV_RECORD_TYPE_DICT_TERMINATION;
    
    return readstat_write_bytes(writer, &termination_record, sizeof(termination_record));
}

static readstat_error_t sav_write_int8(void *row, const readstat_variable_t *var, int8_t value) {
    double dval = value;
    memcpy(row, &dval, sizeof(double));
    return READSTAT_OK;
}

static readstat_error_t sav_write_int16(void *row, const readstat_variable_t *var, int16_t value) {
    double dval = value;
    memcpy(row, &dval, sizeof(double));
    return READSTAT_OK;
}

static readstat_error_t sav_write_int32(void *row, const readstat_variable_t *var, int32_t value) {
    double dval = value;
    memcpy(row, &dval, sizeof(double));
    return READSTAT_OK;
}

static readstat_error_t sav_write_float(void *row, const readstat_variable_t *var, float value) {
    double dval = value;
    memcpy(row, &dval, sizeof(double));
    return READSTAT_OK;
}

static readstat_error_t sav_write_double(void *row, const readstat_variable_t *var, double value) {
    double dval = value;
    memcpy(row, &dval, sizeof(double));
    return READSTAT_OK;
}

static readstat_error_t sav_write_string(void *row, const readstat_variable_t *var, const char *value) {
    memset(row, ' ', var->storage_width);
    if (value != NULL && value[0] != '\0') {
        size_t value_len = strlen(value);
        if (value_len > var->storage_width)
            value_len = var->storage_width;
        memcpy(row, value, value_len);
    }
    return READSTAT_OK;
}

static readstat_error_t sav_write_missing_string(void *row, const readstat_variable_t *var) {
    memset(row, ' ', var->storage_width);
    return READSTAT_OK;
}

static readstat_error_t sav_write_missing_number(void *row, const readstat_variable_t *var) {
    uint64_t missing_val = SAV_MISSING_DOUBLE;
    memcpy(row, &missing_val, sizeof(uint64_t));
    return READSTAT_OK;
}

static readstat_error_t sav_write_missing_tagged(void *row, const readstat_variable_t *var, char tag) {
    return READSTAT_ERROR_TAGGED_VALUES_NOT_SUPPORTED;
}

static size_t sav_variable_width(readstat_type_t type, size_t user_width) {
    if (type == READSTAT_TYPE_STRING) {
        if (user_width > 255 || user_width == 0)
            user_width = 255;
        return (user_width + 7) / 8 * 8;
    }
    return 8;
}

static readstat_error_t sav_begin_data(void *writer_ctx) {
    readstat_writer_t *writer = (readstat_writer_t *)writer_ctx;
    readstat_error_t retval = READSTAT_OK;
    if (!writer->initialized)
        return READSTAT_ERROR_WRITER_NOT_INITIALIZED;

    retval = sav_emit_header(writer);
    if (retval != READSTAT_OK)
        goto cleanup;

    retval = sav_emit_variable_records(writer);
    if (retval != READSTAT_OK)
        goto cleanup;

    retval = sav_emit_value_label_records(writer);
    if (retval != READSTAT_OK)
        goto cleanup;

    retval = sav_emit_integer_info_record(writer);
    if (retval != READSTAT_OK)
        goto cleanup;

    retval = sav_emit_floating_point_info_record(writer);
    if (retval != READSTAT_OK)
        goto cleanup;

    retval = sav_emit_variable_display_record(writer);
    if (retval != READSTAT_OK)
        goto cleanup;

    retval = sav_emit_long_var_name_record(writer);
    if (retval != READSTAT_OK)
        goto cleanup;

    retval = sav_emit_long_value_labels_records(writer);
    if (retval != READSTAT_OK)
        goto cleanup;

    retval = sav_emit_termination_record(writer);
    if (retval != READSTAT_OK)
        goto cleanup;

cleanup:
    return retval;
}

readstat_error_t readstat_begin_writing_sav(readstat_writer_t *writer, void *user_ctx, long row_count) {
    writer->row_count = row_count;
    writer->user_ctx = user_ctx;

    writer->callbacks.variable_width = &sav_variable_width;
    writer->callbacks.write_int8 = &sav_write_int8;
    writer->callbacks.write_int16 = &sav_write_int16;
    writer->callbacks.write_int32 = &sav_write_int32;
    writer->callbacks.write_float = &sav_write_float;
    writer->callbacks.write_double = &sav_write_double;
    writer->callbacks.write_string = &sav_write_string;
    writer->callbacks.write_missing_string = &sav_write_missing_string;
    writer->callbacks.write_missing_number = &sav_write_missing_number;
    writer->callbacks.write_missing_tagged = &sav_write_missing_tagged;
    writer->callbacks.begin_data = &sav_begin_data;
    writer->initialized = 1;

    return READSTAT_OK;
}
