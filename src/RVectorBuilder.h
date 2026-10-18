#ifndef __R_VECTOR_BUILDER__
#define __R_VECTOR_BUILDER__

#include <vector>

#include "haven_types.h"
#include "readstat.h"

#include "cpp11/doubles.hpp"
#include "cpp11/r_string.hpp"
#include "cpp11/sexp.hpp"
#include "cpp11/strings.hpp"

// Stateless helpers for constructing the R vectors and attributes that
// DfReader produces.

// Allocates a vector of the appropriate R type for a readstat variable
cpp11::sexp make_variable_vector(readstat_variable_t* variable, R_xlen_t n) {
  switch (readstat_variable_get_type(variable)) {
  case READSTAT_TYPE_STRING_REF:
  case READSTAT_TYPE_STRING:
    return cpp11::sexp(cpp11::writable::strings(n));
  default:
    return cpp11::sexp(cpp11::writable::doubles(n));
  }
}

// Sets the variable label, original format, and display width attributes
void set_metadata_attributes(
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
cpp11::writable::doubles make_timestamp(time_t time) {
  cpp11::writable::doubles out({(double)time});
  out.attr("class") = {"POSIXct", "POSIXt"};
  out.attr("tzone") = "UTC";
  return out;
}

// Sets the R class corresponding to a date/time format, warning and skipping
// if the format is incompatible with the variable's storage type. Files have
// been observed in the wild with a date format applied to a string variable,
// which should not be possible.
void set_format_class(
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
void set_user_na_attributes(cpp11::sexp col, readstat_variable_t* variable) {
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

#endif
