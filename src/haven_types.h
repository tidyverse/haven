#ifndef __HAVEN_TYPES__
#define __HAVEN_TYPES__

#include <Rcpp.h>
#include <cmath>

enum FileVendor {
  HAVEN_SPSS,
  HAVEN_STATA,
  HAVEN_SAS
};

enum FileExt {
  HAVEN_SAV,
  HAVEN_POR,
  HAVEN_DTA,
  HAVEN_SAS7BDAT,
  HAVEN_SAS7BCAT,
  HAVEN_XPT
};

FileVendor extVendor(FileExt ext);

enum VarType {
  HAVEN_DEFAULT,
  HAVEN_DATE,
  HAVEN_TIME,
  HAVEN_DATETIME
};

std::string formatAttribute(FileVendor vendor);
bool hasPrefix(std::string x, std::string prefix);
VarType numType(SEXP x);
VarType numType(FileVendor vendor, const char* var_format);

// Value conversion -----------------------------------------------------------

int daysOffset(FileVendor vendor);
double adjustDatetimeToR(FileVendor vendor, VarType var, double value);
double adjustDatetimeFromR(FileVendor vendor, SEXP col, double value);

#endif
