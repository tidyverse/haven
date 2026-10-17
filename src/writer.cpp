#include "writer.h"

#include "cpp11/list.hpp"
#include "cpp11/sexp.hpp"
#include "cpp11/strings.hpp"

[[cpp11::register]]
void write_sav_(
    cpp11::list data,
    cpp11::strings path,
    std::string compress,
    cpp11::sexp label,
    cpp11::list meta) {
  Writer writer(HAVEN_SAV, data, path, meta);
  writer.setFileLabel(label);
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
    int strl_threshold,
    cpp11::list meta) {
  Writer writer(HAVEN_DTA, data, path, meta);
  writer.setVersion(version);
  writer.setFileLabel(label);
  writer.setStrLThreshold(strl_threshold);
  writer.write();
}

[[cpp11::register]]
void write_sas_(cpp11::list data, cpp11::strings path, cpp11::list meta) {
  Writer(HAVEN_SAS7BDAT, data, path, meta).write();
}

[[cpp11::register]]
void write_xpt_(
    cpp11::list data,
    cpp11::strings path,
    int version,
    std::string name,
    cpp11::sexp label,
    cpp11::list meta) {
  Writer writer(HAVEN_XPT, data, path, meta);
  writer.setVersion(version);
  writer.setName(name);
  writer.setFileLabel(label);
  writer.write();
}
