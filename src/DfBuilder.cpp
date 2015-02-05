#include <Rcpp.h>
using namespace Rcpp;
#include "readstat.h"
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>

// typedef int (*readstat_handle_value_label_callback)(const char *val_labels,
//   readstat_value_t value, readstat_types_t type, const char *label, void *ctx);

class LabelSet {
  std::vector<std::string> labels;
  std::vector<std::string> values;

public:
  LabelSet() {}

  void add(std::string label, std::string value) {
    labels.push_back(label);
    values.push_back(value);
  }

  int find_label(std::string label) {
    std::vector<std::string>::iterator it =
      find(labels.begin(), labels.end(), label);

    return (it == labels.end()) ? -1 : (it - labels.begin());
  }

};
typedef boost::shared_ptr<LabelSet> LabelSetPtr;

class DfBuilder {
  int nrows_, ncols_;
  List output_;
  CharacterVector names_, labels_, val_labels_;
  std::map<std::string, LabelSetPtr> label_sets_;

public:
  DfBuilder(): nrows_(0), ncols_(0) {
  }

  int info(int obs_count, int var_count) {
    nrows_ = obs_count;
    ncols_ = var_count;

    output_ = List(ncols_);
    names_ = CharacterVector(ncols_);
    labels_ = CharacterVector(ncols_);
    val_labels_ = CharacterVector(ncols_);
    return 0;
  }

  int variable(int index, const char *var_name,
               const char *var_format,
               const char *var_label,
               const char *val_labels,
               readstat_types_t type, size_t max_len) {

    names_[index] = var_name;

    switch(type) {
    case READSTAT_TYPE_LONG_STRING:
    case READSTAT_TYPE_STRING:
    case READSTAT_TYPE_CHAR:
      output_[index] = CharacterVector(nrows_);
      break;
    case READSTAT_TYPE_INT16:
    case READSTAT_TYPE_INT32:
      output_[index] = IntegerVector(nrows_);
      break;
    case READSTAT_TYPE_FLOAT:
    case READSTAT_TYPE_DOUBLE:
      output_[index] = NumericVector(nrows_);
      break;
    }

    if (var_label != NULL) {
      labels_[index] = var_label;

      RObject col = output_[index];
      col.attr("label") = var_label;
    }

    if (val_labels != NULL)
      val_labels_[index] = val_labels;

    return 0;
  }

  int value(int obs_index, int var_index, void *value,
            readstat_types_t type) {

    if (type == READSTAT_TYPE_LONG_STRING || type == READSTAT_TYPE_STRING) {
      // Missing strings and "" are identical in other systems
      CharacterVector col = output_[var_index];
      col[obs_index] = readstat_string_value(value);
    } else if (type == READSTAT_TYPE_CHAR) {
      CharacterVector col = output_[var_index];
      col[obs_index] = readstat_char_value(value);
    } else if (type == READSTAT_TYPE_INT16) {
      IntegerVector col = output_[var_index];
      if (readstat_value_is_missing(value)) {
        col[obs_index] = NA_INTEGER;
      } else {
        col[obs_index] = readstat_int16_value(value);
      }
    } else if (type == READSTAT_TYPE_INT32) {
      IntegerVector col = output_[var_index];
      if (readstat_value_is_missing(value)) {
        col[obs_index] = NA_INTEGER;
      } else {
        col[obs_index] = readstat_int32_value(value);
      }
    } else if (type == READSTAT_TYPE_FLOAT) {
      NumericVector col = output_[var_index];
      if (readstat_value_is_missing(value)) {
        col[obs_index] = NA_REAL;
      } else {
        col[obs_index] = readstat_float_value(value);
      }
    } else if (type == READSTAT_TYPE_DOUBLE) {
      NumericVector col = output_[var_index];
      if (readstat_value_is_missing(value)) {
        col[obs_index] = NA_REAL;
      } else {
        col[obs_index] = readstat_double_value(value);
      }
    }

    return 0;
  }

  int value_label(const char *val_labels, readstat_value_t value,
                  readstat_types_t type, const char *label) {

    return 0;
  }

  List output() {
    output_.attr("names") = names_;
    output_.attr("class") = CharacterVector::create("tbl_df", "tbl", "data.frame");
    output_.attr("row.names") = IntegerVector::create(NA_INTEGER, -nrows_);

    return output_;
  }

};

int dfbuilder_info(int obs_count, int var_count, void *ctx) {
  return ((DfBuilder*) ctx)->info(obs_count, var_count);
}
int dfbuilder_variable(int index, const char *var_name, const char *var_format,
                       const char *var_label, const char *val_labels,
                       readstat_types_t type, size_t max_len,
                       void *ctx) {
  return ((DfBuilder*) ctx)->variable(index, var_name, var_format, var_label,
    val_labels, type, max_len);
}
int dfbuilder_value(int obs_index, int var_index, void *value,
                    readstat_types_t type, void *ctx) {
  return ((DfBuilder*) ctx)->value(obs_index, var_index, value, type);
}
int dfbuilder_value_label(const char *val_labels, readstat_value_t value,
                          readstat_types_t type, const char *label, void *ctx) {
  return ((DfBuilder*) ctx)->value_label(val_labels, value, type, label);
}

// Parser wrappers -------------------------------------------------------------

typedef boost::function<readstat_error_t(const char*, DfBuilder*)> Parser;

List df_parse(std::string filename, Parser parser) {
  DfBuilder builder;
  readstat_error_t result = parser(filename.c_str(), &builder);

  if (result != 0) {
    stop("Failed to parse %s: %s.", filename, readstat_error_message(result));
  }

  return builder.output();
}

// [[Rcpp::export]]
List df_parse_sas(std::string filename) {
  return df_parse(filename, boost::bind(parse_sas7bdat,
    _1, _2,
    dfbuilder_info,
    dfbuilder_variable,
    dfbuilder_value));
}


// [[Rcpp::export]]
List df_parse_dta(std::string filename) {
  return df_parse(filename, boost::bind(parse_dta,
    _1, _2,
    dfbuilder_info,
    dfbuilder_variable,
    dfbuilder_value,
    dfbuilder_value_label));
}

// [[Rcpp::export]]
List df_parse_por(std::string filename) {
  return df_parse(filename, boost::bind(parse_por,
    _1, _2,
    dfbuilder_info,
    dfbuilder_variable,
    dfbuilder_value,
    dfbuilder_value_label));
}

// [[Rcpp::export]]
List df_parse_sav(std::string filename) {
  return df_parse(filename, boost::bind(parse_sav,
    _1, _2,
    dfbuilder_info,
    dfbuilder_variable,
    dfbuilder_value,
    dfbuilder_value_label));
}
