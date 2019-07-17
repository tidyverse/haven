#include <Rcpp.h>
#include <cmath>
#include "haven_types.h"

FileVendor extVendor(FileExt ext) {
  switch (ext) {
  case HAVEN_DTA:      return HAVEN_STATA;
  case HAVEN_SAV:
  case HAVEN_POR:      return HAVEN_SPSS;
  case HAVEN_SAS7BDAT:
  case HAVEN_SAS7BCAT:
  case HAVEN_XPT:      return HAVEN_SAS;
  default:
    Rcpp::stop("Unknown file extension");
  }
}

std::string formatAttribute(FileVendor vendor) {
  switch (vendor) {
  case HAVEN_STATA: return "format.stata";
  case HAVEN_SPSS:  return "format.spss";
  case HAVEN_SAS:   return "format.sas";
  }

  return "";
}

bool hasPrefix(std::string x, std::string prefix) {
  return x.compare(0, prefix.size(), prefix) == 0;
}

VarType numType(SEXP x) {
  if (Rf_inherits(x, "Date")) {
    return HAVEN_DATE;
  } else if (Rf_inherits(x, "POSIXct")) {
    return HAVEN_DATETIME;
  } else if (Rf_inherits(x, "hms")) {
    return HAVEN_TIME;
  } else {
    return HAVEN_DEFAULT;
  }
}

VarType numType(FileVendor vendor, const char* var_format) {
  if (var_format == NULL)
    return HAVEN_DEFAULT;

  std::string format(var_format);

  switch(vendor) {
  case HAVEN_SAS:
    // http://support.sas.com/documentation/cdl/en/lrdict/64316/HTML/default/viewer.htm#a000589916.htm
    if      (hasPrefix(format,"DATETIME")) return HAVEN_DATETIME;
    else if (hasPrefix(format,"IS8601DT")) return HAVEN_DATETIME;
    else if (hasPrefix(format,"E8601DT"))  return HAVEN_DATETIME;
    else if (hasPrefix(format,"B8601DT"))  return HAVEN_DATETIME;
    else if (hasPrefix(format,"IS8601DA")) return HAVEN_DATE;
    else if (hasPrefix(format,"E8601DA"))  return HAVEN_DATE;
    else if (hasPrefix(format,"B8601DA"))  return HAVEN_DATE;
    else if (hasPrefix(format,"WEEKDATE")) return HAVEN_DATE;
    else if (hasPrefix(format,"MMDDYY"))   return HAVEN_DATE;
    else if (hasPrefix(format,"DDMMYY"))   return HAVEN_DATE;
    else if (hasPrefix(format,"YYMMDD"))   return HAVEN_DATE;
    else if (hasPrefix(format,"DATE"))     return HAVEN_DATE;
    else if (hasPrefix(format,"TIME"))     return HAVEN_TIME;
    else if (hasPrefix(format,"HHMM"))     return HAVEN_TIME;
    else if (hasPrefix(format,"IS8601TM")) return HAVEN_TIME;
    else if (hasPrefix(format,"E8601TM"))  return HAVEN_TIME;
    else if (hasPrefix(format,"B8601TM"))  return HAVEN_TIME;
    else                                   return HAVEN_DEFAULT;
  case HAVEN_SPSS:
    // http://www-01.ibm.com/support/knowledgecenter/?lang=en#!/SSLVMB_20.0.0/com.ibm.spss.statistics.help/syn_date_and_time_date_time_formats.htm
    if      (hasPrefix(format, "DATETIME")) return HAVEN_DATETIME;
    else if (hasPrefix(format, "DATE"))     return HAVEN_DATE;
    else if (hasPrefix(format, "ADATE"))    return HAVEN_DATE;
    else if (hasPrefix(format, "EDATE"))    return HAVEN_DATE;
    else if (hasPrefix(format, "JDATE"))    return HAVEN_DATE;
    else if (hasPrefix(format, "SDATE"))    return HAVEN_DATE;
    else if (hasPrefix(format, "TIME"))     return HAVEN_TIME;
    else if (hasPrefix(format, "DTIME"))    return HAVEN_TIME;
    else                                    return HAVEN_DEFAULT;
  case HAVEN_STATA:
    if      (hasPrefix(format, "%tC"))      return HAVEN_DATETIME;
    else if (hasPrefix(format, "%tc"))      return HAVEN_DATETIME;
    else if (hasPrefix(format, "%td"))      return HAVEN_DATE;
    else if (hasPrefix(format, "%d"))       return HAVEN_DATE;
    else                                    return HAVEN_DEFAULT;
  }

  return HAVEN_DEFAULT;
}


// Value conversion -----------------------------------------------------------

int daysOffset(FileVendor vendor) {
  switch(vendor) {
  case HAVEN_SAS:   return 3653;   // 1960-01-01
  case HAVEN_STATA: return 3653;
  case HAVEN_SPSS:  return 141428; // 1582-01-01
  }

  return 0;
}

double adjustDatetimeToR(FileVendor vendor, VarType var, double value) {
  if (std::isnan(value))
    return value;

  double offset = daysOffset(vendor);

  switch(var) {
  case HAVEN_DATETIME:
    if (vendor == HAVEN_STATA) // stored in milliseconds
      value /= 1000;
    return value - offset * 86400;
  case HAVEN_DATE:
    if (vendor == HAVEN_SPSS) // stored in seconds
      value /= 86400;
    return value - offset;
  default:
    return value;
  }
}

double adjustDatetimeFromR(FileVendor vendor, SEXP col, double value) {
  if (std::isnan(value))
    return value;

  double offset = daysOffset(vendor);

  switch(numType(col)) {
  case HAVEN_DATETIME:
    value += offset * 86400;
    if (vendor == HAVEN_STATA) // stored in milliseconds
      value *= 1000;
    return value;
  case HAVEN_DATE:
    value += offset;
    if (vendor == HAVEN_SPSS) // stored in seconds
      value *= 86400;
    return value;
  default:
    return value;
  }
}
