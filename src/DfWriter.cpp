#include <Rcpp.h>
using namespace Rcpp;
#include "readstat.h"
#include "haven_types.h"

ssize_t data_writer(const void *data, size_t len, void *ctx);
std::string rClass(RObject x);

inline const char* string_utf8(SEXP x, int i) {
  return Rf_translateCharUTF8(STRING_ELT(x, i));
}

inline const bool string_is_missing(SEXP x, int i) {
  return STRING_ELT(x, i) == NA_STRING;
}


class Writer {
  List x_;
  readstat_writer_t* writer_;
  FILE* pOut_;

public:
  Writer(List x, std::string path): x_(x) {
    pOut_ = fopen(path.c_str(), "wb");
    if (pOut_ == NULL)
      stop("Failed to open '%s' for writing", path);

    writer_ = readstat_writer_init();
    checkStatus(readstat_set_data_writer(writer_, data_writer));
  }

  ~Writer() {
    try {
      fclose(pOut_);
      readstat_writer_free(writer_);
    } catch (...) {};
  }

  void write_sav() {
    CharacterVector names = as<CharacterVector>(x_.attr("names"));

    int p = x_.size();
    if (p == 0)
      return;

    // Define variables
    for (int j = 0; j < p; ++j) {
      RObject col = x_[j];

      const char* name = string_utf8(names, j);

      switch(TYPEOF(col)) {
      case LGLSXP:
        defineVariable(as<IntegerVector>(col), name);
        break;
      case INTSXP:
        defineVariable(as<IntegerVector>(col), name);
        break;
      case REALSXP:
        defineVariable(as<NumericVector>(col), name);
        break;
      case STRSXP:
        defineVariable(as<CharacterVector>(col), name);
        break;
      default:
        stop("Variables of type %s not supported yet",
          Rf_type2str(TYPEOF(col)));
      }
    }

    int n = Rf_length(x_[0]);
    readstat_begin_writing_sav(writer_, this, n);

    // Write data
    for (int i = 0; i < n; ++i) {
      readstat_begin_row(writer_);
      for (int j = 0; j < p; ++j) {
        RObject col = x_[j];
        readstat_variable_t* var = readstat_get_variable(writer_, j);

        switch (TYPEOF(col)) {
        case LGLSXP: {
          int val = LOGICAL(col)[i];
          insertValue(var, val, val == NA_LOGICAL);
          break;
        }
        case INTSXP: {
          int val = INTEGER(col)[i];
          insertValue(var, val, val == NA_INTEGER);
          break;
        }
        case REALSXP: {
          double val = REAL(col)[i];
          insertValue(var, val, !R_finite(val));
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
      readstat_end_row(writer_);
    }

    readstat_end_writing(writer_);

  }

  void write_dta() {
    CharacterVector names = as<CharacterVector>(x_.attr("names"));

    int p = x_.size();
    if (p == 0)
      return;

    // Define variables
    for (int j = 0; j < p; ++j) {
      RObject col = x_[j];
      const char* name = string_utf8(names, j);

      switch(numType(col)) {
      case HAVEN_DATETIME:
        defineVariable(as<NumericVector>(col), name, "%tc");
        break;
      case HAVEN_DATE:
        defineVariable(as<NumericVector>(col), name, "%td");
        break;
      case HAVEN_TIME: // Stata doesn't have a pure time variable
      case HAVEN_DEFAULT:
        switch(TYPEOF(col)) {
        case LGLSXP:
          defineVariable(as<IntegerVector>(col), name);
          break;
        case INTSXP:
          defineVariable(as<IntegerVector>(col), name);
          break;
        case REALSXP:
          defineVariable(as<NumericVector>(col), name);
          break;
        case STRSXP:
          defineVariable(as<CharacterVector>(col), name);
          break;
        default:
          stop("Variables of type %s not supported yet",
            Rf_type2char(TYPEOF(col)));
        }
      }
    }

    int n = Rf_length(x_[0]);
    readstat_begin_writing_dta(writer_, this, n);

    // Write data
    for (int i = 0; i < n; ++i) {
      readstat_begin_row(writer_);
      for (int j = 0; j < p; ++j) {
        RObject col = x_[j];
        readstat_variable_t* var = readstat_get_variable(writer_, j);

        switch (TYPEOF(col)) {
        case LGLSXP: {
          int val = LOGICAL(col)[i];
          insertValue(var, val, val == NA_LOGICAL);
          break;
        }
        case INTSXP: {
          int val = INTEGER(col)[i];
          insertValue(var, (int) adjustDatetimeFromR(HAVEN_STATA, col, val), val == NA_INTEGER);
          break;
        }
        case REALSXP: {
          double val = REAL(col)[i];
          insertValue(var, adjustDatetimeFromR(HAVEN_STATA, col, val), !R_finite(val));
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
      readstat_end_row(writer_);
    }

    readstat_end_writing(writer_);
  }

  // Define variables ----------------------------------------------------------

  const char* var_label(RObject x) {
    RObject label = x.attr("label");

    if (label == R_NilValue)
      return NULL;

    return string_utf8(label, 0);
  }

  void defineVariable(IntegerVector x, const char* name, const char* format = NULL) {
    readstat_label_set_t* labelSet = NULL;
    if (rClass(x) == "factor") {
      labelSet = readstat_add_label_set(writer_, READSTAT_TYPE_INT32, name);

      CharacterVector levels = as<CharacterVector>(x.attr("levels"));
      for (int i = 0; i < levels.size(); ++i)
        readstat_label_int32_value(labelSet, i + 1, string_utf8(levels, i));
    } else if (rClass(x) == "labelled") {
      labelSet = readstat_add_label_set(writer_, READSTAT_TYPE_INT32, name);

      IntegerVector values = as<IntegerVector>(x.attr("labels"));
      CharacterVector labels = as<CharacterVector>(values.attr("names"));

      for (int i = 0; i < values.size(); ++i)
        readstat_label_int32_value(labelSet, values[i], string_utf8(labels, i));
    }

    readstat_add_variable(writer_, READSTAT_TYPE_INT32, 0, name,
      var_label(x), format, labelSet);
  }

  void defineVariable(NumericVector x, const char* name, const char* format = NULL) {
    readstat_label_set_t* labelSet = NULL;
    if (rClass(x) == "labelled") {
      labelSet = readstat_add_label_set(writer_, READSTAT_TYPE_DOUBLE, name);

      NumericVector values = as<NumericVector>(x.attr("labels"));
      CharacterVector labels = as<CharacterVector>(values.attr("names"));

      for (int i = 0; i < values.size(); ++i)
        readstat_label_double_value(labelSet, values[i], string_utf8(labels, i));
    }

    readstat_add_variable(writer_, READSTAT_TYPE_DOUBLE, 0, name,
      var_label(x), format, labelSet);
  }

  void defineVariable(CharacterVector x, const char* name, const char* format = NULL) {
    readstat_label_set_t* labelSet = NULL;
    if (rClass(x) == "labelled") {
      labelSet = readstat_add_label_set(writer_, READSTAT_TYPE_STRING, name);

      CharacterVector values = as<CharacterVector>(x.attr("labels"));
      CharacterVector labels = as<CharacterVector>(values.attr("names"));

      for (int i = 0; i < values.size(); ++i)
        readstat_label_string_value(labelSet, string_utf8(values, i), string_utf8(labels, i));
    }

    int max_length = 0;
    for (int i = 0; i < x.size(); ++i) {
      int length = std::string(x[i]).size();
      if (length > max_length)
        max_length = length;
    }

    readstat_add_variable(writer_, READSTAT_TYPE_STRING, max_length,
      name, var_label(x), format, labelSet);
  }

  // Value helper -------------------------------------------------------------

  void insertValue(readstat_variable_t* var, int val, bool is_missing) {
    if (is_missing) {
      readstat_insert_missing_value(writer_, var);
    } else {
      readstat_insert_int32_value(writer_, var, val);
    }
  }

  void insertValue(readstat_variable_t* var, double val, bool is_missing) {
    if (is_missing) {
      readstat_insert_missing_value(writer_, var);
    } else {
      readstat_insert_double_value(writer_, var, val);
    }
  }

  void insertValue(readstat_variable_t* var, const char* val, bool is_missing) {
    if (is_missing) {
      readstat_insert_missing_value(writer_, var);
    } else {
      readstat_insert_string_value(writer_, var, val);
    }
  }

  // Misc ----------------------------------------------------------------------

  void checkStatus(readstat_error_t err) {
    if (err == 0) return;

    stop("Writing failure: %s.", readstat_error_message(err));
  }

  ssize_t write(const void *data, size_t len) {
    return fwrite(data, sizeof(char), len, pOut_);
  }
};

ssize_t data_writer(const void *data, size_t len, void *ctx) {
  return ((Writer*) ctx)->write(data, len);
}

std::string rClass(RObject x) {
  RObject klass_ = x.attr("class");
  std::string klass;
  if (klass_ == R_NilValue)
    return "";

  CharacterVector klassv = as<Rcpp::CharacterVector>(klass_);
  return std::string(klassv[0]);
}
// [[Rcpp::export]]
void write_sav_(List data, std::string path) {
  Writer(data, path).write_sav();
}

// [[Rcpp::export]]
void write_dta_(List data, std::string path) {
  Writer(data, path).write_dta();
}
