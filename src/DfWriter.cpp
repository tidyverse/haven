#include <Rcpp.h>
using namespace Rcpp;
#include "readstat.h"

ssize_t data_writer(const void *data, size_t len, void *ctx);
std::string rClass(RObject x);

enum FileType {
  HAVEN_SAV,
  HAVEN_DTA
};

class Writer {
  List x_;
  readstat_writer_t* writer_;
  FileType type_;
  FILE* pOut_;

public:
  Writer(List x, std::string path_, FileType type): x_(x), type_(type) {
    pOut_ = fopen(path_.c_str(), "wb");
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
      std::string name(names[j]);
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

    int n = Rf_length(x_[0]);
    switch (type_) {
    case HAVEN_DTA:
      readstat_begin_writing_dta(writer_, this, "", n);
      break;
    case HAVEN_SAV:
      readstat_begin_writing_sav(writer_, this, "", n);
      break;
    }

    // Write data
    for (int i = 0; i < n; ++i) {
      readstat_begin_row(writer_);
      for (int j = 0; j < p; ++j) {
        RObject col = x_[j];
        readstat_variable_t* var = readstat_get_variable(writer_, j);

        switch (TYPEOF(col)) {
        case LGLSXP: {
          int val = LOGICAL(col)[i];
          if (val == NA_LOGICAL) {
            readstat_insert_missing_value(writer_, var);
          } else {
            readstat_insert_int32_value(writer_, var, val);
          }
          break;
        }
        case INTSXP: {
          int val = INTEGER(col)[i];
          if (val == NA_INTEGER) {
            readstat_insert_missing_value(writer_, var);
          } else {
            readstat_insert_int32_value(writer_, var, val);
          }
          break;
        }
        case REALSXP: {
          double val = REAL(col)[i];
          if (val == NA_REAL) {
            readstat_insert_missing_value(writer_, var);
          } else {
            readstat_insert_double_value(writer_, var, val);
          }
          break;
        }
        case STRSXP: {
          SEXP val = STRING_ELT(col, i);
          if (val == NA_STRING) {
            readstat_insert_missing_value(writer_, var);
          } else {
            readstat_insert_string_value(writer_, var, CHAR(val));
          }
          break;
        }
          break;
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

    return CHAR(STRING_ELT(label, 0));
  }

  void defineVariable(IntegerVector x, std::string name) {
    readstat_label_set_t* labelSet = NULL;
    if (rClass(x) == "factor") {
      labelSet = readstat_add_label_set(writer_, READSTAT_TYPE_INT32, name.c_str());

      CharacterVector levels = as<CharacterVector>(x.attr("levels"));
      for (int i = 0; i < levels.size(); ++i)
        readstat_label_int32_value(labelSet, i + 1, std::string(levels[i]).c_str());
    } else if (rClass(x) == "labelled") {
      labelSet = readstat_add_label_set(writer_, READSTAT_TYPE_INT32, name.c_str());

      IntegerVector values = as<IntegerVector>(x.attr("labels"));
      CharacterVector labels = as<CharacterVector>(values.attr("names"));

      for (int i = 0; i < values.size(); ++i)
        readstat_label_int32_value(labelSet, values[i], std::string(labels[i]).c_str());

    }

    readstat_add_variable(writer_, READSTAT_TYPE_INT32, 0, name.c_str(),
      var_label(x), NULL, labelSet);
  }

  void defineVariable(NumericVector x, std::string name) {
    readstat_label_set_t* labelSet = NULL;
    if (rClass(x) == "labelled") {
      labelSet = readstat_add_label_set(writer_, READSTAT_TYPE_DOUBLE, name.c_str());

      NumericVector values = as<NumericVector>(x.attr("labels"));
      CharacterVector labels = as<CharacterVector>(values.attr("names"));

      for (int i = 0; i < values.size(); ++i)
        readstat_label_double_value(labelSet, values[i], std::string(labels[i]).c_str());
    }

    readstat_add_variable(writer_, READSTAT_TYPE_DOUBLE, 0, name.c_str(),
      var_label(x), NULL, labelSet);
  }

  void defineVariable(CharacterVector x, std::string name) {
    readstat_label_set_t* labelSet = NULL;
    if (rClass(x) == "labelled") {
      labelSet = readstat_add_label_set(writer_, READSTAT_TYPE_STRING, name.c_str());

      CharacterVector values = as<CharacterVector>(x.attr("labels"));
      CharacterVector labels = as<CharacterVector>(values.attr("names"));

      for (int i = 0; i < values.size(); ++i)
        readstat_label_string_value(labelSet, values[i], std::string(labels[i]).c_str());
    }

    int max_length = 0;
    for (int i = 0; i < x.size(); ++i) {
      int length = std::string(x[i]).size();
      if (length > max_length)
        max_length = length;
    }

    readstat_add_variable(writer_, READSTAT_TYPE_STRING, max_length,
      name.c_str(), var_label(x), NULL, labelSet);
  }

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

//' @export
//' @rdname read_spss
// [[Rcpp::export]]
void write_sav(List data, std::string path) {
  Writer(data, path, HAVEN_SAV).write_sav();
}

//' @export
//' @rdname read_dta
// [[Rcpp::export]]
void write_dta(List data, std::string path) {
  Writer(data, path, HAVEN_DTA).write_sav();
}
