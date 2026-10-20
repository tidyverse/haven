#ifndef HAVEN_TYPES_H
#define HAVEN_TYPES_H

#include <string>

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

#endif
