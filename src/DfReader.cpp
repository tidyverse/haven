#include <Rcpp.h>
#include <fstream>
#include <iostream>
#include <sstream>

using namespace Rcpp;
#include "readstat.h"
#include "haven_types.h"
#include "tagged_na.h"

double haven_double_value_udm(readstat_value_t value, readstat_variable_t* var, bool user_na) {
  if (readstat_value_is_tagged_missing(value)) {
    return make_tagged_na(tolower(readstat_value_tag(value)));
  } else if (!user_na & readstat_value_is_defined_missing(value, var)) {
    return NA_REAL;
  } else if (readstat_value_is_system_missing(value)) {
    return NA_REAL;
  } else {
    return readstat_double_value(value);
  }
}

double haven_double_value(readstat_value_t value) {
  if (readstat_value_is_tagged_missing(value)) {
    return make_tagged_na(tolower(readstat_value_tag(value)));
  } else {
    return readstat_double_value(value);
  }
}


// LabelSet -------------------------------------------------------------------

class LabelSet {
  std::vector<std::string> labels_;
  std::vector<std::string> values_s_;
  std::vector<int> values_i_;
  std::vector<double> values_d_;

public:
  LabelSet() {}

  void add(const char* value, std::string label) {
    if (values_i_.size() > 0 || values_d_.size() > 0)
      stop("Can't add string to integer/double labelset");

    std::string string(value);
    values_s_.push_back(string);
    labels_.push_back(label);
  }

  void add(int value, std::string label) {
    if (values_d_.size() > 0 || values_s_.size() > 0)
      stop("Can't add integer to string/double labelset");

    values_i_.push_back(value);
    labels_.push_back(label);
  }

  void add(double value, std::string label) {
    if (values_i_.size() > 0 || values_s_.size() > 0)
      stop("Can't add double to integer/string labelset");

    values_d_.push_back(value);
    labels_.push_back(label);
  }

  size_t size() const {
    return labels_.size();
  }

  RObject labels() const {
    RObject out;

    if (values_i_.size() > 0) {
      int n = values_i_.size();
      IntegerVector values(n);
      CharacterVector labels(n);

      for (int i = 0; i < n; ++i) {
        values[i] = values_i_[i];
        labels[i] = Rf_mkCharCE(labels_[i].c_str(), CE_UTF8);
      }

      values.attr("names") = labels;
      out = values;
    } else if (values_d_.size() > 0) {
      int n = values_d_.size();
      NumericVector values(n);
      CharacterVector labels(n);

      for (int i = 0; i < n; ++i) {
        values[i] = values_d_[i];
        labels[i] = Rf_mkCharCE(labels_[i].c_str(), CE_UTF8);
      }

      values.attr("names") = labels;
      out = values;
    } else {
      int n = values_s_.size();
      CharacterVector values(n), labels(n);

      for (int i = 0; i < n; ++i) {
        values[i] = Rf_mkCharCE(values_s_[i].c_str(), CE_UTF8);
        labels[i] = Rf_mkCharCE(labels_[i].c_str(), CE_UTF8);
      }

      values.attr("names") = labels;
      out = values;
    }

    return out;
  }
};

// DfReader ------------------------------------------------------------------

class DfReader {
  FileType type_;
  int nrows_, nrowsAlloc_;
  int ncols_;
  List output_;
  CharacterVector names_;
  bool user_na_;

  std::vector<std::string> val_labels_;
  std::map<std::string, LabelSet> label_sets_;
  std::vector<VarType> var_types_;
  std::vector<std::string> notes_;

  // If empty, assume all will be read in
  std::set<std::string> colsOnly_;

public:
  DfReader(FileType type, bool user_na = false): type_(type), nrows_(0), ncols_(0), user_na_(user_na) {
  }

  void restrictCols(const std::set<std::string>& cols) {
    colsOnly_ = cols;
  }

  void setInfo(int obs_count, int var_count) {
    if (obs_count < 0) {
      // If unknown, start with 1e5, and use doubling strategy
      nrowsAlloc_ = 1e5;
      nrows_ = 0;
    } else {
      nrowsAlloc_ = nrows_ = obs_count;
    }

    ncols_ = colsOnly_.empty() ? var_count : colsOnly_.size();

    output_ = List(ncols_);
    names_ = CharacterVector(ncols_);
    val_labels_.resize(ncols_);
    var_types_.resize(ncols_);
  }

  void setMetadata(const char *file_label, time_t timestamp, long format_version) {
    if (file_label != NULL && strcmp(file_label, "") != 0) {
      output_.attr("label") = CharacterVector::create(Rf_mkCharCE(file_label, CE_UTF8));
    }
  }

  void setNote(int note_index, const char *note) {
    if (note != NULL && strcmp(note, "") != 0) {
      notes_.push_back(note);
    }
  }

  int createVariable(int index, readstat_variable_t *variable, const char *val_labels) {

    const char* name = readstat_variable_get_name(variable);

    if (!colsOnly_.empty() && colsOnly_.count(name) == 0) {
      return READSTAT_HANDLER_SKIP_VARIABLE;
    }

    int var_index = readstat_variable_get_index_after_skipping(variable);

    names_[var_index] = Rf_mkCharCE(name, CE_UTF8);

    switch(readstat_variable_get_type(variable)) {
    case READSTAT_TYPE_STRING_REF:
    case READSTAT_TYPE_STRING:
      output_[var_index] = CharacterVector(nrowsAlloc_);
      break;
    case READSTAT_TYPE_INT8:
    case READSTAT_TYPE_INT16:
    case READSTAT_TYPE_INT32:
    case READSTAT_TYPE_FLOAT:
    case READSTAT_TYPE_DOUBLE:
      output_[var_index] = NumericVector(nrowsAlloc_);
      break;
    }

    RObject col = output_[var_index];

    const char* var_label = readstat_variable_get_label(variable);
    if (var_label != NULL && strcmp(var_label, "") != 0) {
      col.attr("label") = CharacterVector::create(Rf_mkCharCE(var_label, CE_UTF8));
    }

    if (val_labels != NULL)
      val_labels_[var_index] = val_labels;

    const char* var_format = readstat_variable_get_format(variable);

    VarType var_type = numType(type_, var_format);
    // Rcout << var_name << ": " << var_format << " [" << var_type << "]\n";
    var_types_[var_index] = var_type;
    switch(var_type) {
    case HAVEN_DATE:
      col.attr("class") = "Date";
      break;
    case HAVEN_TIME:
      col.attr("class") = CharacterVector::create("hms", "difftime");
      col.attr("units") = "secs";
      break;
    case HAVEN_DATETIME:
      col.attr("class") = CharacterVector::create("POSIXct", "POSIXt");
      col.attr("tzone") = "UTC";
      break;
    default:
      break;
    }

    // User defined missing values
    int n_ranges = readstat_variable_get_missing_ranges_count(variable);
    if (user_na_ && n_ranges > 0) {
      switch(readstat_variable_get_type(variable)) {
      case READSTAT_TYPE_STRING_REF:
      case READSTAT_TYPE_STRING:
      {
        CharacterVector na_values(n_ranges);

        for (int i = 0; i < n_ranges; ++i) {
          readstat_value_t value = readstat_variable_get_missing_range_lo(variable, i);
          const char* str_value = readstat_string_value(value);
          na_values[0] = str_value == NULL ? NA_STRING : Rf_mkCharCE(str_value, CE_UTF8);
        }

        col.attr("na_values") = na_values;
        break;
      }
      case READSTAT_TYPE_INT8:
      case READSTAT_TYPE_INT16:
      case READSTAT_TYPE_INT32:
      case READSTAT_TYPE_FLOAT:
      case READSTAT_TYPE_DOUBLE:
      {
        std::vector<double> na_values;
        NumericVector na_range(2);
        bool has_range = false;

        for (int i = 0; i < n_ranges; ++i) {
          readstat_value_t
            lo_value = readstat_variable_get_missing_range_lo(variable, i),
            hi_value = readstat_variable_get_missing_range_hi(variable, i);

          double
            lo = readstat_double_value(lo_value),
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
          col.attr("na_values") = Rcpp::wrap(na_values);
        if (has_range)
          col.attr("na_range") = na_range;

      }
      }
      col.attr("class") = CharacterVector::create("labelled_spss", "labelled");
    }


    // Store original format as attribute
    if (var_format != NULL && strcmp(var_format, "") != 0) {
      col.attr(formatAttribute(type_)) = Rf_ScalarString(Rf_mkCharCE(var_format, CE_UTF8));
    }

    // Store original display width as attribute if it differs from the default
    int display_width = readstat_variable_get_display_width(variable);
    if (type_ == HAVEN_SPSS && display_width != 8) {
      col.attr("display_width") = Rf_ScalarInteger(display_width);
    }

    return READSTAT_HANDLER_OK;
  }

  void setValue(int obs_index, readstat_variable_t *variable, readstat_value_t value) {
    int var_index = readstat_variable_get_index_after_skipping(variable);

    VarType var_type = var_types_[var_index];

    if (obs_index >= nrowsAlloc_)
      resizeCols(nrowsAlloc_ * 2);
    if (obs_index >= nrows_)
      nrows_ = obs_index + 1;

    switch(value.type) {
    case READSTAT_TYPE_STRING_REF:
    case READSTAT_TYPE_STRING:
    {
      CharacterVector col = output_[var_index];

      const char* str_value = readstat_string_value(value);
      col[obs_index] = str_value == NULL ? NA_STRING : Rf_mkCharCE(str_value, CE_UTF8);
      break;
    }
    case READSTAT_TYPE_INT8:
    case READSTAT_TYPE_INT16:
    case READSTAT_TYPE_INT32:
    case READSTAT_TYPE_FLOAT:
    case READSTAT_TYPE_DOUBLE:
    {
      NumericVector col = output_[var_index];
      double val = haven_double_value_udm(value, variable, user_na_);
      col[obs_index] = adjustDatetimeToR(type_, var_type, val);
      break;
    }
    }
  }

  void setValueLabels(const char *val_labels, readstat_value_t value,
                     const char *label) {
    LabelSet& label_set = label_sets_[val_labels];
    std::string label_s(label);

    switch(value.type) {
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
      Rf_warning("Unsupported label type: %s", value.type);
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
      output_[i] = Rf_lengthgets(output_[i], n);
    }
  }

  List output() {
    if (nrows_ != nrowsAlloc_)
      resizeCols(nrows_);

    for (int i = 0; i < output_.size(); ++i) {
      RObject col = output_[i];

      if (hasLabel(i)) {
        if (col.attr("class") == R_NilValue) {
          col.attr("class") = "labelled";
        }
        col.attr("labels") = label_sets_[val_labels_[i]].labels();
      }
    }

    int nNotes = notes_.size();
    if (nNotes > 0) {
      CharacterVector notes(nNotes);
      for (int i = 0; i < nNotes; ++i) {
        notes[i] = Rf_mkCharCE(notes_[i].c_str(), CE_UTF8);
      }

      output_.attr("notes") = notes_;
    }

    output_.attr("names") = names_;

    static Function as_tibble("as_tibble", Environment::namespace_env("tibble"));
    return as_tibble(output_);
  }

};

int dfreader_info(int obs_count, int var_count, void *ctx) {
  ((DfReader*) ctx)->setInfo(obs_count, var_count);
  return 0;
}

int dfreader_metadata(const char *file_label, time_t timestamp, long format_version, void *ctx) {
  ((DfReader*) ctx)->setMetadata(file_label, timestamp, format_version);
  return 0;
}

int dfreader_note(int note_index, const char *note, void *ctx) {
  ((DfReader*) ctx)->setNote(note_index, note);
  return 0;
}

int dfreader_variable(int index, readstat_variable_t *variable,
                      const char *val_labels, void *ctx) {
  return ((DfReader*) ctx)->createVariable(index, variable, val_labels);
}
int dfreader_value(int obs_index, readstat_variable_t *variable,
                   readstat_value_t value, void *ctx) {
  // Check for user interrupts every 10,000 rows or cols
  if ((obs_index + 1) % 10000 == 0 || (variable->index + 1) % 10000 == 0)
    checkUserInterrupt();

  ((DfReader*) ctx)->setValue(obs_index, variable, value);
  return 0;
}
int dfreader_value_label(const char *val_labels, readstat_value_t value,
                         const char *label, void *ctx) {
  ((DfReader*) ctx)->setValueLabels(val_labels, value, label);
  return 0;
}

void print_error(const char* error_message, void* ctx) {
  Rcout << error_message << "\n";
}


// IO handling -----------------------------------------------------------

class DfReaderInput {
public:
  virtual ~DfReaderInput() {};

  virtual int open(void* io_ctx) = 0;
  virtual int close(void* io_ctx) = 0;
  virtual readstat_off_t seek(readstat_off_t offset, readstat_io_flags_t whence, void *io_ctx) = 0;
  virtual ssize_t read(void *buf, size_t nbyte, void *io_ctx) = 0;
};

template <typename Stream>
class DfReaderInputStream : public DfReaderInput {
protected:
  Stream file_;

public:
  readstat_off_t seek(readstat_off_t offset, readstat_io_flags_t whence, void *io_ctx) {
    std::ios_base::seekdir dir;
    switch(whence) {
    case READSTAT_SEEK_SET: dir = file_.beg; break;
    case READSTAT_SEEK_CUR: dir = file_.cur; break;
    case READSTAT_SEEK_END: dir = file_.end; break;
    }

    file_.seekg(offset, dir);
    return file_.tellg(); // returns -1 if failed
  }

  ssize_t read(void *buf, size_t nbyte, void *io_ctx) {
    file_.read((char*) buf, nbyte);
    return (file_.good() || file_.eof()) ? file_.gcount() : -1;
  }
};

class DfReaderInputFile : public DfReaderInputStream<std::ifstream> {
  std::string filename_;

public:
  DfReaderInputFile(Rcpp::List spec) {
    filename_ = as<std::string>(spec[0]);
  }

  int open(void* io_ctx) {
    file_.open(filename_.c_str(), std::ifstream::binary);
    return file_.is_open() ? 0 : -1;
  }

  int close(void* io_ctx) {
    file_.close();
    return file_.is_open() ? -1 : 0;
  }
};

class DfReaderInputRaw : public DfReaderInputStream<std::istringstream> {
public:
  DfReaderInputRaw(Rcpp::List spec) {
    Rcpp::RawVector raw_data(spec[0]);
    std::string string_data((char*) RAW(raw_data), Rf_length(raw_data));
    file_.str(string_data);
  }

  int open(void* io_ctx) {
    return 0;
  }

  int close(void* io_ctx) {
    return 0;
  }
};


int dfreader_open(const char* path, void *io_ctx) {
  return ((DfReaderInput*) io_ctx)->open(io_ctx);
}
int dfreader_close(void *io_ctx) {
  return ((DfReaderInput*) io_ctx)->close(io_ctx);
}
readstat_off_t dfreader_seek(readstat_off_t offset, readstat_io_flags_t whence, void* io_ctx) {
  return ((DfReaderInput*) io_ctx)->seek(offset, whence, io_ctx);
}
ssize_t dfreader_read(void* buf, size_t nbyte, void* io_ctx) {
  return ((DfReaderInput*) io_ctx)->read(buf, nbyte, io_ctx);
}
readstat_error_t dfreader_update(long file_size, readstat_progress_handler progress_handler, void *user_ctx, void *io_ctx) {
  return READSTAT_OK;
}

// Parser wrappers -------------------------------------------------------------

readstat_parser_t* haven_init_parser(std::string encoding = "") {
  readstat_parser_t* parser = readstat_parser_init();
  readstat_set_info_handler(parser, dfreader_info);
  readstat_set_metadata_handler(parser, dfreader_metadata);
  readstat_set_note_handler(parser, dfreader_note);
  readstat_set_variable_handler(parser, dfreader_variable);
  readstat_set_value_handler(parser, dfreader_value);
  readstat_set_value_label_handler(parser, dfreader_value_label);
  readstat_set_error_handler(parser, print_error);
  if (encoding != "") {
    readstat_set_file_character_encoding(parser, encoding.c_str());
  }
  return parser;
}

template<typename InputClass>
void haven_init_io(readstat_parser_t* parser, InputClass &builder_input) {
  readstat_set_open_handler(parser, dfreader_open);
  readstat_set_close_handler(parser, dfreader_close);
  readstat_set_seek_handler(parser, dfreader_seek);
  readstat_set_read_handler(parser, dfreader_read);
  readstat_set_update_handler(parser, dfreader_update);
  readstat_set_io_ctx(parser, (void*) &builder_input);
}

std::string haven_error_message(Rcpp::List spec) {
  std::string source_class(as<CharacterVector>(spec.attr("class"))[0]);
  if (source_class == "source_raw")
    return "file";
  else
    return as<std::string>(spec[0]);
}

template<typename InputClass>
List df_parse_spss(Rcpp::List spec, bool user_na = false, bool por = false) {
  DfReader builder(HAVEN_SPSS, user_na);
  InputClass builder_input(spec);

  readstat_parser_t* parser = haven_init_parser();
  haven_init_io(parser, builder_input);

  readstat_error_t result;
  if (por) {
    result = readstat_parse_por(parser, "", &builder);
  } else {
    result = readstat_parse_sav(parser, "", &builder);
  }

  readstat_parser_free(parser);

  if (result != 0) {
    stop("Failed to parse %s: %s.", haven_error_message(spec),
         readstat_error_message(result));
  }

  return builder.output();
}

template<typename InputClass>
List df_parse_dta(Rcpp::List spec, std::string encoding = "") {
  DfReader builder(HAVEN_STATA);
  InputClass builder_input(spec);

  readstat_parser_t* parser = haven_init_parser(encoding);
  haven_init_io(parser, builder_input);

  readstat_error_t result = readstat_parse_dta(parser, "", &builder);
  readstat_parser_free(parser);

  if (result != 0) {
    stop("Failed to parse %s: %s.",
         haven_error_message(spec),
         readstat_error_message(result));
  }

  return builder.output();
}


template<typename InputClass>
List df_parse_xpt(Rcpp::List spec, std::string encoding = "") {
  DfReader builder(HAVEN_XPT);
  InputClass builder_input(spec);

  readstat_parser_t* parser = haven_init_parser();
  haven_init_io(parser, builder_input);

  readstat_error_t result = readstat_parse_xport(parser, "", &builder);
  readstat_parser_free(parser);

  if (result != 0) {
    stop("Failed to parse %s: %s.",
         haven_error_message(spec),
         readstat_error_message(result));
  }

  return builder.output();
}


template<typename InputClass>
List df_parse_sas(Rcpp::List spec_b7dat, Rcpp::List spec_b7cat,
                  std::string encoding, std::vector<std::string> cols_only) {
  DfReader builder(HAVEN_SAS);

  if (!cols_only.empty()) {
    std::set<std::string> cols_set(cols_only.begin(), cols_only.end());
    builder.restrictCols(cols_set);
  }

  InputClass builder_input_dat(spec_b7dat);

  readstat_parser_t* parser = haven_init_parser(encoding);
  haven_init_io(parser, builder_input_dat);

  if (spec_b7cat.size() != 0) {
    InputClass builder_input_cat(spec_b7cat);
    readstat_set_io_ctx(parser, (void*) &builder_input_cat);

    readstat_error_t result = readstat_parse_sas7bcat(parser, "", &builder);

    if (result != 0) {
      readstat_parser_free(parser);
      stop("Failed to parse %s: %s.", haven_error_message(spec_b7cat),
           readstat_error_message(result));
    }
  }

  readstat_set_io_ctx(parser, (void*) &builder_input_dat);
  readstat_error_t result = readstat_parse_sas7bdat(parser, "", &builder);
  readstat_parser_free(parser);

  if (result != 0) {
    stop("Failed to parse %s: %s.", haven_error_message(spec_b7dat),
         readstat_error_message(result));
  }

  return builder.output();
}

// [[Rcpp::export]]
List df_parse_sas_file(Rcpp::List spec_b7dat, Rcpp::List spec_b7cat,
                       std::string encoding, std::vector<std::string> cols_only) {
  return df_parse_sas<DfReaderInputFile>(spec_b7dat, spec_b7cat, encoding, cols_only);
}
// [[Rcpp::export]]
List df_parse_sas_raw(Rcpp::List spec_b7dat, Rcpp::List spec_b7cat,
                      std::string encoding, std::vector<std::string> cols_only) {
  return df_parse_sas<DfReaderInputRaw>(spec_b7dat, spec_b7cat, encoding, cols_only);
}

// [[Rcpp::export]]
List df_parse_xpt_file(Rcpp::List spec) {
  return df_parse_xpt<DfReaderInputFile>(spec);
}
// [[Rcpp::export]]
List df_parse_xpt_raw(Rcpp::List spec) {
  return df_parse_xpt<DfReaderInputRaw>(spec);
}

// [[Rcpp::export]]
List df_parse_dta_file(Rcpp::List spec, std::string encoding) {
  return df_parse_dta<DfReaderInputFile>(spec, encoding);
}
// [[Rcpp::export]]
List df_parse_dta_raw(Rcpp::List spec, std::string encoding) {
  return df_parse_dta<DfReaderInputRaw>(spec, encoding);
}

// [[Rcpp::export]]
List df_parse_sav_file(Rcpp::List spec, bool user_na) {
  return df_parse_spss<DfReaderInputFile>(spec, user_na, false);
}
// [[Rcpp::export]]
List df_parse_sav_raw(Rcpp::List spec, bool user_na) {
  return df_parse_spss<DfReaderInputRaw>(spec, user_na, false);
}

// [[Rcpp::export]]
List df_parse_por_file(Rcpp::List spec, bool user_na) {
  return df_parse_spss<DfReaderInputFile>(spec, user_na, true);
}
// [[Rcpp::export]]
List df_parse_por_raw(Rcpp::List spec, bool user_na) {
  return df_parse_spss<DfReaderInputRaw>(spec, user_na, true);
}
