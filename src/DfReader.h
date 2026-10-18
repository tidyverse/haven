#ifndef __DF_READER__
#define __DF_READER__

#include <map>
#include <set>
#include <string>
#include <vector>

#include "DfReaderLabelSet.h"
#include "haven_types.h"
#include "readstat.h"

#include "cpp11/doubles.hpp"
#include "cpp11/integers.hpp"
#include "cpp11/list.hpp"
#include "cpp11/r_string.hpp"
#include "cpp11/sexp.hpp"
#include "cpp11/strings.hpp"

#include "cpp11/function.hpp"
#include "cpp11/protect.hpp"

class DfReader {
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

public:
  DfReader(FileExt ext, bool user_na = false)
      : vendor_(extVendor(ext)),
        nrows_(0),
        ncols_(0),
        output_(static_cast<R_xlen_t>(0)),
        user_na_(user_na),
        quiet_(false) {}

  // Suppress metadata warnings, e.g. when only reading the header
  void setQuiet(bool quiet) { quiet_ = quiet; }

  void skipCols(cpp11::integers cols) {
    std::set<int> cols_set(cols.begin(), cols.end());
    colsSkip_ = cols_set;
  }

  void setInfo(int obs_count, int var_count) {
    if (obs_count < 0) {
      // If unknown, start with 1e5, and use doubling strategy
      nrowsAlloc_ = 1e5;
      nrows_ = 0;
    } else {
      nrowsAlloc_ = nrows_ = obs_count;
    }

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
      output_.attr("creation_timestamp") = timestamp(c_time);
    }
    if (m_time != 0) {
      output_.attr("modified_timestamp") = timestamp(m_time);
    }
  }

  cpp11::writable::doubles timestamp(time_t time) {
    cpp11::writable::doubles out({(double)time});
    out.attr("class") = {"POSIXct", "POSIXt"};
    out.attr("tzone") = "UTC";
    return out;
  }

  void setNote(int note_index, const char* note) {
    if (note != NULL && strcmp(note, "") != 0) {
      notes_.push_back(note);
    }
  }

  int createVariable(
      int index, readstat_variable_t* variable, const char* val_labels) {

    const char* name = readstat_variable_get_name(variable);

    if (colsSkip_.count(index) > 0) {
      return READSTAT_HANDLER_SKIP_VARIABLE;
    }

    int var_index = readstat_variable_get_index_after_skipping(variable);

    names_[var_index] = name;

    switch (readstat_variable_get_type(variable)) {
    case READSTAT_TYPE_STRING_REF:
    case READSTAT_TYPE_STRING:
      output_[var_index] = cpp11::writable::strings(nrowsAlloc_);
      break;
    case READSTAT_TYPE_INT8:
    case READSTAT_TYPE_INT16:
    case READSTAT_TYPE_INT32:
    case READSTAT_TYPE_FLOAT:
    case READSTAT_TYPE_DOUBLE:
      output_[var_index] = cpp11::writable::doubles(nrowsAlloc_);
      break;
    }

    cpp11::sexp col(output_[var_index]);

    const char* var_label = readstat_variable_get_label(variable);
    if (var_label != NULL && strcmp(var_label, "") != 0) {
      col.attr("label") = var_label;
    }

    if (val_labels != NULL)
      val_labels_[var_index] = val_labels;

    const char* var_format = readstat_variable_get_format(variable);

    VarType var_type = numType(vendor_, var_format);
    // Rcout << name << ": " << var_format << " [" << var_type << "]\n";
    var_types_[var_index] = var_type;
    // Files have been observed in the wild with a date format applied to a
    // string variable. This should not be possible, so we throw a warning,
    // don't apply a class and continue.
    if (var_type != HAVEN_DEFAULT &&
        readstat_variable_get_type_class(variable) ==
            READSTAT_TYPE_CLASS_STRING) {
      if (!quiet_)
        Rf_warning(
            "String variable '%s' has incompatible format '%s' and will "
            "be returned as a regular string variable.",
            name,
            var_format);
    } else {
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

    // User defined missing values
    int n_ranges = readstat_variable_get_missing_ranges_count(variable);
    if (user_na_ && n_ranges > 0) {
      switch (readstat_variable_get_type(variable)) {
      case READSTAT_TYPE_STRING_REF:
      case READSTAT_TYPE_STRING: {
        cpp11::writable::strings na_values(R_xlen_t(0));
        cpp11::writable::strings na_range(2);
        bool has_range = false;

        for (int i = 0; i < n_ranges; ++i) {
          readstat_value_t lo_value = readstat_variable_get_missing_range_lo(
                               variable, i),
                           hi_value = readstat_variable_get_missing_range_hi(
                               variable, i);

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
          readstat_value_t lo_value = readstat_variable_get_missing_range_lo(
                               variable, i),
                           hi_value = readstat_variable_get_missing_range_hi(
                               variable, i);

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

    // Store original format as attribute
    if (var_format != NULL && strcmp(var_format, "") != 0) {
      col.attr(formatAttribute(vendor_)) = var_format;
    }

    // Store original display width as attribute if it differs from the default
    int display_width = readstat_variable_get_display_width(variable);
    if (vendor_ == HAVEN_SPSS && display_width != 8) {
      col.attr("display_width") = Rf_ScalarInteger(display_width);
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

    int nNotes = notes_.size();
    if (nNotes > 0) {
      cpp11::writable::strings notes(nNotes);

      for (int i = 0; i < nNotes; ++i) {
        notes[i] = notes_[i].c_str();
      }

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

int dfreader_metadata(readstat_metadata_t* metadata, void* ctx) {
  ((DfReader*)ctx)
      ->setInfo(
          readstat_get_row_count(metadata), readstat_get_var_count(metadata));
  ((DfReader*)ctx)->setMetadata(readstat_get_file_label(metadata));
  ((DfReader*)ctx)
      ->setTimestamp(
          readstat_get_creation_time(metadata),
          readstat_get_modified_time(metadata));
  return 0;
}

int dfreader_note(int note_index, const char* note, void* ctx) {
  ((DfReader*)ctx)->setNote(note_index, note);
  return 0;
}

int dfreader_variable(
    int index,
    readstat_variable_t* variable,
    const char* val_labels,
    void* ctx) {
  return ((DfReader*)ctx)->createVariable(index, variable, val_labels);
}
int dfreader_value(
    int obs_index,
    readstat_variable_t* variable,
    readstat_value_t value,
    void* ctx) {
  // Check for user interrupts every 10,000 rows or cols
  if ((obs_index + 1) % 10000 == 0 || (variable->index + 1) % 10000 == 0)
    cpp11::check_user_interrupt();

  ((DfReader*)ctx)->setValue(obs_index, variable, value);
  return 0;
}
int dfreader_value_label(
    const char* val_labels,
    readstat_value_t value,
    const char* label,
    void* ctx) {
  ((DfReader*)ctx)->setValueLabels(val_labels, value, label);
  return 0;
}

void print_error(const char* error_message, void* ctx) {
  Rprintf("%s\n", error_message);
}

#endif
