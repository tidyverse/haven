#include "DfWriter.h"

#include "cpp11/list.hpp"
#include "cpp11/sexp.hpp"
#include "cpp11/strings.hpp"

[[cpp11::register]]
void write_sav_(cpp11::list data, cpp11::strings path, std::string compress) {
  Writer writer(HAVEN_SAV, data, path);
  if (compress == "zsav")
    writer.setCompression(READSTAT_COMPRESS_BINARY);
  else if (compress == "none")
    writer.setCompression(READSTAT_COMPRESS_NONE);
  else
    writer.setCompression(READSTAT_COMPRESS_ROWS);
  writer.write();
}

[[cpp11::register]]
void write_dta_(
    cpp11::list data,
    cpp11::strings path,
    int version,
    cpp11::sexp label,
    int strl_threshold) {
  Writer writer(HAVEN_DTA, data, path);
  writer.setVersion(version);
  writer.setFileLabel(label);
  writer.setStrLThreshold(strl_threshold);
  writer.write();
}

[[cpp11::register]]
void write_sas_(cpp11::list data, cpp11::strings path) {
  Writer(HAVEN_SAS7BDAT, data, path).write();
}

[[cpp11::register]]
void write_xpt_(
    cpp11::list data,
    cpp11::strings path,
    int version,
    std::string name,
    cpp11::sexp label) {
  Writer writer(HAVEN_XPT, data, path);
  writer.setVersion(version);
  writer.setName(name);
  writer.setFileLabel(label);
  writer.write();
}
