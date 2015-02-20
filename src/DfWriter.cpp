#include <Rcpp.h>
using namespace Rcpp;
#include "readstat.h"

ssize_t data_writer(const void *data, size_t len, void *ctx);

class Writer {
  List x_;
  readstat_writer_t* writer_;
  FILE* pOut_;

public:
  Writer(List x, std::string path_): x_(x) {
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
    // No value sets for now

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
    readstat_begin_writing_sav(writer_, this, "", n);

    // Write data
    for (int i = 0; i < n; ++i) {
      readstat_begin_row(writer_);
      for (int j = 0; j < p; ++j) {
        RObject col = x_[j];
        readstat_variable_t* var = readstat_get_variable(writer_, j);

        switch (TYPEOF(col)) {
        case LGLSXP:
          readstat_insert_int32_value(writer_, var, LOGICAL(col)[i]);
          break;
        case INTSXP:
          readstat_insert_int32_value(writer_, var, INTEGER(col)[i]);
          break;
        case REALSXP:
          readstat_insert_double_value(writer_, var, REAL(col)[i]);
          break;
        case STRSXP:
          readstat_insert_string_value(writer_, var, CHAR(STRING_ELT(col, i)));
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

  void defineVariable(IntegerVector x, std::string name) {
    readstat_add_variable(writer_, READSTAT_TYPE_INT32, 0, name.c_str(),
      NULL, NULL, NULL);
  }

  void defineVariable(NumericVector x, std::string name) {
    readstat_add_variable(writer_, READSTAT_TYPE_DOUBLE, 0, name.c_str(),
      NULL, NULL, NULL);
  }

  void defineVariable(CharacterVector x, std::string name) {
    int max_length = 0;
    for (int i = 0; i < x.size(); ++i) {
      int length = std::string(x[i]).size();
      if (length > max_length)
        max_length = length;
    }

    readstat_add_variable(writer_, READSTAT_TYPE_STRING, max_length,
      name.c_str(), NULL, NULL, NULL);
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

// [[Rcpp::export]]
void write_sav(List data, std::string path) {
  Writer(data, path).write_sav();
}


// // Next define your value labels, if any. Create as many named sets as you'd like.
// readstat_label_set_t *readstat_add_label_set(readstat_writer_t *writer, readstat_types_t type, const char *name);
// void readstat_label_double_value(readstat_label_set_t *label_set, double value, const char *label);
// void readstat_label_int32_value(readstat_label_set_t *label_set, int32_t value, const char *label);
// void readstat_label_string_value(readstat_label_set_t *label_set, const char *value, const char *label);
//
// // Call one of these at any time before the first invocation of readstat_begin_row
// readstat_error_t readstat_begin_writing_sav(readstat_writer_t *writer, void *user_ctx,
//   const char *file_label, long row_count);
//
// readstat_error_t readstat_begin_writing_dta(readstat_writer_t *writer, void *user_ctx,
//   const char *file_label, long row_count);
//
// // Start a row of data (that is, a case or observation)
// readstat_error_t readstat_begin_row(readstat_writer_t *writer);
//
// // Then call one of these for each variable
// readstat_error_t readstat_insert_char_value(readstat_writer_t *writer, readstat_variable_t *variable, char value);
// readstat_error_t readstat_insert_int16_value(readstat_writer_t *writer, readstat_variable_t *variable, int16_t value);
// readstat_error_t readstat_insert_int32_value(readstat_writer_t *writer, readstat_variable_t *variable, int32_t value);
// readstat_error_t readstat_insert_float_value(readstat_writer_t *writer, readstat_variable_t *variable, float value);
// readstat_error_t readstat_insert_double_value(readstat_writer_t *writer, readstat_variable_t *variable, double value);
// readstat_error_t readstat_insert_string_value(readstat_writer_t *writer, readstat_variable_t *variable, const char *value);
// readstat_error_t readstat_insert_missing_value(readstat_writer_t *writer, readstat_variable_t *variable);
//
// // Finally, close out the row
// readstat_error_t readstat_end_row(readstat_writer_t *writer);
