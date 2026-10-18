#ifndef HAVEN_READER_LABELS_H
#define HAVEN_READER_LABELS_H

#include <string>
#include <vector>

#include "readstat.h"

#include "cpp11/doubles.hpp"
#include "cpp11/integers.hpp"
#include "cpp11/sexp.hpp"
#include "cpp11/strings.hpp"

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
