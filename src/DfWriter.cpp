#include <Rcpp.h>

#include "readstat.h"
#include "haven_types.h"

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

inline int displayWidth(Rcpp::RObject x) {
  Rcpp::RObject display_width_obj = x.attr("display_width");
  switch(TYPEOF(display_width_obj)) {
  case INTSXP:
    return INTEGER(display_width_obj)[0];
  case REALSXP:
    return REAL(display_width_obj)[0];
  }
  return 0;
}


class Writer {
  FileExt ext_;
  FileVendor vendor_;

  Rcpp::List x_;
  readstat_writer_t* writer_;
  FILE* pOut_;

public:
  Writer(FileExt ext, Rcpp::List x, Rcpp::CharacterVector pathEnc): ext_(ext), vendor_(extVendor(ext)), x_(x) {
    std::string path(Rf_translateChar(pathEnc[0]));

    pOut_ = fopen(path.c_str(), "wb");
    if (pOut_ == NULL)
      Rcpp::stop("Failed to open '%s' for writing", path);

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
    readstat_writer_set_file_format_version(writer_, version);
  }

  void setName(const std::string& name) {
    readstat_writer_set_table_name(writer_, name.c_str());
  }

  void setFileLabel(Rcpp::RObject label) {
    if (label == R_NilValue)
      return;

    readstat_writer_set_file_label(writer_, string_utf8(label, 0));
  }

  void write() {
    int p = x_.size();
    if (p == 0)
      return;

    Rcpp::CharacterVector names(Rcpp::as<Rcpp::CharacterVector>(x_.attr("names")));

    // Define variables
    for (int j = 0; j < p; ++j) {
      Rcpp::RObject col = x_[j];
      VarType type = numType(col);

      const char* name = string_utf8(names, j);
      const char* format = var_format(col, type);

      switch(TYPEOF(col)) {
        case LGLSXP:  defineVariable(Rcpp::as<IntegerVector>(col), name, format); break;
        case INTSXP:  defineVariable(Rcpp::as<IntegerVector>(col), name, format); break;
        case REALSXP: defineVariable(Rcpp::as<NumericVector>(col), name, format);  break;
        case STRSXP:  defineVariable(Rcpp::as<Rcpp::CharacterVector>(col), name, format); break;
      default:
                      Rcpp::stop("Variables of type %s not supported yet",
          Rf_type2char(TYPEOF(col)));
      }
    }

    int n = Rf_length(x_[0]);

    switch(ext_) {
    case HAVEN_SAV:
      checkStatus(readstat_begin_writing_sav(writer_, this, n));
      break;
    case HAVEN_DTA:
      checkStatus(readstat_begin_writing_dta(writer_, this, n));
      break;
    case HAVEN_SAS7BDAT:
      checkStatus(readstat_begin_writing_sas7bdat(writer_, this, n));
      break;
    case HAVEN_XPT:
      checkStatus(readstat_begin_writing_xport(writer_, this, n));
      break;
    }

    // Write data
    for (int i = 0; i < n; ++i) {
      checkStatus(readstat_begin_row(writer_));
      for (int j = 0; j < p; ++j) {
        Rcpp::RObject col = x_[j];
        readstat_variable_t* var = readstat_get_variable(writer_, j);

        switch (TYPEOF(col)) {
        case LGLSXP: {
          int val = LOGICAL(col)[i];
          insertValue(var, val, val == NA_LOGICAL);
          break;
        }
        case INTSXP: {
          int val = INTEGER(col)[i];
          insertValue(var, (int) adjustDatetimeFromR(vendor_, col, val), val == NA_INTEGER);
          break;
        }
        case REALSXP: {
          double val = REAL(col)[i];
          insertValue(var, adjustDatetimeFromR(vendor_, col, val), !R_finite(val));
          break;
        }
        case STRSXP: {
          insertValue(var, string_utf8(col, i), string_is_missing(col, i));
          break;
        }
        default:
          break;
        }
      }

      checkStatus(readstat_end_row(writer_));
    }

    checkStatus(readstat_end_writing(writer_));
  }

  // Define variables ----------------------------------------------------------

  const char* var_label(Rcpp::RObject x) {
    Rcpp::RObject label = x.attr("label");

    if (label == R_NilValue)
      return NULL;

    return string_utf8(label, 0);
  }

  const char* var_format(Rcpp::RObject x, VarType varType) {
    // Use attribute, if present
    Rcpp::RObject format = x.attr(formatAttribute(vendor_));
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

  void defineVariable(Rcpp::IntegerVector x, const char* name, const char* format = NULL) {
    readstat_label_set_t* labelSet = NULL;
    if (Rf_inherits(x, "factor")) {
      labelSet = readstat_add_label_set(writer_, READSTAT_TYPE_INT32, name);

      Rcpp::CharacterVector levels = Rcpp::as<Rcpp::CharacterVector>(x.attr("levels"));
      for (int i = 0; i < levels.size(); ++i)
        readstat_label_int32_value(labelSet, i + 1, string_utf8(levels, i));
    } else if (Rf_inherits(x, "haven_labelled") && TYPEOF(x.attr("labels")) != NILSXP) {
      labelSet = readstat_add_label_set(writer_, READSTAT_TYPE_INT32, name);

      Rcpp::IntegerVector values = Rcpp::as<Rcpp::IntegerVector>(x.attr("labels"));
      Rcpp::CharacterVector labels = Rcpp::as<Rcpp::CharacterVector>(values.attr("names"));

      for (int i = 0; i < values.size(); ++i)
        readstat_label_int32_value(labelSet, values[i], string_utf8(labels, i));
    }

    readstat_variable_t* var =
      readstat_add_variable(writer_, name, READSTAT_TYPE_INT32, 0);
    readstat_variable_set_format(var, format);
    readstat_variable_set_label(var, var_label(x));
    readstat_variable_set_label_set(var, labelSet);
    readstat_variable_set_measure(var, measureType(x));
    readstat_variable_set_display_width(var, displayWidth(x));
  }

  void defineVariable(Rcpp::NumericVector x, const char* name, const char* format = NULL) {
    readstat_label_set_t* labelSet = NULL;
    if (Rf_inherits(x, "haven_labelled") && TYPEOF(x.attr("labels")) != NILSXP) {
      labelSet = readstat_add_label_set(writer_, READSTAT_TYPE_DOUBLE, name);

      Rcpp::NumericVector values = Rcpp::as<Rcpp::NumericVector>(x.attr("labels"));
      Rcpp::CharacterVector labels = Rcpp::as<Rcpp::CharacterVector>(values.attr("names"));

      for (int i = 0; i < values.size(); ++i)
        readstat_label_double_value(labelSet, values[i], string_utf8(labels, i));
    }

    readstat_variable_t* var =
      readstat_add_variable(writer_, name, READSTAT_TYPE_DOUBLE, 0);

    readstat_variable_set_format(var, format);
    readstat_variable_set_label(var, var_label(x));
    readstat_variable_set_label_set(var, labelSet);
    readstat_variable_set_measure(var, measureType(x));
    readstat_variable_set_display_width(var, displayWidth(x));

    if (Rf_inherits(x, "haven_labelled_spss")) {
      SEXP na_range = x.attr("na_range");
      if (TYPEOF(na_range) == REALSXP && Rf_length(na_range) == 2) {
        readstat_variable_add_missing_double_range(var, REAL(na_range)[0], REAL(na_range)[1]);
      }

      SEXP na_values = x.attr("na_values");
      if (TYPEOF(na_values) == REALSXP) {
        int n = Rf_length(na_values);
        for (int i = 0; i < n; ++i) {
          readstat_variable_add_missing_double_value(var, REAL(na_values)[i]);
        }
      }
    }
  }

  void defineVariable(Rcpp::CharacterVector x, const char* name, const char* format = NULL) {
    readstat_label_set_t* labelSet = NULL;
    if (Rf_inherits(x, "haven_labelled") && TYPEOF(x.attr("labels")) != NILSXP) {
      labelSet = readstat_add_label_set(writer_, READSTAT_TYPE_STRING, name);

      Rcpp::CharacterVector values = Rcpp::as<Rcpp::CharacterVector>(x.attr("labels"));
      Rcpp::CharacterVector labels = Rcpp::as<Rcpp::CharacterVector>(values.attr("names"));

      for (int i = 0; i < values.size(); ++i)
        readstat_label_string_value(labelSet, string_utf8(values, i), string_utf8(labels, i));
    }
    int max_length = 0;
    for (int i = 0; i < x.size(); ++i) {
      int length = strlen(string_utf8(x, i));
      if (length > max_length)
        max_length = length;
    }

    readstat_variable_t* var =
      readstat_add_variable(writer_, name, READSTAT_TYPE_STRING, max_length);
    readstat_variable_set_format(var, format);
    readstat_variable_set_label(var, var_label(x));
    readstat_variable_set_label_set(var, labelSet);
    readstat_variable_set_measure(var, measureType(x));
    readstat_variable_set_display_width(var, displayWidth(x));
  }

  // Value helper -------------------------------------------------------------

  void insertValue(readstat_variable_t* var, int val, bool is_missing) {
    if (is_missing) {
      checkStatus(readstat_insert_missing_value(writer_, var));
    } else {
      checkStatus(readstat_insert_int32_value(writer_, var, val));
    }
  }

  void insertValue(readstat_variable_t* var, double val, bool is_missing) {
    if (is_missing) {
      checkStatus(readstat_insert_missing_value(writer_, var));
    } else {
      checkStatus(readstat_insert_double_value(writer_, var, val));
    }
  }

  void insertValue(readstat_variable_t* var, const char* val, bool is_missing) {
    if (is_missing) {
      checkStatus(readstat_insert_missing_value(writer_, var));
    } else {
      checkStatus(readstat_insert_string_value(writer_, var, val));
    }
  }

  // Misc ----------------------------------------------------------------------

  void checkStatus(readstat_error_t err) {
    if (err == 0) return;

    Rcpp::stop("Writing failure: %s.", readstat_error_message(err));
  }

  ssize_t write(const void *data, size_t len) {
    return fwrite(data, sizeof(char), len, pOut_);
  }
};

ssize_t data_writer(const void *data, size_t len, void *ctx) {
  return ((Writer*) ctx)->write(data, len);
}

[[tidycpp::export]]
void write_sav_(Rcpp::List data, Rcpp::CharacterVector path, bool compress) {
  Writer writer(HAVEN_SAV, data, path);
  if (compress)
    writer.setCompression(READSTAT_COMPRESS_BINARY);
  writer.write();
}

[[tidycpp::export]]
void write_dta_(Rcpp::List data, Rcpp::CharacterVector path, int version, Rcpp::RObject label) {
  Writer writer(HAVEN_DTA, data, path);
  writer.setVersion(version);
  writer.setFileLabel(label);
  writer.write();
}

[[tidycpp::export]]
void write_sas_(Rcpp::List data, Rcpp::CharacterVector path) {
  Writer(HAVEN_SAS7BDAT, data, path).write();
}

[[tidycpp::export]]
void write_xpt_(Rcpp::List data, Rcpp::CharacterVector path, int version, std::string name) {
  Writer writer(HAVEN_XPT, data, path);
  writer.setVersion(version);
  writer.setName(name);
  writer.write();
}
