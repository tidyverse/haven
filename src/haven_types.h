#ifndef __HAVEN_TYPES__
#define __HAVEN_TYPES__

#include <Rcpp.h>

enum FileType {
  HAVEN_SPSS,
  HAVEN_STATA,
  HAVEN_SAS
};

enum VarType {
  HAVEN_DEFAULT,
  HAVEN_DATE,
  HAVEN_TIME,
  HAVEN_DATETIME
};

inline int daysOffset(FileType type) {
  switch(type) {
  case HAVEN_SAS:   return 3653;   // 1960-01-01
  case HAVEN_STATA: return 3653;
  case HAVEN_SPSS:  return 141428; // 1582-01-01
  }

  return 0;
}

inline bool hasPrefix(std::string x, std::string prefix) {
  return x.compare(0, prefix.size(), prefix) == 0;
}

inline VarType numType(FileType type, const char* var_format) {
  if (var_format == NULL)
    return HAVEN_DEFAULT;

  std::string format(var_format);

  switch(type) {
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
    else if (hasPrefix(format, "DATE"))     return HAVEN_DATETIME;
    else if (hasPrefix(format, "ADATE"))    return HAVEN_DATETIME;
    else if (hasPrefix(format, "EDATE"))    return HAVEN_DATETIME;
    else if (hasPrefix(format, "JDATE"))    return HAVEN_DATETIME;
    else if (hasPrefix(format, "SDATE"))    return HAVEN_DATETIME;
    else if (hasPrefix(format, "TIME"))     return HAVEN_TIME;
    else if (hasPrefix(format, "DTIME"))    return HAVEN_TIME;
    else                                    return HAVEN_DEFAULT;
  case HAVEN_STATA:
    if      (format == "%tC")               return HAVEN_DATETIME;
    else if (format == "%tc")               return HAVEN_DATETIME;
    else if (hasPrefix(format, "%td"))      return HAVEN_DATE;
    else                                    return HAVEN_DEFAULT;
  }

  return HAVEN_DEFAULT;
}

#endif
