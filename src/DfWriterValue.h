#ifndef __HAVEN_DFWRITER_VALUE__
#define __HAVEN_DFWRITER_VALUE__

#include "haven_types.h"
#include "readstat.h"
#include "tagged_na.h"

#include <cmath>
#include <string>
#include <unordered_map>

// SAS formats use uppercase tagged NAs
inline char write_tag(char tag, FileExt ext) {
  if (ext == HAVEN_XPT || ext == HAVEN_SAS7BDAT || ext == HAVEN_SAS7BCAT) {
    return toupper(tag);
  }
  return tag;
}

inline readstat_error_t insertValue(
    readstat_writer_t* writer,
    FileExt ext,
    readstat_variable_t* var,
    double val,
    bool is_missing) {
  if (is_missing) {
    char tag = tagged_na_value(val);
    if (tag == '\0') {
      return readstat_insert_missing_value(writer, var);
    } else {
      return readstat_insert_tagged_missing_value(
          writer, var, write_tag(tag, ext));
    }
  }

  switch (var->type) {
  case READSTAT_TYPE_INT8:
    return readstat_insert_int8_value(writer, var, val);
  case READSTAT_TYPE_INT16:
    return readstat_insert_int16_value(writer, var, val);
  case READSTAT_TYPE_INT32:
    return readstat_insert_int32_value(writer, var, val);
  default:
    return readstat_insert_double_value(writer, var, val);
  }
}

inline readstat_error_t insertValue(
    readstat_writer_t* writer,
    std::unordered_map<std::string, readstat_string_ref_t*>& string_ref,
    readstat_variable_t* var,
    const char* val,
    bool is_missing) {
  if (var->type == READSTAT_TYPE_STRING_REF) {
    std::string val_s(is_missing ? "" : val);
    return readstat_insert_string_ref(writer, var, string_ref[val_s]);
  } else if (is_missing) {
    return readstat_insert_missing_value(writer, var);
  } else {
    return readstat_insert_string_value(writer, var, val);
  }
}

#endif
