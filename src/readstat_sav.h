//
//  Header.h
//  Wizard
//
//  Created by Evan Miller on 3/31/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#include "readstat.h"
#include "readstat_bits.h"

#pragma pack(push, 1)

// SAV files

typedef struct sav_file_header_record_s {
    char     rec_type[4];
    char     prod_name[60];
    int32_t  layout_code;
    int32_t  nominal_case_size;
    int32_t  compressed;
    int32_t  weight_index;
    int32_t  ncases;
    double   bias; /* TODO is this portable? */
    char     creation_date[9];
    char     creation_time[8];
    char     file_label[64];
    char     padding[3];
} sav_file_header_record_t;

typedef struct sav_variable_record_s {
    int32_t  type;
    int32_t  has_var_label;
    int32_t  n_missing_values;
    int32_t  print;
    int32_t  write;
    char     name[8];
} sav_variable_record_t;

typedef struct sav_info_record_header_s {
    int32_t  rec_type;
    int32_t  subtype;
    int32_t  size;
    int32_t  count;
} sav_info_record_t;

typedef struct sav_machine_integer_info_record_s {
    int32_t  version_major;
    int32_t  version_minor;
    int32_t  version_revision;
    int32_t  machine_code;
    int32_t  floating_point_rep;
    int32_t  compression_code;
    int32_t  endianness;
    int32_t  character_code;
} sav_machine_integer_info_record_t;

typedef struct sav_machine_floating_point_info_record_s {
    double   sysmis;
    double   highest;
    double   lowest;
} sav_machine_floating_point_info_record_t;

typedef struct sav_dictionary_termination_record_s {
    int32_t  rec_type;
    int32_t  filler;
} sav_dictionary_termination_record_t;

#pragma pack(pop)

typedef struct sav_format_s {
    int    type;
    int    width;
    int    decimal_places;
} sav_format_t;

typedef struct sav_varinfo_s {
    readstat_types_t type;
    int              labels_index;
    int              index;
    int              offset;
    int              width;
    int              string_length;
    sav_format_t     print_format;
    sav_format_t     write_format;
    int              n_segments;
    int              n_missing_values;
    int              missing_range;
    double           missing_values[3];
    char             name[8*4+1];
    char             longname[64*4+1];
    char            *label;
} sav_varinfo_t;

typedef struct sav_ctx_s {
    sav_varinfo_t *varinfo;
    size_t         varinfo_capacity;
    iconv_t        converter;
    int            var_index;
    int            var_offset;
    int            var_count;
    int            record_count;
    int            value_labels_count;
    int            data_is_compressed:1;
    int            machine_needs_byte_swap:1;
} sav_ctx_t;

#define SAV_RECORD_TYPE_VARIABLE         2
#define SAV_RECORD_TYPE_VALUE_LABEL      3
#define SAV_RECORD_TYPE_DOCUMENT         6
#define SAV_RECORD_TYPE_HAS_DATA         7
#define SAV_RECORD_TYPE_DICT_TERMINATION 999

#define SAV_RECORD_SUBTYPE_INTEGER_INFO  3
#define SAV_RECORD_SUBTYPE_FP_INFO       4
#define SAV_RECORD_SUBTYPE_VAR_DISPLAY   11
#define SAV_RECORD_SUBTYPE_LONG_VAR_NAME 13
#define SAV_RECORD_SUBTYPE_VERY_LONG_STR 14

#define SAV_FLOATING_POINT_REP_IEEE      1
#define SAV_FLOATING_POINT_REP_IBM       2
#define SAV_FLOATING_POINT_REP_VAX       3

#define SAV_ENDIANNESS_BIG               1
#define SAV_ENDIANNESS_LITTLE            2

#define SAV_HIGHEST_DOUBLE   0x7FEFFFFFFFFFFFFFUL
#define SAV_MISSING_DOUBLE   0xFFEFFFFFFFFFFFFFUL
#define SAV_LOWEST_DOUBLE    0xFFEFFFFFFFFFFFFEUL
