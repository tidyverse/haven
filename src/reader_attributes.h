#ifndef HAVEN_READER_ATTRIBUTES_H
#define HAVEN_READER_ATTRIBUTES_H

#include <string>
#include <vector>

#include "datetime.h"
#include "haven_types.h"
#include "readstat.h"

#include "cpp11/doubles.hpp"
#include "cpp11/integers.hpp"
#include "cpp11/r_string.hpp"
#include "cpp11/sexp.hpp"
#include "cpp11/strings.hpp"

// Helpers for constructing the R vectors and attributes that Reader produces.

// Allocates a vector of the appropriate R type for a readstat variable
inline cpp11::sexp
make_variable_vector(readstat_variable_t* variable, R_xlen_t n) {
  // The explicit SEXP casts avoid an ambiguous conversion under C++14, the
  // default for R 4.2. They can be dropped once we no longer support R 4.2.
  switch (readstat_variable_get_type(variable)) {
  case READSTAT_TYPE_STRING_REF:
  case READSTAT_TYPE_STRING:
    return cpp11::sexp(static_cast<SEXP>(cpp11::writable::strings(n)));
  default:
    return cpp11::sexp(static_cast<SEXP>(cpp11::writable::doubles(n)));
  }
}

// Sets the variable label, original format, and display width attributes
inline void set_metadata_attributes(
    cpp11::sexp col, readstat_variable_t* variable, FileVendor vendor) {
  const char* var_label = readstat_variable_get_label(variable);
  if (var_label != NULL && strcmp(var_label, "") != 0) {
    col.attr("label") = var_label;
  }

  // Store original format as attribute
  const char* var_format = readstat_variable_get_format(variable);
  if (var_format != NULL && strcmp(var_format, "") != 0) {
    col.attr(formatAttribute(vendor)) = var_format;
  }

  // Store original display width as attribute if it differs from the default
  int display_width = readstat_variable_get_display_width(variable);
  if (vendor == HAVEN_SPSS && display_width != 8) {
    col.attr("display_width") = Rf_ScalarInteger(display_width);
  }
}

// A length-1 POSIXct vector in UTC
inline cpp11::writable::doubles make_timestamp(time_t time) {
  cpp11::writable::doubles out({(double)time});
  out.attr("class") = {"POSIXct", "POSIXt"};
  out.attr("tzone") = "UTC";
  return out;
}

// Sets the R class corresponding to a date/time format, warning and skipping
// if the format is incompatible with the variable's storage type. Files have
// been observed in the wild with a date format applied to a string variable,
// which should not be possible.
inline void set_format_class(
    cpp11::sexp col,
    readstat_variable_t* variable,
    VarType var_type,
    bool quiet) {
  if (var_type != HAVEN_DEFAULT && readstat_variable_get_type_class(variable) ==
                                       READSTAT_TYPE_CLASS_STRING) {
    if (!quiet)
      Rf_warning(
          "String variable '%s' has incompatible format '%s' and will "
          "be returned as a regular string variable.",
          readstat_variable_get_name(variable),
          readstat_variable_get_format(variable));
    return;
  }

  switch (var_type) {
  case HAVEN_DATE:
    col.attr("class") = "Date";
    break;
  case HAVEN_TIME:
    col.attr("class") = {"hms", "difftime"};
    col.attr("units") = "secs";
    break;
  case HAVEN_DATETIME:
    col.attr("class") = {"POSIXct", "POSIXt"};
    col.attr("tzone") = "UTC";
    break;
  default:
    break;
  }
}

// Sets the `na_values`/`na_range` attributes for SPSS user-defined missing
// values and upgrades `col` to haven_labelled_spss
inline void
set_user_na_attributes(cpp11::sexp col, readstat_variable_t* variable) {
  int n_ranges = readstat_variable_get_missing_ranges_count(variable);
  if (n_ranges == 0) {
    return;
  }

  switch (readstat_variable_get_type(variable)) {
  case READSTAT_TYPE_STRING_REF:
  case READSTAT_TYPE_STRING: {
    cpp11::writable::strings na_values(R_xlen_t(0));
    cpp11::writable::strings na_range(2);
    bool has_range = false;

    for (int i = 0; i < n_ranges; ++i) {
      readstat_value_t lo_value =
                           readstat_variable_get_missing_range_lo(variable, i),
                       hi_value =
                           readstat_variable_get_missing_range_hi(variable, i);

      const char* lo = readstat_string_value(lo_value);
      const char* hi = readstat_string_value(hi_value);

      if (lo == hi) {
        // single value
        na_values.push_back(
            lo == NULL ? cpp11::r_string(NA_STRING) : cpp11::r_string(lo));
      } else {
        has_range = true;
        // Can only ever be one range
        na_range[0] = lo;
        na_range[1] = hi;
      }
    }

    if (na_values.size() > 0)
      col.attr("na_values") = na_values;
    if (has_range)
      col.attr("na_range") = na_range;

    col.attr("class") = {
        "haven_labelled_spss", "haven_labelled", "vctrs_vctr", "character"};
    break;
  }
  case READSTAT_TYPE_INT8:
  case READSTAT_TYPE_INT16:
  case READSTAT_TYPE_INT32:
  case READSTAT_TYPE_FLOAT:
  case READSTAT_TYPE_DOUBLE: {
    std::vector<double> na_values;
    cpp11::writable::doubles na_range(2);
    bool has_range = false;

    for (int i = 0; i < n_ranges; ++i) {
      readstat_value_t lo_value =
                           readstat_variable_get_missing_range_lo(variable, i),
                       hi_value =
                           readstat_variable_get_missing_range_hi(variable, i);

      double lo = readstat_double_value(lo_value),
             hi = readstat_double_value(hi_value);

      if (lo == hi) {
        // Single value
        na_values.push_back(lo);
      } else {
        has_range = true;
        // Can only ever be one range
        na_range[0] = lo;
        na_range[1] = hi;
      }
    }

    if (na_values.size() > 0)
      col.attr("na_values") = na_values;
    if (has_range)
      col.attr("na_range") = na_range;

    col.attr("class") = {
        "haven_labelled_spss", "haven_labelled", "vctrs_vctr", "double"};
  }
  }
}

// Accumulates value labels for a single label set, then converts them to
// the named vector used by the `labels` attribute.
class LabelSet {
  std::vector<std::string> labels_;
  std::vector<std::string> values_s_;
  std::vector<int> values_i_;
  std::vector<double> values_d_;

public:
  LabelSet() {}

  void add(const char* value, std::string label) {
    if (values_i_.size() > 0 || values_d_.size() > 0)
      cpp11::stop("Can't add string to integer/double labelset");

    values_s_.push_back(value);
    labels_.push_back(label);
  }

  void add(int value, std::string label) {
    if (values_d_.size() > 0 || values_s_.size() > 0)
      cpp11::stop("Can't add integer to string/double labelset");

    values_i_.push_back(value);
    labels_.push_back(label);
  }

  void add(double value, std::string label) {
    if (values_i_.size() > 0 || values_s_.size() > 0)
      cpp11::stop("Can't add double to integer/string labelset");

    values_d_.push_back(value);
    labels_.push_back(label);
  }

  size_t size() const { return labels_.size(); }

  cpp11::sexp labels() const {
    cpp11::sexp out;

    if (values_i_.size() > 0) {
      int n = values_i_.size();
      cpp11::writable::integers values(n);
      cpp11::writable::strings labels(n);

      for (int i = 0; i < n; ++i) {
        values[i] = values_i_[i];
        labels[i] = labels_[i].c_str();
      }

      values.attr("names") = labels;
      out = values;
    } else if (values_d_.size() > 0) {
      int n = values_d_.size();
      cpp11::writable::doubles values(n);
      cpp11::writable::strings labels(n);

      for (int i = 0; i < n; ++i) {
        values[i] = values_d_[i];
        labels[i] = labels_[i].c_str();
      }

      values.attr("names") = labels;
      out = values;
    } else {
      int n = values_s_.size();
      cpp11::writable::strings values(n), labels(n);

      for (int i = 0; i < n; ++i) {
        values[i] = values_s_[i].c_str();
        labels[i] = labels_[i].c_str();
      }

      values.attr("names") = labels;
      out = values;
    }

    return out;
  }
};

#endif
