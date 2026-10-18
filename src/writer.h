#ifndef HAVEN_WRITER_H
#define HAVEN_WRITER_H

#include "datetime.h"
#include "haven_types.h"
#include "readstat.h"
#include "writer_value.h"
#include "writer_variable.h"

#include "cpp11/doubles.hpp"
#include "cpp11/integers.hpp"
#include "cpp11/list.hpp"
#include "cpp11/sexp.hpp"
#include "cpp11/strings.hpp"
#include <unordered_map>

inline ssize_t data_writer(const void* data, size_t len, void* ctx);

// Owns the output file and readstat writer, and orchestrates writing:
// beginning the file, defining variables (via VariableDefiner), then writing
// data row by row.
class Writer {
  FileExt ext_;
  FileVendor vendor_;

  cpp11::list x_;
  readstat_writer_t* writer_;
  FILE* pOut_;

  std::unordered_map<std::string, readstat_string_ref_t*> string_ref_;
  VariableDefiner definer_;

public:
  Writer(FileExt ext, cpp11::list x, cpp11::strings pathEnc)
      : ext_(ext), vendor_(extVendor(ext)), x_(x) {
    std::string path(Rf_translateChar(pathEnc[0]));

    pOut_ = fopen(path.c_str(), "wb");
    if (pOut_ == NULL)
      cpp11::stop("Failed to open '%s' for writing", path.c_str());

    writer_ = readstat_writer_init();
    checkStatus(readstat_set_data_writer(writer_, data_writer));

    definer_ = VariableDefiner(writer_, ext_, &string_ref_);
  }

  ~Writer() {
    try {
      fclose(pOut_);
      readstat_writer_free(writer_);
    } catch (...) {
    };
  }

  void setCompression(readstat_compress_t version) {
    readstat_writer_set_compression(writer_, version);
  }

  void setVersion(int version) {
    readstat_writer_set_file_format_version(writer_, version);
    definer_.setVersion(version);
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
    definer_.setStrLThreshold(strl_threshold);
  }

  void write() {
    int p = x_.size();
    if (p == 0)
      return;

    int n = Rf_length(x_[0]);

    readstat_error_t status;
    switch (ext_) {
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
      cpp11::stop(
          "Failed to write metadata: %s.", readstat_error_message(status));
    }

    cpp11::strings names(x_.attr("names"));

    // Define variables
    for (int j = 0; j < p; ++j) {
      cpp11::sexp col = x_[j];
      const char* name = string_utf8(names, j);

      switch (TYPEOF(col)) {
      case LGLSXP:
        status = definer_.defineVariable(
            cpp11::integers(cpp11::safe[Rf_coerceVector](col, INTSXP)), name);
        break;
      case INTSXP:
        status = definer_.defineVariable(cpp11::integers(col), name);
        break;
      case REALSXP:
        status = definer_.defineVariable(cpp11::doubles(col), name);
        break;
      case STRSXP:
        status = definer_.defineVariable(cpp11::strings(col), name);
        break;
      default:
        cpp11::stop(
            "Columns of type %s not supported yet", Rf_type2char(TYPEOF(col)));
      }
      if (status) {
        cpp11::stop(
            "Failed to create column `%s`: %s.",
            name,
            readstat_error_message(status));
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
          status = insertValue(writer_, ext_, var, val, val == NA_LOGICAL);
          break;
        }
        case INTSXP: {
          int val = INTEGER(col)[i];
          status = insertValue(
              writer_,
              ext_,
              var,
              adjustDatetimeFromR(vendor_, col, val),
              val == NA_INTEGER);
          break;
        }
        case REALSXP: {
          double val = REAL(col)[i];
          status = insertValue(
              writer_,
              ext_,
              var,
              adjustDatetimeFromR(vendor_, col, val),
              !R_finite(val));
          break;
        }
        case STRSXP: {
          status = insertValue(
              writer_,
              string_ref_,
              var,
              string_utf8(col, i),
              string_is_missing(col, i));
          break;
        }
        default:
          status = READSTAT_OK;
          break;
        }

        if (status) {
          cpp11::stop(
              "Failed to insert value [%i, %i]: %s.",
              i + 1,
              j + 1,
              readstat_error_message(status));
        }
      }

      checkStatus(readstat_end_row(writer_));
    }

    checkStatus(readstat_end_writing(writer_));
  }

  void checkStatus(readstat_error_t err) {
    if (err == 0)
      return;

    cpp11::stop("Writing failure: %s.", readstat_error_message(err));
  }

  ssize_t write(const void* data, size_t len) {
    return fwrite(data, sizeof(char), len, pOut_);
  }
};

inline ssize_t data_writer(const void* data, size_t len, void* ctx) {
  return ((Writer*)ctx)->write(data, len);
}

#endif
