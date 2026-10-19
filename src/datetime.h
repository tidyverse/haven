#ifndef HAVEN_DATETIME_H
#define HAVEN_DATETIME_H

#include "haven_types.h"

#include "cpp11/R.hpp"

enum VarType { HAVEN_DEFAULT, HAVEN_DATE, HAVEN_TIME, HAVEN_DATETIME };

// Classifies an R vector by its class (Date, POSIXct, or hms)
VarType varTypeFromR(SEXP x);
// Classifies a variable by its vendor-specific format string
VarType varTypeFromFormat(FileVendor vendor, const char* format);
// The vendor-specific format to use for a VarType, or NULL if none
const char* defaultFormat(FileVendor vendor, VarType type);

double adjustDatetimeToR(FileVendor vendor, VarType type, double value);
double adjustDatetimeFromR(FileVendor vendor, VarType type, double value);

#endif
