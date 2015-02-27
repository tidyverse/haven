#include <Rcpp.h>
using namespace Rcpp;
#include "readstat.h"
#include "haven_types.h"

class LabelSet {
  std::vector<std::string> labels_;
  std::vector<std::string> values_s_;
  std::vector<double> values_d_;

public:
  LabelSet() {}

  void add(char* value, std::string label) {
    if (values_d_.size() > 0)
      stop("Can't add string to integer labelset");

    std::string string(value);
    values_s_.push_back(string);
    labels_.push_back(label);
  }

  void add(double value, std::string label) {
    if (values_s_.size() > 0)
      stop("Can't add integer to string labelset");

    values_d_.push_back(value);
    labels_.push_back(label);
  }

  RObject labels() const {
    RObject out;

    if (values_d_.size() > 0) {
      int n = values_d_.size();
      IntegerVector values(n);
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

class DfBuilder {
  FileType type_;
  int nrows_, ncols_;
  List output_;
  CharacterVector names_;

  std::vector<std::string> val_labels_;
  std::map<std::string, LabelSet> label_sets_;
  std::vector<VarType> var_types_;

public:
  DfBuilder(FileType type): type_(type), nrows_(0), ncols_(0) {
  }

  int info(int obs_count, int var_count) {
    nrows_ = obs_count;
    ncols_ = var_count;

    output_ = List(ncols_);
    names_ = CharacterVector(ncols_);
    val_labels_.resize(ncols_);
    var_types_.resize(ncols_);
    return 0;
  }

  int variable(int index, const char *var_name,
               const char *var_format,
               const char *var_label,
               const char *val_labels,
               readstat_types_t type) {

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

    RObject col = output_[index];
    if (var_label != NULL && strcmp(var_label, "") != 0) {
      col.attr("label") = CharacterVector::create(Rf_mkCharCE(var_label, CE_UTF8));
    }

    if (val_labels != NULL)
      val_labels_[index] = val_labels;

    VarType var_type = numType(type_, var_format);
    // Rcout << var_name << ": " << var_format << " [" << var_type << "]\n";
    var_types_[index] = var_type;
    switch(var_type) {
    case HAVEN_DATE:
      col.attr("class") = "Date";
      break;
    case HAVEN_TIME:
      col.attr("class") = "hms";
      break;
    case HAVEN_DATETIME:
      col.attr("class") = CharacterVector::create("POSIXct", "POSIXt");
      col.attr("tzone") = "UTC";
      break;
    default:
      break;
    }

    return 0;
  }

  int value(int obs_index, int var_index, void *value,
            readstat_types_t type) {

    // Check for user interrupts every 1000 rows or cols
    if (obs_index % 1000 == 0 || var_index % 1000 == 0)
      checkUserInterrupt();

    VarType var_type = var_types_[var_index];

    if (type == READSTAT_TYPE_LONG_STRING || type == READSTAT_TYPE_STRING) {
      // Missing strings and "" are identical in other systems
      CharacterVector col = output_[var_index];
      col[obs_index] = Rf_mkCharCE(readstat_string_value(value), CE_UTF8);
    } else if (type == READSTAT_TYPE_CHAR) {
      CharacterVector col = output_[var_index];
      col[obs_index] = readstat_char_value(value);
    } else if (type == READSTAT_TYPE_INT16) {
      IntegerVector col = output_[var_index];
      if (readstat_value_is_missing(value)) {
        col[obs_index] = NA_INTEGER;
      } else {
        col[obs_index] = adjust_datetime(readstat_int16_value(value), var_type);
      }
    } else if (type == READSTAT_TYPE_INT32) {
      IntegerVector col = output_[var_index];
      if (readstat_value_is_missing(value)) {
        col[obs_index] = NA_INTEGER;
      } else {
        col[obs_index] = adjust_datetime(readstat_int32_value(value), var_type);
      }
    } else if (type == READSTAT_TYPE_FLOAT) {
      NumericVector col = output_[var_index];
      if (readstat_value_is_missing(value)) {
        col[obs_index] = NA_REAL;
      } else {
        col[obs_index] = adjust_datetime(readstat_float_value(value), var_type);
      }
    } else if (type == READSTAT_TYPE_DOUBLE) {
      NumericVector col = output_[var_index];
      if (readstat_value_is_missing(value)) {
        col[obs_index] = NA_REAL;
      } else {
        double val = readstat_double_value(value);
        col[obs_index] = std::isnan(val) ? NA_REAL : adjust_datetime(val, var_type);
      }
    }

    return 0;
  }

  double adjust_datetime(double value, VarType var_type) {
    double offset = daysOffset(type_);

    switch(var_type) {
    case HAVEN_DEFAULT:
      return value;
    case HAVEN_DATETIME:
      if (type_ == HAVEN_STATA) // stored in milliseconds
        value /= 1000;
      return value - offset * 86400;
    case HAVEN_DATE:
      return value - offset;
    case HAVEN_TIME:
      return value;
    }
  }

  int value_label(const char *val_labels, readstat_value_t value,
                  readstat_types_t type, const char *label) {
    LabelSet& label_set = label_sets_[val_labels];
    std::string label_s(label);

    switch(type) {
    case READSTAT_TYPE_STRING:
      // Encoded to utf-8 on output
      label_set.add(readstat_string_value(value), label_s);
      break;
    case READSTAT_TYPE_CHAR:
      label_set.add(readstat_char_value(value), label_s);
      break;
    case READSTAT_TYPE_INT16:
      label_set.add(readstat_int16_value(value), label_s);
      break;
    case READSTAT_TYPE_INT32:
      label_set.add(readstat_int32_value(value), label_s);
      break;
    case READSTAT_TYPE_DOUBLE:
      label_set.add(readstat_double_value(value), label_s);
      break;
    default:
      Rf_warning("Unsupported label type: %s", type);
    }

    return 0;
  }

  List output() {
    for (int i = 0; i < output_.size(); ++i) {
      std::string label = val_labels_[i];
      if (label == "")
        continue;
      if (label_sets_.count(label) == 0)
        continue;

      RObject col = output_[i];
      col.attr("class") = "labelled";
      col.attr("labels") = label_sets_[label].labels();
    }

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
                       readstat_types_t type, void *ctx) {
  return ((DfBuilder*) ctx)->variable(index, var_name, var_format, var_label,
    val_labels, type);
}
int dfbuilder_value(int obs_index, int var_index, void *value,
                    readstat_types_t type, void *ctx) {
  return ((DfBuilder*) ctx)->value(obs_index, var_index, value, type);
}
int dfbuilder_value_label(const char *val_labels, readstat_value_t value,
                          readstat_types_t type, const char *label, void *ctx) {
  return ((DfBuilder*) ctx)->value_label(val_labels, value, type, label);
}

void print_error(const char* error_message) {
  Rcout << error_message << "\n";
}

// Parser wrappers -------------------------------------------------------------

template<typename ParseFunction>
List df_parse(FileType type, std::string filename, ParseFunction parse_f) {
  DfBuilder builder(type);

  readstat_parser_t* parser = readstat_parser_init();
  readstat_set_info_handler(parser, dfbuilder_info);
  readstat_set_variable_handler(parser, dfbuilder_variable);
  readstat_set_value_handler(parser, dfbuilder_value);
  readstat_set_value_label_handler(parser, dfbuilder_value_label);
  readstat_set_error_handler(parser, print_error);

  readstat_error_t result = parse_f(parser, filename.c_str(), &builder);

  if (result != 0) {
    readstat_parser_free(parser);
    stop("Failed to parse %s: %s.", filename, readstat_error_message(result));
  }

  readstat_parser_free(parser);
  return builder.output();
}

// [[Rcpp::export]]
List df_parse_sas(const std::string& b7dat, const std::string& b7cat) {
  DfBuilder builder(HAVEN_SAS);

  readstat_parser_t* parser = readstat_parser_init();
  readstat_set_info_handler(parser, dfbuilder_info);
  readstat_set_variable_handler(parser, dfbuilder_variable);
  readstat_set_value_handler(parser, dfbuilder_value);
  readstat_set_value_label_handler(parser, dfbuilder_value_label);
  readstat_set_error_handler(parser, print_error);

  if (b7cat != "") {
    readstat_error_t result = readstat_parse_sas7bcat(parser, b7cat.c_str(), &builder);
    if (result != 0) {
      readstat_parser_free(parser);
      stop("Failed to parse %s: %s.", b7cat.c_str(), readstat_error_message(result));
    }
  }

  readstat_error_t result = readstat_parse_sas7bdat(parser, b7dat.c_str(), &builder);
  if (result != 0) {
    readstat_parser_free(parser);
    stop("Failed to parse %s: %s.", b7dat.c_str(), readstat_error_message(result));
  }


  readstat_parser_free(parser);
  return builder.output();
}

// [[Rcpp::export]]
List df_parse_dta(std::string filename) {
  return df_parse(HAVEN_STATA, filename, readstat_parse_dta);
}

// [[Rcpp::export]]
List df_parse_por(std::string filename) {
  return df_parse(HAVEN_SPSS, filename, readstat_parse_por);
}

// [[Rcpp::export]]
List df_parse_sav(std::string filename) {
  return df_parse(HAVEN_SPSS, filename, readstat_parse_sav);
}
