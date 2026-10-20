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
  FileExt ext_;
  FileVendor vendor_;

  int nrows_;      // number of rows read so far
  int nrowsAlloc_; // number of rows currently allocated in output vectors
  int ncols_;
  cpp11::writable::list output_;
  cpp11::writable::strings names_;
  bool user_na_;

  std::vector<std::string> val_labels_;
  std::map<std::string, LabelSet> label_sets_;
  std::vector<VarType> var_types_;
  std::vector<std::string> notes_;

  // File-level metadata
  time_t created_, modified_;
  std::string table_name_;
  int version_;
  cpp11::sexp mr_sets_;

  cpp11::sexp var_proto_, df_proto_;

  std::set<int> colsSkip_;
  bool quiet_;
  long n_max_;

public:
  Reader(FileExt ext, bool user_na = false)
      : ext_(ext),
        vendor_(extVendor(ext)),
        nrows_(0),
        nrowsAlloc_(0),
        ncols_(0),
        output_(static_cast<R_xlen_t>(0)),
        user_na_(user_na),
        created_(0),
        modified_(0),
        version_(0),
        mr_sets_(R_NilValue),
        quiet_(false),
        n_max_(-1) {
    cpp11::function meta_proto = cpp11::package("haven")["meta_proto"];
    var_proto_ = meta_proto(vendorName(vendor_), false);
    df_proto_ = meta_proto(vendorName(vendor_), true);
  }

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

    if (var_count < 1) {
      return; // sas7bcat has var_count = 0
    }

    ncols_ = var_count - colsSkip_.size();

    output_.resize(ncols_);
    names_.resize(ncols_);
    val_labels_.resize(ncols_);
    var_types_.resize(ncols_);
  }

  void setMetadata(readstat_metadata_t* metadata) {
    const char* file_label = readstat_get_file_label(metadata);
    if (!is_empty(file_label)) {
      output_.attr("label") = file_label;
    }

    // Called for both catalog and data file; the data file comes last so wins
    created_ = readstat_get_creation_time(metadata);
    modified_ = readstat_get_modified_time(metadata);

    const char* table_name = readstat_get_table_name(metadata);
    table_name_ = is_empty(table_name) ? "" : table_name;

    version_ = readstat_get_file_format_version(metadata);

    size_t n_mr_sets = readstat_get_multiple_response_sets_length(metadata);
    if (n_mr_sets > 0) {
      mr_sets_ =
          mrSets(readstat_get_multiple_response_sets(metadata), n_mr_sets);
    }
  }

  cpp11::writable::list mrSets(const mr_set_t* mr_sets, size_t n) {
    using namespace cpp11::literals;
    cpp11::writable::list out(n);
    cpp11::writable::strings names(n);

    for (size_t i = 0; i < n; ++i) {
      const mr_set_t& mr_set = mr_sets[i];
      names[i] = mr_set.name;

      cpp11::writable::strings variables(mr_set.num_subvars);
      for (int j = 0; j < mr_set.num_subvars; ++j) {
        variables[j] = mr_set.subvariables[j];
      }

      cpp11::writable::list set(
          {"label"_nm = mr_set.label == NULL ? "" : mr_set.label,
           "type"_nm = mr_set.is_dichotomy ? "dichotomy" : "category",
           "counted_value"_nm =
               mr_set.counted_string == NULL
                   ? cpp11::sexp(R_NilValue)
                   : cpp11::sexp(
                         cpp11::as_sexp(std::string(mr_set.counted_string))),
           "variables"_nm = variables});
      out[i] = set;
    }

    out.attr("names") = names;
    return out;
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

    SEXP meta = varMeta(variable);
    if (meta != R_NilValue) {
      col.attr("haven_meta") = meta;
    }

    return READSTAT_HANDLER_OK;
  }

  SEXP varMeta(readstat_variable_t* variable) {
    MetaBuilder meta(var_proto_);

    const char* format = readstat_variable_get_format(variable);
    if (!is_empty(format)) {
      meta.set("format", format);
    }

    readstat_type_class_t type_class =
        readstat_variable_get_type_class(variable);
    int width = readstat_variable_get_storage_width(variable);

    switch (vendor_) {
    case HAVEN_SAS: {
      const char* informat = readstat_variable_get_informat(variable);
      if (!is_empty(informat)) {
        meta.set("informat", informat);
      }
      // For XPT files, display_width is the format width (namestr nfl), which
      // is what SAS users think of as the variable width. sas7bdat doesn't
      // set display_width, so fall back to the storage width.
      int display_width = readstat_variable_get_display_width(variable);
      if (display_width > 0) {
        meta.set("width", display_width);
      } else if (width > 0) {
        meta.set("width", width);
      }
      break;
    }
    case HAVEN_SPSS: {
      int display_width = readstat_variable_get_display_width(variable);
      if (display_width > 0) {
        meta.set("display_width", display_width);
      }
      // Numeric variables are always 8 bytes
      if (type_class == READSTAT_TYPE_CLASS_STRING && width > 0) {
        meta.set("width", width);
      }
      switch (readstat_variable_get_measure(variable)) {
      case READSTAT_MEASURE_NOMINAL:
        meta.set("measure", "nominal");
        break;
      case READSTAT_MEASURE_ORDINAL:
        meta.set("measure", "ordinal");
        break;
      case READSTAT_MEASURE_SCALE:
        meta.set("measure", "scale");
        break;
      default:
        break;
      }
      switch (readstat_variable_get_alignment(variable)) {
      case READSTAT_ALIGNMENT_LEFT:
        meta.set("alignment", "left");
        break;
      case READSTAT_ALIGNMENT_CENTER:
        meta.set("alignment", "center");
        break;
      case READSTAT_ALIGNMENT_RIGHT:
        meta.set("alignment", "right");
        break;
      default:
        break;
      }
      break;
    }
    case HAVEN_STATA: {
      switch (readstat_variable_get_type(variable)) {
      case READSTAT_TYPE_INT8:
        meta.set("type", "byte");
        break;
      case READSTAT_TYPE_INT16:
        meta.set("type", "int");
        break;
      case READSTAT_TYPE_INT32:
        meta.set("type", "long");
        break;
      case READSTAT_TYPE_FLOAT:
        meta.set("type", "float");
        break;
      case READSTAT_TYPE_DOUBLE:
        meta.set("type", "double");
        break;
      // ReadStat reports strLs as strings with no storage width
      case READSTAT_TYPE_STRING:
        if (width == 0) {
          meta.set("type", "strL");
        }
        break;
      default:
        break;
      }
      break;
    }
    }

    return meta.get();
  }

  SEXP dfMeta() {
    MetaBuilder meta(df_proto_);

    if (created_ != 0) {
      meta.set("created", make_timestamp(created_));
    }
    if (modified_ != 0) {
      meta.set("modified", make_timestamp(modified_));
    }

    switch (vendor_) {
    case HAVEN_SAS:
      if (table_name_ != "") {
        meta.set("table_name", table_name_);
      }
      if (ext_ == HAVEN_XPT && (version_ == 5 || version_ == 8)) {
        meta.set("xpt_version", version_);
      }
      break;
    case HAVEN_SPSS:
      if (notes_.size() > 0) {
        meta.set("notes", notes());
      }
      if (mr_sets_ != R_NilValue) {
        meta.set("mr_sets", mr_sets_);
      }
      break;
    case HAVEN_STATA: {
      if (notes_.size() > 0) {
        meta.set("notes", notes());
      }
      int version = stata_version(version_);
      if (version != 0) {
        meta.set("version", version);
      }
      break;
    }
    }

    return meta.get();
  }

  cpp11::writable::strings notes() {
    int n = notes_.size();
    cpp11::writable::strings out(n);
    for (int i = 0; i < n; ++i) {
      out[i] = notes_[i].c_str();
    }
    return out;
  }

  void setValue(
      int obs_index, readstat_variable_t* variable, readstat_value_t value) {
    int var_index = readstat_variable_get_index_after_skipping(variable);

    VarType var_type = var_types_[var_index];

    // Grow by doubling, always ensuring progress
    if (obs_index >= nrowsAlloc_)
      resizeCols((obs_index + 1) * 2);
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

    SEXP meta = dfMeta();
    if (meta != R_NilValue) {
      output_.attr("haven_meta") = meta;
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
