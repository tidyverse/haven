#include "datetime.h"
#include <cmath>
#include <string>
#include <vector>

namespace {

typedef std::vector<std::string> Prefixes;

bool hasAnyPrefix(const std::string& x, const Prefixes& prefixes) {
  for (const std::string& prefix : prefixes) {
    if (x.compare(0, prefix.size(), prefix) == 0)
      return true;
  }
  return false;
}

VarType matchFormat(
    const std::string& format,
    const Prefixes& datetime,
    const Prefixes& date,
    const Prefixes& time) {
  if (hasAnyPrefix(format, datetime))
    return HAVEN_DATETIME;
  if (hasAnyPrefix(format, date))
    return HAVEN_DATE;
  if (hasAnyPrefix(format, time))
    return HAVEN_TIME;
  return HAVEN_DEFAULT;
}

// http://support.sas.com/documentation/cdl/en/lrdict/64316/HTML/default/viewer.htm#a000589916.htm
const Prefixes sas_datetime = {
    "DATETIME",
    "DT",
    "DATEAMPM",
    "MDYAMPM",
    "NLDATMT",
    "NLDATM",
    "IS8601DN",
    "IS8601DT",
    "IS8601DZ",
    "B8601DN",
    "B8601DT",
    "B8601DX",
    "B8601DZ",
    "B8601LX",
    "E8601DN",
    "E8601DT",
    "E8601DX",
    "E8601DZ",
    "E8601LX"};
const Prefixes sas_date = {
    "DATE",   "NLDATE", "DOWNAME", "IS8601DA", "B8601DA", "E8601DA",
    "DAY",    "WEEK",   "MON",     "QTR",      "YEAR",    "MMDDYY",
    "DDMMYY", "YYMMDD", "MMYY",    "YYMM",     "YY",      "WORDDAT",
    "NENGO",  "JULIAN", "JULDAY",  "PDJULG",   "PDJULI"};
const Prefixes sas_time = {
    "TIME",
    "TIMEAMPM",
    "NLTIM",
    "TOD",
    "HOUR",
    "HHMM",
    "MMSS",
    "IS8601T",
    "B8601T",
    "B8601LZ",
    "E8601T",
    "E8601LZ"};

// http://www-01.ibm.com/support/knowledgecenter/?lang=en#!/SSLVMB_20.0.0/com.ibm.spss.statistics.help/syn_date_and_time_date_time_formats.htm
const Prefixes spss_datetime = {"DATETIME"};
const Prefixes spss_date = {"DATE", "ADATE", "EDATE", "JDATE", "SDATE"};
const Prefixes spss_time = {"TIME", "DTIME"};

const Prefixes stata_datetime = {"%tC", "%tc"};
const Prefixes stata_date = {"%td", "%d"};
const Prefixes stata_time = {};

} // namespace

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
  switch (vendor) {
  case HAVEN_SAS:
    return matchFormat(format, sas_datetime, sas_date, sas_time);
  case HAVEN_SPSS:
    return matchFormat(format, spss_datetime, spss_date, spss_time);
  case HAVEN_STATA:
    return matchFormat(format, stata_datetime, stata_date, stata_time);
  }

  return HAVEN_DEFAULT;
}

// Value conversion -----------------------------------------------------------

int daysOffset(FileVendor vendor) {
  switch (vendor) {
  case HAVEN_SAS:
    return 3653; // 1960-01-01
  case HAVEN_STATA:
    return 3653;
  case HAVEN_SPSS:
    return 141428; // 1582-01-01
  }

  return 0;
}

double adjustDatetimeToR(FileVendor vendor, VarType var, double value) {
  if (std::isnan(value))
    return value;

  double offset = daysOffset(vendor);

  switch (var) {
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

  switch (numType(col)) {
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
