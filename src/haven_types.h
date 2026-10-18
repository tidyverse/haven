#ifndef HAVEN_TYPES_H
#define HAVEN_TYPES_H

#include <cpp11/R.hpp>
#include <cpp11/integers.hpp>
#include <cpp11/list.hpp>
#include <cpp11/strings.hpp>
#include <string>

#define CPP11_PARTIAL

enum FileVendor { HAVEN_SPSS, HAVEN_STATA, HAVEN_SAS };

enum FileExt {
  HAVEN_SAV,
  HAVEN_POR,
  HAVEN_DTA,
  HAVEN_SAS7BDAT,
  HAVEN_SAS7BCAT,
  HAVEN_XPT
};

FileVendor extVendor(FileExt ext);

std::string formatAttribute(FileVendor vendor);

#endif
