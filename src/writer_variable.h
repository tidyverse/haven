#ifndef HAVEN_WRITER_VARIABLE_H
#define HAVEN_WRITER_VARIABLE_H

#include "datetime.h"
#include "haven_types.h"
#include "readstat.h"
#include "tagged_na.h"
#include "writer_value.h"

#include "cpp11/doubles.hpp"
#include "cpp11/integers.hpp"
#include "cpp11/sexp.hpp"
#include "cpp11/strings.hpp"
#include <algorithm>
#include <cmath>
#include <string>
#include <unordered_map>

inline readstat_measure_e measureType(SEXP x) {
  if (Rf_inherits(x, "ordered")) {
    return READSTAT_MEASURE_ORDINAL;
  } else if (Rf_inherits(x, "factor")) {
    return READSTAT_MEASURE_NOMINAL;
  } else {
    switch (TYPEOF(x)) {
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

inline int intAttribute(cpp11::sexp x, const char* name) {
  cpp11::sexp obj(x.attr(name));
  switch (TYPEOF(obj)) {
  case INTSXP:
    return INTEGER(obj)[0];
  case REALSXP:
    return REAL(obj)[0];
  }
  return 0;
}

inline bool has_labels(cpp11::sexp x) {
  return Rf_inherits(x, "haven_labelled") && TYPEOF(x.attr("labels")) != NILSXP;
}

// Defines variables (and their label sets and missing values) in a
// readstat writer, choosing storage types and formats appropriate for
// the file format.
class VariableDefiner {
  FileExt ext_;
  FileVendor vendor_;
  int version_;
  int strl_threshold_;
  std::unordered_map<std::string, readstat_string_ref_t*>* string_ref_;
  readstat_writer_t* writer_;

public:
  VariableDefiner()
      : ext_(HAVEN_DTA),
        vendor_(HAVEN_STATA),
        version_(0),
        strl_threshold_(0),
        string_ref_(NULL),
        writer_(NULL) {}

  VariableDefiner(
      readstat_writer_t* writer,
      FileExt ext,
      std::unordered_map<std::string, readstat_string_ref_t*>* string_ref)
      : ext_(ext),
        vendor_(extVendor(ext)),
        version_(0),
        strl_threshold_(0),
        string_ref_(string_ref),
        writer_(writer) {}

  void setVersion(int version) { version_ = version; }

  void setStrLThreshold(int strl_threshold) {
    strl_threshold_ = strl_threshold;
  }

  readstat_error_t defineVariable(cpp11::integers x, const char* name) {
    readstat_label_set_t* labelSet = label_set(x, name);
    readstat_variable_t* var = readstat_add_variable(
        writer_, name, stataNumericType(x, READSTAT_TYPE_INT32), userWidth(x));
    setMetadata(var, x, labelSet);
    addSpssMissing(var, x);
    return readstat_validate_variable(writer_, var);
  }

  readstat_error_t defineVariable(cpp11::doubles x, const char* name) {
    readstat_label_set_t* labelSet = label_set(x, name);
    readstat_variable_t* var = readstat_add_variable(
        writer_, name, stataNumericType(x, READSTAT_TYPE_DOUBLE), userWidth(x));
    setMetadata(var, x, labelSet);
    addSpssMissing(var, x);
    return readstat_validate_variable(writer_, var);
  }

  readstat_error_t defineVariable(cpp11::strings x, const char* name) {
    readstat_label_set_t* labelSet = label_set(x, name);
    int width = stringWidth(x, name, labelSet);

    // Use strL for "long" strings in stata. strL has an 80 byte overhead so
    // we use it when it's likely to be more efficient. The main downside of
    // strL is that it can't be used as a join key but this seems unlikely for
    // very long strings.
    readstat_variable_t* var;
    if (ext_ == HAVEN_DTA && version_ >= 117 && width > strl_threshold_) {
      var =
          readstat_add_variable(writer_, name, READSTAT_TYPE_STRING_REF, width);
      for (int i = 0; i < x.size(); ++i) {
        // Stata has no missing strL; use an empty string instead
        std::string val(string_is_missing(x, i) ? "" : string_utf8(x, i));
        if (!string_ref_->count(val)) {
          (*string_ref_)[val] = readstat_add_string_ref(writer_, val.c_str());
        }
      }
    } else {
      var = readstat_add_variable(writer_, name, READSTAT_TYPE_STRING, width);
    }

    setMetadata(var, x, labelSet);
    addSpssMissing(var, x);
    return readstat_validate_variable(writer_, var);
  }

private:
  // Label sets ---------------------------------------------------------------

  readstat_label_set_t* label_set(cpp11::integers x, const char* name) {
    if (Rf_inherits(x, "factor")) {
      readstat_label_set_t* labelSet =
          readstat_add_label_set(writer_, READSTAT_TYPE_INT32, name);

      cpp11::strings levels(x.attr("levels"));
      for (int i = 0; i < levels.size(); ++i)
        readstat_label_int32_value(labelSet, i + 1, string_utf8(levels, i));
      return labelSet;
    } else if (has_labels(x)) {
      readstat_label_set_t* labelSet =
          readstat_add_label_set(writer_, READSTAT_TYPE_INT32, name);

      cpp11::integers values(x.attr("labels"));
      cpp11::strings labels(values.attr("names"));

      for (int i = 0; i < values.size(); ++i)
        readstat_label_int32_value(labelSet, values[i], string_utf8(labels, i));
      return labelSet;
    }
    return NULL;
  }

  readstat_label_set_t* label_set(cpp11::doubles x, const char* name) {
    if (!has_labels(x))
      return NULL;

    readstat_label_set_t* labelSet =
        readstat_add_label_set(writer_, READSTAT_TYPE_DOUBLE, name);

    cpp11::doubles values(x.attr("labels"));
    cpp11::strings labels(values.attr("names"));

    for (int i = 0; i < values.size(); ++i) {
      char tag = tagged_na_value(values[i]);
      if (!std::isnan(values[i]) || tag == '\0') {
        readstat_label_double_value(
            labelSet, values[i], string_utf8(labels, i));
      } else {
        readstat_label_tagged_value(
            labelSet, write_tag(tag, ext_), string_utf8(labels, i));
      }
    }
    return labelSet;
  }

  readstat_label_set_t* label_set(cpp11::strings x, const char* name) {
    if (!has_labels(x))
      return NULL;

    readstat_label_set_t* labelSet =
        readstat_add_label_set(writer_, READSTAT_TYPE_STRING, name);

    cpp11::strings values(x.attr("labels"));
    cpp11::strings labels(values.attr("names"));

    for (int i = 0; i < values.size(); ++i)
      readstat_label_string_value(
          labelSet, string_utf8(values, i), string_utf8(labels, i));
    return labelSet;
  }

  // Variable metadata
  // ----------------------------------------------------------

  void setMetadata(
      readstat_variable_t* var, cpp11::sexp x, readstat_label_set_t* labelSet) {
    readstat_variable_set_format(var, var_format(x));
    readstat_variable_set_label(var, var_label(x));
    readstat_variable_set_label_set(var, labelSet);
    readstat_variable_set_measure(var, measureType(x));
    readstat_variable_set_display_width(var, intAttribute(x, "display_width"));
  }

  const char* var_label(cpp11::sexp x) {
    cpp11::sexp label(x.attr("label"));

    if (label == R_NilValue)
      return NULL;

    return string_utf8(label, 0);
  }

  const char* var_format(cpp11::sexp x) {
    // Use attribute, if present
    cpp11::sexp format(x.attr(formatAttribute(vendor_).c_str()));
    if (format != R_NilValue)
      return string_utf8(format, 0);

    switch (numType(x)) {
    case HAVEN_DEFAULT:
      return NULL;

    case HAVEN_DATETIME:
      switch (vendor_) {
      case HAVEN_SAS:
        return "DATETIME";
      case HAVEN_SPSS:
        return "DATETIME";
      case HAVEN_STATA:
        return "%tc";
      }
    case HAVEN_DATE:
      switch (vendor_) {
      case HAVEN_SAS:
        return "DATE";
      case HAVEN_SPSS:
        return "DATE";
      case HAVEN_STATA:
        return "%td";
      }
    case HAVEN_TIME:
      switch (vendor_) {
      case HAVEN_SAS:
        return "TIME";
      case HAVEN_SPSS:
        return "TIME";
      case HAVEN_STATA:
        return NULL; // Stata doesn't have a pure time type
      }
    }

    return NULL;
  }

  int userWidth(cpp11::sexp x) { return intAttribute(x, "width"); }

  // Storage types and widths
  // ---------------------------------------------------

  // Stata storage types are fixed width, so like Stata's `compress`, use the
  // smallest integer type that can exactly represent every value.
  readstat_type_t
  stataNumericType(cpp11::sexp col, readstat_type_t default_type) {
    if (ext_ != HAVEN_DTA)
      return default_type;
    // Datetimes are stored in milliseconds so need the precision of a double
    if (numType(col) == HAVEN_DATETIME)
      return READSTAT_TYPE_DOUBLE;

    // Compute the range of non-missing values to find the smallest integer
    // type that can hold them; a fractional value forces double
    double min = R_PosInf, max = R_NegInf;
    int n = Rf_length(col);
    for (int i = 0; i < n; ++i) {
      double val;
      switch (TYPEOF(col)) {
      case INTSXP:
        if (INTEGER(col)[i] == NA_INTEGER)
          continue;
        val = INTEGER(col)[i];
        break;
      default:
        val = REAL(col)[i];
        if (!R_finite(val))
          continue;
        break;
      }

      if (val != std::trunc(val))
        return READSTAT_TYPE_DOUBLE;
      min = std::min(min, val);
      max = std::max(max, val);
    }
    // Remaining adjustments are integer shifts, so only affect the range
    min = adjustDatetimeFromR(vendor_, col, min);
    max = adjustDatetimeFromR(vendor_, col, max);

    // Stata reserves the largest values of each type for missing values
    if (min >= -127 && max <= 100) {
      return READSTAT_TYPE_INT8;
    } else if (min >= -32767 && max <= 32740) {
      return READSTAT_TYPE_INT16;
    } else if (min >= -2147483647 && max <= 2147483620) {
      return READSTAT_TYPE_INT32;
    } else {
      return READSTAT_TYPE_DOUBLE;
    }
  }

  int stringWidth(
      cpp11::strings x, const char* name, readstat_label_set_t* labelSet) {
    int user_width = userWidth(x);
    int max_length = 1;
    for (int i = 0; i < x.size(); ++i) {
      int length = string_len_missing(x, i);
      if (length > max_length)
        max_length = length;
    }
    if (labelSet) {
      for (int i = 0; i < labelSet->value_labels_count; ++i) {
        int length = labelSet->value_labels[i].string_key_len;
        if (length > max_length)
          max_length = length;
      }
    }
    if (max_length > user_width) {
      if (user_width > 0) {
        cpp11::warning(
            "Column `%s` contains string values longer than user "
            "width %d. Width set to %d to accommodate.",
            name,
            user_width,
            max_length);
      }
      user_width = max_length;
    }
    return user_width;
  }

  // SPSS missing values
  // ---------------------------------------------------------

  void addSpssMissing(readstat_variable_t* var, cpp11::sexp x) {
    if (!Rf_inherits(x, "haven_labelled_spss"))
      return;

    SEXP na_range = x.attr("na_range");
    if (Rf_length(na_range) == 2) {
      if (TYPEOF(na_range) == REALSXP) {
        readstat_variable_add_missing_double_range(
            var, REAL(na_range)[0], REAL(na_range)[1]);
      } else if (TYPEOF(na_range) == INTSXP) {
        readstat_variable_add_missing_double_range(
            var, INTEGER(na_range)[0], INTEGER(na_range)[1]);
      } else if (TYPEOF(na_range) == STRSXP) {
        readstat_variable_add_missing_string_range(
            var,
            R_CHAR(STRING_ELT(na_range, 0)),
            R_CHAR(STRING_ELT(na_range, 1)));
      }
    }

    SEXP na_values = x.attr("na_values");
    int n = Rf_length(na_values);
    for (int i = 0; i < n; ++i) {
      if (TYPEOF(na_values) == REALSXP) {
        readstat_variable_add_missing_double_value(var, REAL(na_values)[i]);
      } else if (TYPEOF(na_values) == INTSXP) {
        readstat_variable_add_missing_double_value(var, INTEGER(na_values)[i]);
      } else if (TYPEOF(na_values) == STRSXP) {
        readstat_variable_add_missing_string_value(
            var, R_CHAR(STRING_ELT(na_values, i)));
      }
    }
  }
};

#endif
