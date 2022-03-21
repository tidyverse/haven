#include "readstat.h"
#include "haven_types.h"
#include "tagged_na.h"

#include <unordered_map>
#include "cpp11/doubles.hpp"
#include "cpp11/strings.hpp"
#include "cpp11/integers.hpp"
#include "cpp11/sexp.hpp"
#include "cpp11/list.hpp"

ssize_t data_writer(const void *data, size_t len, void *ctx);

inline const char* string_utf8(SEXP x, int i) {
  return Rf_translateCharUTF8(STRING_ELT(x, i));
}

inline const bool string_is_missing(SEXP x, int i) {
  return STRING_ELT(x, i) == NA_STRING;
}


inline readstat_measure_e measureType(SEXP x) {
  if (Rf_inherits(x, "ordered")) {
    return READSTAT_MEASURE_ORDINAL;
  } else if (Rf_inherits(x, "factor")) {
    return READSTAT_MEASURE_NOMINAL;
  } else {
    switch(TYPEOF(x)) {
    case INTSXP:
    case REALSXP:
      return READSTAT_MEASURE_SCALE;
    case LGLSXP:
    case STRSXP:
      return READSTAT_MEASURE_NOMINAL;
    default:
      return READSTAT_MEASURE_UNKNOWN;
    }
  }
}

inline int displayWidth(cpp11::sexp x) {
  cpp11::sexp display_width_obj(x.attr("display_width"));
  switch(TYPEOF(display_width_obj)) {
  case INTSXP:
    return INTEGER(display_width_obj)[0];
  case REALSXP:
    return REAL(display_width_obj)[0];
  }
  return 0;
}

inline int userWidth(cpp11::sexp x) {
  cpp11::sexp user_width_obj(x.attr("width"));
  switch(TYPEOF(user_width_obj)) {
  case INTSXP:
    return INTEGER(user_width_obj)[0];
  case REALSXP:
    return REAL(user_width_obj)[0];
  }
  return 0;
}

class Writer {
  FileExt ext_;
  FileVendor vendor_;
  int version_;
  int strl_threshold_;
  std::unordered_map<const char*, readstat_string_ref_t*> string_ref_;

  cpp11::list x_;
  readstat_writer_t* writer_;
  FILE* pOut_;

public:
  Writer(FileExt ext, cpp11::list x, cpp11::strings pathEnc): ext_(ext), vendor_(extVendor(ext)), version_(0), x_(x) {
    std::string path(Rf_translateChar(pathEnc[0]));

    pOut_ = fopen(path.c_str(), "wb");
    if (pOut_ == NULL)
      cpp11::stop("Failed to open '%s' for writing", path.c_str());

    writer_ = readstat_writer_init();
    checkStatus(readstat_set_data_writer(writer_, data_writer));
  }

  ~Writer() {
    try {
      fclose(pOut_);
      readstat_writer_free(writer_);
    } catch (...) {};
  }

  void setCompression(readstat_compress_t version) {
    readstat_writer_set_compression(writer_, version);
  }

  void setVersion(int version) {
    version_ = version;
    readstat_writer_set_file_format_version(writer_, version);
  }

  void setName(const std::string& name) {
    readstat_writer_set_table_name(writer_, name.c_str());
  }

  void setFileLabel(cpp11::sexp label) {
    if (label == R_NilValue)
      return;

    readstat_writer_set_file_label(writer_, string_utf8(label, 0));
  }

  void setStrLThreshold(int strl_threshold) {
    strl_threshold_ = strl_threshold;
  }

  void write() {
    int p = x_.size();
    if (p == 0)
      return;

    int n = Rf_length(x_[0]);

    readstat_error_t status;
    switch(ext_) {
    case HAVEN_SAV:
      status = readstat_begin_writing_sav(writer_, this, n);
      break;
    case HAVEN_DTA:
      status = readstat_begin_writing_dta(writer_, this, n);
      break;
    case HAVEN_SAS7BDAT:
      status = readstat_begin_writing_sas7bdat(writer_, this, n);
      break;
    case HAVEN_XPT:
      status = readstat_begin_writing_xport(writer_, this, n);
      break;
    case HAVEN_POR:
    case HAVEN_SAS7BCAT:
      status = READSTAT_OK;
      // not used
      break;
    }
    if (status) {
      cpp11::stop("Failed to create file: %s.", readstat_error_message(status));
    }

    status = readstat_validate_metadata(writer_);
    if (status) {
      cpp11::stop("Failed to write metadata: %s.", readstat_error_message(status));
    }

    cpp11::strings names(x_.attr("names"));

    // Define variables
    for (int j = 0; j < p; ++j) {
      cpp11::sexp col = x_[j];
      VarType type = numType(col);

      const char* name = string_utf8(names, j);
      const char* format = var_format(col, type);

      switch(TYPEOF(col)) {
        case LGLSXP:  status = defineVariable(cpp11::integers(cpp11::safe[Rf_coerceVector](col, INTSXP)), name, format); break;
        case INTSXP:  status = defineVariable(cpp11::integers(col), name, format); break;
        case REALSXP: status = defineVariable(cpp11::doubles(col), name, format);  break;
        case STRSXP:  status = defineVariable(cpp11::strings(col), name, format); break;
        default:      cpp11::stop("Columns of type %s not supported yet", Rf_type2char(TYPEOF(col)));
      }
      if (status) {
        cpp11::stop("Failed to create column `%s`: %s.", name, readstat_error_message(status));
      }
    }

    // Write data
    for (int i = 0; i < n; ++i) {
      checkStatus(readstat_begin_row(writer_));
      for (int j = 0; j < p; ++j) {
        cpp11::sexp col(x_[j]);
        readstat_variable_t* var = readstat_get_variable(writer_, j);

        switch (TYPEOF(col)) {
        case LGLSXP: {
          int val = LOGICAL(col)[i];
          status = insertValue(var, val, val == NA_LOGICAL);
          break;
        }
        case INTSXP: {
          int val = INTEGER(col)[i];
          status = insertValue(var, (int) adjustDatetimeFromR(vendor_, col, val), val == NA_INTEGER);
          break;
        }
        case REALSXP: {
          double val = REAL(col)[i];
          status = insertValue(var, adjustDatetimeFromR(vendor_, col, val), !R_finite(val));
          break;
        }
        case STRSXP: {
          status = insertValue(var, string_utf8(col, i), string_is_missing(col, i));
          break;
        }
        default:
          status = READSTAT_OK;
          break;
        }

        if (status) {
          cpp11::stop("Failed to insert value [%i, %i]: %s.", i + 1, j + 1, readstat_error_message(status));
        }
      }

      checkStatus(readstat_end_row(writer_));
    }

    checkStatus(readstat_end_writing(writer_));
  }

  // Define variables ----------------------------------------------------------

  const char* var_label(cpp11::sexp x) {
    cpp11::sexp label(x.attr("label"));

    if (label == R_NilValue)
      return NULL;

    return string_utf8(label, 0);
  }

  const char* var_format(cpp11::sexp x, VarType varType) {
    // Use attribute, if present
    cpp11::sexp format(x.attr(formatAttribute(vendor_).c_str()));
    if (format != R_NilValue)
      return string_utf8(format, 0);

    switch(varType) {
    case HAVEN_DEFAULT:
      return NULL;

    case HAVEN_DATETIME:
      switch(vendor_) {
      case HAVEN_SAS:   return "DATETIME";
      case HAVEN_SPSS:  return "DATETIME";
      case HAVEN_STATA: return "%tc";
      }
    case HAVEN_DATE:
      switch(vendor_) {
      case HAVEN_SAS:   return "DATE";
      case HAVEN_SPSS:  return "DATE";
      case HAVEN_STATA: return "%td";
      }
    case HAVEN_TIME:
      switch(vendor_) {
      case HAVEN_SAS:   return "TIME";
      case HAVEN_SPSS:  return "TIME";
      case HAVEN_STATA: return NULL; // Stata doesn't have a pure time type
      }
    }

    return NULL;
  }

  readstat_error_t defineVariable(cpp11::integers x, const char* name, const char* format = NULL) {
    readstat_label_set_t* labelSet = NULL;
    if (Rf_inherits(x, "factor")) {
      labelSet = readstat_add_label_set(writer_, READSTAT_TYPE_INT32, name);

      cpp11::strings levels(x.attr("levels"));
      for (int i = 0; i < levels.size(); ++i)
        readstat_label_int32_value(labelSet, i + 1, string_utf8(levels, i));
    } else if (Rf_inherits(x, "haven_labelled") && TYPEOF(x.attr("labels")) != NILSXP) {
      labelSet = readstat_add_label_set(writer_, READSTAT_TYPE_INT32, name);

      cpp11::integers values(x.attr("labels"));
      cpp11::strings labels(values.attr("names"));

      for (int i = 0; i < values.size(); ++i)
        readstat_label_int32_value(labelSet, values[i], string_utf8(labels, i));
    }

    readstat_variable_t* var =
      readstat_add_variable(writer_, name, READSTAT_TYPE_INT32, userWidth(x));
    readstat_variable_set_format(var, format);
    readstat_variable_set_label(var, var_label(x));
    readstat_variable_set_label_set(var, labelSet);
    readstat_variable_set_measure(var, measureType(x));
    readstat_variable_set_display_width(var, displayWidth(x));

    if (Rf_inherits(x, "haven_labelled_spss")) {
      SEXP na_range = x.attr("na_range");
      if (TYPEOF(na_range) == REALSXP && Rf_length(na_range) == 2) {
        readstat_variable_add_missing_double_range(var, REAL(na_range)[0], REAL(na_range)[1]);
      } else if (TYPEOF(na_range) == INTSXP && Rf_length(na_range) == 2) {
        readstat_variable_add_missing_double_range(var, INTEGER(na_range)[0], INTEGER(na_range)[1]);
      }

      SEXP na_values = x.attr("na_values");
      if (TYPEOF(na_values) == INTSXP) {
        int n = Rf_length(na_values);
        for (int i = 0; i < n; ++i) {
          readstat_variable_add_missing_double_value(var, INTEGER(na_values)[i]);
        }
      }
    }
    return readstat_validate_variable(writer_, var);
  }

  readstat_error_t defineVariable(cpp11::doubles x, const char* name, const char* format = NULL) {
    readstat_label_set_t* labelSet = NULL;
    if (Rf_inherits(x, "haven_labelled") && TYPEOF(x.attr("labels")) != NILSXP) {
      labelSet = readstat_add_label_set(writer_, READSTAT_TYPE_DOUBLE, name);

      cpp11::doubles values(x.attr("labels"));
      cpp11::strings labels(values.attr("names"));

      for (int i = 0; i < values.size(); ++i) {
        char tag = tagged_na_value(values[i]);
        if (!std::isnan(values[i]) || tag == '\0') {
          readstat_label_double_value(labelSet, values[i], string_utf8(labels, i));
        } else {
          readstat_label_tagged_value(labelSet, tag, string_utf8(labels, i));
        }
      }
    }

    readstat_variable_t* var =
      readstat_add_variable(writer_, name, READSTAT_TYPE_DOUBLE, userWidth(x));

    readstat_variable_set_format(var, format);
    readstat_variable_set_label(var, var_label(x));
    readstat_variable_set_label_set(var, labelSet);
    readstat_variable_set_measure(var, measureType(x));
    readstat_variable_set_display_width(var, displayWidth(x));

    if (Rf_inherits(x, "haven_labelled_spss")) {
      SEXP na_range = x.attr("na_range");
      if (TYPEOF(na_range) == REALSXP && Rf_length(na_range) == 2) {
        readstat_variable_add_missing_double_range(var, REAL(na_range)[0], REAL(na_range)[1]);
      } else if (TYPEOF(na_range) == INTSXP && Rf_length(na_range) == 2) {
        readstat_variable_add_missing_double_range(var, INTEGER(na_range)[0], INTEGER(na_range)[1]);
      }

      SEXP na_values = x.attr("na_values");
      if (TYPEOF(na_values) == REALSXP) {
        int n = Rf_length(na_values);
        for (int i = 0; i < n; ++i) {
          readstat_variable_add_missing_double_value(var, REAL(na_values)[i]);
        }
      }
    }
    return readstat_validate_variable(writer_, var);
  }

  readstat_error_t defineVariable(cpp11::strings x, const char* name, const char* format = NULL) {
    readstat_label_set_t* labelSet = NULL;
    if (Rf_inherits(x, "haven_labelled") && TYPEOF(x.attr("labels")) != NILSXP) {
      labelSet = readstat_add_label_set(writer_, READSTAT_TYPE_STRING, name);

      cpp11::strings values(x.attr("labels"));
      cpp11::strings labels(values.attr("names"));

      for (int i = 0; i < values.size(); ++i)
        readstat_label_string_value(labelSet, string_utf8(values, i), string_utf8(labels, i));
    }

    int user_width = userWidth(x);
    int max_length = 1;
    for (int i = 0; i < x.size(); ++i) {
      int length = strlen(string_utf8(x, i));
      if (length > max_length)
        max_length = length;
    }
    if (max_length > user_width) {
      if (user_width > 0) {
        cpp11::warning("Column `%s` contains string values longer than user width %d. Width set to %d to accommodate.", name, user_width, max_length);
      }
      user_width = max_length;
    }


    // Use strL for "long" strings in stata. strL has an 80 byte overhead so
    // we use it when it's likely to be more efficient. The main downside of
    // strL is that it can't be used as a join key but this seems unlikely for
    // very long strings.
    readstat_variable_t* var;
    if (ext_ == HAVEN_DTA && version_ >= 117 && user_width > strl_threshold_) {
      var = readstat_add_variable(writer_, name, READSTAT_TYPE_STRING_REF, user_width);
      for (int i = 0; i < x.size(); ++i) {
        const char* val = string_utf8(x, i);
        if (!string_ref_.count(val)) {
          string_ref_[val] = readstat_add_string_ref(writer_, val);
        }
      }
    } else {
      var = readstat_add_variable(writer_, name, READSTAT_TYPE_STRING, user_width);
    }

    readstat_variable_set_format(var, format);
    readstat_variable_set_label(var, var_label(x));
    readstat_variable_set_label_set(var, labelSet);
    readstat_variable_set_measure(var, measureType(x));
    readstat_variable_set_display_width(var, displayWidth(x));

    if (Rf_inherits(x, "haven_labelled_spss")) {
      SEXP na_range = x.attr("na_range");
      if (Rf_length(na_range) == 2) {
        if (TYPEOF(na_range) == STRSXP) {
          readstat_variable_add_missing_string_range(var,
            R_CHAR(STRING_ELT(na_range, 0)),
            R_CHAR(STRING_ELT(na_range, 1))
          );
        }
      }

      SEXP na_values = x.attr("na_values");
      int n = Rf_length(na_values);
      if (TYPEOF(na_values) == STRSXP) {
        for (int i = 0; i < n; ++i) {
          readstat_variable_add_missing_string_value(var, R_CHAR(STRING_ELT(na_values, i)));
        }
      }
    }

    return readstat_validate_variable(writer_, var);
  }

  // Value helper -------------------------------------------------------------

  readstat_error_t insertValue(readstat_variable_t* var, int val, bool is_missing) {
    if (is_missing) {
      return readstat_insert_missing_value(writer_, var);
    } else {
      return readstat_insert_int32_value(writer_, var, val);
    }
  }

  readstat_error_t insertValue(readstat_variable_t* var, double val, bool is_missing) {
    if (is_missing) {
      char tag = tagged_na_value(val);
      if (tag == '\0') {
        return readstat_insert_missing_value(writer_, var);
      } else {
        return readstat_insert_tagged_missing_value(writer_, var, tag);
      }
    } else {
      return readstat_insert_double_value(writer_, var, val);
    }
  }

  readstat_error_t insertValue(readstat_variable_t* var, const char* val, bool is_missing) {
    if (is_missing) {
      return readstat_insert_missing_value(writer_, var);
    } else if (var->type == READSTAT_TYPE_STRING_REF) {
      return readstat_insert_string_ref(writer_, var, string_ref_[val]);
    } else {
      return readstat_insert_string_value(writer_, var, val);
    }
  }

  // Misc ----------------------------------------------------------------------

  void checkStatus(readstat_error_t err) {
    if (err == 0) return;

    cpp11::stop("Writing failure: %s.", readstat_error_message(err));
  }

  ssize_t write(const void *data, size_t len) {
    return fwrite(data, sizeof(char), len, pOut_);
  }
};

ssize_t data_writer(const void *data, size_t len, void *ctx) {
  return ((Writer*) ctx)->write(data, len);
}

[[cpp11::register]]
void write_sav_(cpp11::list data, cpp11::strings path, std::string compress) {
  Writer writer(HAVEN_SAV, data, path);
  if (compress == "zsav")
    writer.setCompression(READSTAT_COMPRESS_BINARY);
  else if (compress == "none")
    writer.setCompression(READSTAT_COMPRESS_NONE);
  else
    writer.setCompression(READSTAT_COMPRESS_ROWS);
  writer.write();
}

[[cpp11::register]]
void write_dta_(cpp11::list data, cpp11::strings path, int version, cpp11::sexp label, int strl_threshold) {
  Writer writer(HAVEN_DTA, data, path);
  writer.setVersion(version);
  writer.setFileLabel(label);
  writer.setStrLThreshold(strl_threshold);
  writer.write();
}

[[cpp11::register]]
void write_sas_(cpp11::list data, cpp11::strings path) {
  Writer(HAVEN_SAS7BDAT, data, path).write();
}

[[cpp11::register]]
void write_xpt_(cpp11::list data, cpp11::strings path, int version, std::string name, cpp11::sexp label) {
  Writer writer(HAVEN_XPT, data, path);
  writer.setVersion(version);
  writer.setName(name);
  writer.setFileLabel(label);
  writer.write();
}
