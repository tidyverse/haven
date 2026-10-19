#ifndef HAVEN_READER_H
#define HAVEN_READER_H

#include <algorithm>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "datetime.h"
#include "haven_types.h"
#include "reader_attributes.h"
#include "readstat.h"
#include "tagged_na.h"

#include "cpp11/doubles.hpp"
#include "cpp11/integers.hpp"
#include "cpp11/list.hpp"
#include "cpp11/r_string.hpp"
#include "cpp11/sexp.hpp"
#include "cpp11/strings.hpp"

#include "cpp11/function.hpp"
#include "cpp11/protect.hpp"

// Converts a readstat double to an R double, handling user-defined missings:
// tagged missings become tagged NAs, and defined/system missings become
// NA_REAL unless `user_na` is true.
inline double haven_double_value_with_user_na(
    readstat_value_t value, readstat_variable_t* var, bool user_na) {
  if (readstat_value_is_tagged_missing(value)) {
    return make_tagged_na(tolower(readstat_value_tag(value)));
  } else if (!user_na && readstat_value_is_defined_missing(value, var)) {
    return NA_REAL;
  } else if (readstat_value_is_system_missing(value)) {
    return NA_REAL;
  } else {
    return readstat_double_value(value);
  }
}

inline double haven_double_value(readstat_value_t value) {
  if (readstat_value_is_tagged_missing(value)) {
    return make_tagged_na(tolower(readstat_value_tag(value)));
  } else {
    return readstat_double_value(value);
  }
}

class Reader {
  FileVendor vendor_;

  int nrows_, nrowsAlloc_;
  int ncols_;
  cpp11::writable::list output_;
  cpp11::writable::strings names_;
  bool user_na_;

  std::vector<std::string> val_labels_;
  std::map<std::string, LabelSet> label_sets_;
  std::vector<VarType> var_types_;
  std::vector<std::string> notes_;

  std::set<int> colsSkip_;
  bool quiet_;
  long n_max_;

public:
  Reader(FileExt ext, bool user_na = false)
      : vendor_(extVendor(ext)),
        nrows_(0),
        ncols_(0),
        output_(static_cast<R_xlen_t>(0)),
        user_na_(user_na),
        quiet_(false),
        n_max_(-1) {}

  // Suppress metadata warnings, e.g. when only reading the header
  void setQuiet(bool quiet) { quiet_ = quiet; }

  void setMaxRows(long n) { n_max_ = n; }

  void skipCols(cpp11::integers cols) {
    std::set<int> cols_set(cols.begin(), cols.end());
    colsSkip_ = cols_set;
  }

  void setInfo(int obs_count, int var_count) {
    // If unknown, start with 1e5, and use doubling strategy
    nrowsAlloc_ = obs_count < 0 ? 1e5 : obs_count;
    // But never allocate more rows than what we request
    if (n_max_ >= 0 && n_max_ < nrowsAlloc_)
      nrowsAlloc_ = n_max_;

    nrows_ = obs_count < 0 ? 0 : obs_count;

    if (var_count < 1) {
      return; // sas7bcat has var_count = 0
    }

    ncols_ = var_count - colsSkip_.size();

    output_.resize(ncols_);
    names_.resize(ncols_);
    val_labels_.resize(ncols_);
    var_types_.resize(ncols_);
  }

  void setMetadata(const char* file_label) {
    if (file_label != NULL && strcmp(file_label, "") != 0) {
      output_.attr("label") = file_label;
    }
  }

  void setTimestamp(time_t c_time, time_t m_time) {
    if (c_time != 0) {
      output_.attr("creation_timestamp") = make_timestamp(c_time);
    }
    if (m_time != 0) {
      output_.attr("modified_timestamp") = make_timestamp(m_time);
    }
  }

  void setNote(int note_index, const char* note) {
    if (note != NULL && strcmp(note, "") != 0) {
      notes_.push_back(note);
    }
  }

  int createVariable(
      int index, readstat_variable_t* variable, const char* val_labels) {
    if (colsSkip_.count(index) > 0) {
      return READSTAT_HANDLER_SKIP_VARIABLE;
    }

    const char* name = readstat_variable_get_name(variable);
    int var_index = readstat_variable_get_index_after_skipping(variable);
    const char* var_format = readstat_variable_get_format(variable);
    VarType var_type = varTypeFromFormat(vendor_, var_format);
    // Rcout << name << ": " << var_format << " [" << var_type << "]\n";

    names_[var_index] = name;
    if (val_labels != NULL)
      val_labels_[var_index] = val_labels;
    var_types_[var_index] = var_type;

    output_[var_index] = make_variable_vector(variable, nrowsAlloc_);
    cpp11::sexp col(output_[var_index]);

    set_format_class(col, variable, var_type, quiet_);
    set_metadata_attributes(col, variable, vendor_);
    // User defined missing values
    if (user_na_) {
      set_user_na_attributes(col, variable);
    }

    return READSTAT_HANDLER_OK;
  }

  void setValue(
      int obs_index, readstat_variable_t* variable, readstat_value_t value) {
    int var_index = readstat_variable_get_index_after_skipping(variable);

    VarType var_type = var_types_[var_index];

    if (obs_index >= nrowsAlloc_)
      resizeCols(nrowsAlloc_ * 2);
    if (obs_index >= nrows_)
      nrows_ = obs_index + 1;

    switch (value.type) {
    case READSTAT_TYPE_STRING_REF:
    case READSTAT_TYPE_STRING: {
      cpp11::writable::strings col(output_[var_index]);
      const char* str_value = readstat_string_value(value);

      if (readstat_value_is_tagged_missing(value)) {
        col[obs_index] = NA_STRING;
      } else if (
          !user_na_ && readstat_value_is_defined_missing(value, variable)) {
        col[obs_index] = NA_STRING;
      } else if (readstat_value_is_system_missing(value)) {
        col[obs_index] = NA_STRING;
      } else if (str_value == NULL) {
        col[obs_index] = cpp11::r_string("");
      } else {
        col[obs_index] = cpp11::r_string(str_value);
      }
      break;
    }
    case READSTAT_TYPE_INT8:
    case READSTAT_TYPE_INT16:
    case READSTAT_TYPE_INT32:
    case READSTAT_TYPE_FLOAT:
    case READSTAT_TYPE_DOUBLE: {
      cpp11::writable::doubles col(output_[var_index]);
      double val = haven_double_value_with_user_na(value, variable, user_na_);
      col[obs_index] = adjustDatetimeToR(vendor_, var_type, val);
      break;
    }
    }
  }

  void setValueLabels(
      const char* val_labels, readstat_value_t value, const char* label) {
    LabelSet& label_set = label_sets_[val_labels];
    std::string label_s(label);

    switch (value.type) {
    case READSTAT_TYPE_STRING:
      // Encoded to utf-8 on output
      label_set.add(readstat_string_value(value), label_s);
      break;
    case READSTAT_TYPE_INT8:
    case READSTAT_TYPE_INT16:
    case READSTAT_TYPE_INT32:
    case READSTAT_TYPE_DOUBLE:
      label_set.add(haven_double_value(value), label_s);
      break;
    default:
      if (!quiet_)
        Rf_warning("Unsupported label type: %i", value.type);
    }
  }

  bool hasLabel(int var_index) const {
    std::string label = val_labels_[var_index];
    if (label == "")
      return false;

    return label_sets_.count(label) > 0;
  }

  void resizeCols(int n) {
    // Rcout << "resizing to " << n << "\n";
    nrowsAlloc_ = n;

    for (int i = 0; i < ncols_; ++i) {
      cpp11::sexp copy(Rf_lengthgets(output_[i], n));
      Rf_copyMostAttrib(output_[i], copy);
      output_[i] = copy;
    }
  }

  void limitRows(long n) {
    if (nrows_ > n) {
      nrows_ = n;
    }
  }

  cpp11::list output() {
    if (nrows_ != nrowsAlloc_)
      resizeCols(nrows_);

    for (int i = 0; i < output_.size(); ++i) {
      cpp11::sexp col(output_[i]);

      if (hasLabel(i)) {
        if (Rf_getAttrib(col, R_ClassSymbol) == R_NilValue) {
          col.attr("class") = {
              "haven_labelled", "vctrs_vctr", Rf_type2char(TYPEOF(col))};
        }
        col.attr("labels") = label_sets_[val_labels_[i]].labels();
      }
    }

    if (!notes_.empty()) {
      output_.attr("notes") = notes_;
    }

    output_.attr("names") = names_;

    static cpp11::function as_tibble = cpp11::package("tibble")["as_tibble"];
    using namespace cpp11::literals;
    return SEXP(as_tibble(
        output_,
        ".rows"_nm = nrows_,
        ".name_repair"_nm = cpp11::r_string("none")));
  }
};

#endif
