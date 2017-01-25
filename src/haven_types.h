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

inline bool hasPrefix(std::string x, std::string prefix) {
  return x.compare(0, prefix.size(), prefix) == 0;
}

inline VarType numType(SEXP x) {
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

inline std::string formatAttribute(FileType type) {
  switch (type) {
  case HAVEN_STATA: return "format.stata";
  case HAVEN_SPSS:  return "format.spss";
  case HAVEN_SAS:   return "format.sas";
  case HAVEN_XPT:   return "format.xpt";
  }

  return "";
}


inline VarType numType(FileType type, const char* var_format) {
  if (var_format == NULL)
    return HAVEN_DEFAULT;

  std::string format(var_format);

  switch(type) {
  case HAVEN_XPT:
  case HAVEN_SAS:
    if      (format == "DATETIME") return HAVEN_DATETIME;
    else if (format == "WEEKDATE") return HAVEN_DATE;
    else if (format == "MMDDYY")   return HAVEN_DATE;
    else if (format == "DDMMYY")   return HAVEN_DATE;
    else if (format == "YYMMDD")   return HAVEN_DATE;
    else if (format == "DATE")     return HAVEN_DATE;
    else if (format == "TIME")     return HAVEN_TIME;
    else if (format == "HHMM")     return HAVEN_TIME;
    else                           return HAVEN_DEFAULT;
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

inline int daysOffset(FileType type) {
  switch(type) {
  case HAVEN_XPT:
  case HAVEN_SAS:   return 3653;   // 1960-01-01
  case HAVEN_STATA: return 3653;
  case HAVEN_SPSS:  return 141428; // 1582-01-01
  }

  return 0;
}

inline double adjustDatetimeToR(FileType file, VarType var, double value) {
  if (std::isnan(value))
    return value;

  double offset = daysOffset(file);

  switch(var) {
  case HAVEN_DATETIME:
    if (file == HAVEN_STATA) // stored in milliseconds
      value /= 1000;
    return value - offset * 86400;
  case HAVEN_DATE:
    if (file == HAVEN_SPSS) // stored in seconds
      value /= 86400;
    return value - offset;
  default:
    return value;
  }
}

inline double adjustDatetimeFromR(FileType file, SEXP col, double value) {
  if (std::isnan(value))
    return value;

  double offset = daysOffset(file);

  switch(numType(col)) {
  case HAVEN_DATETIME:
    value += offset * 86400;
    if (file == HAVEN_STATA) // stored in milliseconds
      value *= 1000;
    return value;
  case HAVEN_DATE:
    value += offset;
    if (file == HAVEN_SPSS) // stored in seconds
      value *= 86400;
    return value;
  default:
    return value;
  }
}
#endif
