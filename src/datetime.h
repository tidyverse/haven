#ifndef HAVEN_DATETIME_H
#define HAVEN_DATETIME_H

#include "haven_types.h"

enum VarType { HAVEN_DEFAULT, HAVEN_DATE, HAVEN_TIME, HAVEN_DATETIME };

VarType numType(SEXP x);
VarType numType(FileVendor vendor, const char* var_format);

int daysOffset(FileVendor vendor);
double adjustDatetimeToR(FileVendor vendor, VarType var, double value);
double adjustDatetimeFromR(FileVendor vendor, SEXP col, double value);

#endif
