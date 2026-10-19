#include "haven_types.h"
#include <cpp11/protect.hpp>

FileVendor extVendor(FileExt ext) {
  switch (ext) {
  case HAVEN_DTA:
    return HAVEN_STATA;
  case HAVEN_SAV:
  case HAVEN_POR:
    return HAVEN_SPSS;
  case HAVEN_SAS7BDAT:
  case HAVEN_SAS7BCAT:
  case HAVEN_XPT:
    return HAVEN_SAS;
  default:
    cpp11::stop("Unknown file extension");
  }
  /* not actually reached */
  return HAVEN_SAS;
}

std::string formatAttribute(FileVendor vendor) {
  switch (vendor) {
  case HAVEN_STATA:
    return "format.stata";
  case HAVEN_SPSS:
    return "format.spss";
  case HAVEN_SAS:
    return "format.sas";
  }

  return "";
}
