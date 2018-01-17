#ifndef __HAVEN_TYPES__
#define __HAVEN_TYPES__

#include <Rcpp.h>
#include <cmath>

enum FileType {
  HAVEN_SPSS,
  HAVEN_STATA,
  HAVEN_SAS,
  HAVEN_XPT
};

enum VarType {
  HAVEN_DEFAULT,
  HAVEN_DATE,
  HAVEN_TIME,
  HAVEN_DATETIME
};

std::string formatAttribute(FileType type);
bool hasPrefix(std::string x, std::string prefix);
VarType numType(SEXP x);
VarType numType(FileType type, const char* var_format);

// Value conversion -----------------------------------------------------------

int daysOffset(FileType type);
double adjustDatetimeToR(FileType file, VarType var, double value);
double adjustDatetimeFromR(FileType file, SEXP col, double value);

#endif
