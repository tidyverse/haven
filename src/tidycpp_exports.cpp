// Generated by tidycpp: do not edit by hand

#include "tidycpp/declarations.hpp"
#include <Rcpp.h>
using namespace Rcpp;

// DfReader.cpp
tidycpp::list df_parse_sas_file(tidycpp::list spec_b7dat, tidycpp::list spec_b7cat, std::string encoding, std::string catalog_encoding, std::vector<std::string> cols_skip, long n_max, long rows_skip, std::string name_repair);
extern "C" SEXP _tidycpp_df_parse_sas_file(SEXP spec_b7dat, SEXP spec_b7cat, SEXP encoding, SEXP catalog_encoding, SEXP cols_skip, SEXP n_max, SEXP rows_skip, SEXP name_repair) {
  BEGIN_TIDYCPP
    return tidycpp::as_sexp(df_parse_sas_file(tidycpp::as_cpp<tidycpp::list>(spec_b7dat), tidycpp::as_cpp<tidycpp::list>(spec_b7cat), tidycpp::as_cpp<std::string>(encoding), tidycpp::as_cpp<std::string>(catalog_encoding), tidycpp::as_cpp<std::vector<std::string>>(cols_skip), tidycpp::as_cpp<long>(n_max), tidycpp::as_cpp<long>(rows_skip), tidycpp::as_cpp<std::string>(name_repair)));
  END_TIDYCPP
}
// DfReader.cpp
tidycpp::list df_parse_sas_raw(tidycpp::list spec_b7dat, tidycpp::list spec_b7cat, std::string encoding, std::string catalog_encoding, std::vector<std::string> cols_skip, long n_max, long rows_skip, std::string name_repair);
extern "C" SEXP _tidycpp_df_parse_sas_raw(SEXP spec_b7dat, SEXP spec_b7cat, SEXP encoding, SEXP catalog_encoding, SEXP cols_skip, SEXP n_max, SEXP rows_skip, SEXP name_repair) {
  BEGIN_TIDYCPP
    return tidycpp::as_sexp(df_parse_sas_raw(tidycpp::as_cpp<tidycpp::list>(spec_b7dat), tidycpp::as_cpp<tidycpp::list>(spec_b7cat), tidycpp::as_cpp<std::string>(encoding), tidycpp::as_cpp<std::string>(catalog_encoding), tidycpp::as_cpp<std::vector<std::string>>(cols_skip), tidycpp::as_cpp<long>(n_max), tidycpp::as_cpp<long>(rows_skip), tidycpp::as_cpp<std::string>(name_repair)));
  END_TIDYCPP
}
// DfReader.cpp
tidycpp::list df_parse_xpt_file(tidycpp::list spec, std::vector<std::string> cols_skip, long n_max, long rows_skip, std::string name_repair);
extern "C" SEXP _tidycpp_df_parse_xpt_file(SEXP spec, SEXP cols_skip, SEXP n_max, SEXP rows_skip, SEXP name_repair) {
  BEGIN_TIDYCPP
    return tidycpp::as_sexp(df_parse_xpt_file(tidycpp::as_cpp<tidycpp::list>(spec), tidycpp::as_cpp<std::vector<std::string>>(cols_skip), tidycpp::as_cpp<long>(n_max), tidycpp::as_cpp<long>(rows_skip), tidycpp::as_cpp<std::string>(name_repair)));
  END_TIDYCPP
}
// DfReader.cpp
tidycpp::list df_parse_xpt_raw(tidycpp::list spec, std::vector<std::string> cols_skip, long n_max, long rows_skip, std::string name_repair);
extern "C" SEXP _tidycpp_df_parse_xpt_raw(SEXP spec, SEXP cols_skip, SEXP n_max, SEXP rows_skip, SEXP name_repair) {
  BEGIN_TIDYCPP
    return tidycpp::as_sexp(df_parse_xpt_raw(tidycpp::as_cpp<tidycpp::list>(spec), tidycpp::as_cpp<std::vector<std::string>>(cols_skip), tidycpp::as_cpp<long>(n_max), tidycpp::as_cpp<long>(rows_skip), tidycpp::as_cpp<std::string>(name_repair)));
  END_TIDYCPP
}
// DfReader.cpp
tidycpp::list df_parse_dta_file(tidycpp::list spec, std::string encoding, std::vector<std::string> cols_skip, long n_max, long rows_skip, std::string name_repair);
extern "C" SEXP _tidycpp_df_parse_dta_file(SEXP spec, SEXP encoding, SEXP cols_skip, SEXP n_max, SEXP rows_skip, SEXP name_repair) {
  BEGIN_TIDYCPP
    return tidycpp::as_sexp(df_parse_dta_file(tidycpp::as_cpp<tidycpp::list>(spec), tidycpp::as_cpp<std::string>(encoding), tidycpp::as_cpp<std::vector<std::string>>(cols_skip), tidycpp::as_cpp<long>(n_max), tidycpp::as_cpp<long>(rows_skip), tidycpp::as_cpp<std::string>(name_repair)));
  END_TIDYCPP
}
// DfReader.cpp
tidycpp::list df_parse_dta_raw(tidycpp::list spec, std::string encoding, std::vector<std::string> cols_skip, long n_max, long rows_skip, std::string name_repair);
extern "C" SEXP _tidycpp_df_parse_dta_raw(SEXP spec, SEXP encoding, SEXP cols_skip, SEXP n_max, SEXP rows_skip, SEXP name_repair) {
  BEGIN_TIDYCPP
    return tidycpp::as_sexp(df_parse_dta_raw(tidycpp::as_cpp<tidycpp::list>(spec), tidycpp::as_cpp<std::string>(encoding), tidycpp::as_cpp<std::vector<std::string>>(cols_skip), tidycpp::as_cpp<long>(n_max), tidycpp::as_cpp<long>(rows_skip), tidycpp::as_cpp<std::string>(name_repair)));
  END_TIDYCPP
}
// DfReader.cpp
tidycpp::list df_parse_sav_file(tidycpp::list spec, std::string encoding, bool user_na, std::vector<std::string> cols_skip, long n_max, long rows_skip, std::string name_repair);
extern "C" SEXP _tidycpp_df_parse_sav_file(SEXP spec, SEXP encoding, SEXP user_na, SEXP cols_skip, SEXP n_max, SEXP rows_skip, SEXP name_repair) {
  BEGIN_TIDYCPP
    return tidycpp::as_sexp(df_parse_sav_file(tidycpp::as_cpp<tidycpp::list>(spec), tidycpp::as_cpp<std::string>(encoding), tidycpp::as_cpp<bool>(user_na), tidycpp::as_cpp<std::vector<std::string>>(cols_skip), tidycpp::as_cpp<long>(n_max), tidycpp::as_cpp<long>(rows_skip), tidycpp::as_cpp<std::string>(name_repair)));
  END_TIDYCPP
}
// DfReader.cpp
tidycpp::list df_parse_sav_raw(tidycpp::list spec, std::string encoding, bool user_na, std::vector<std::string> cols_skip, long n_max, long rows_skip, std::string name_repair);
extern "C" SEXP _tidycpp_df_parse_sav_raw(SEXP spec, SEXP encoding, SEXP user_na, SEXP cols_skip, SEXP n_max, SEXP rows_skip, SEXP name_repair) {
  BEGIN_TIDYCPP
    return tidycpp::as_sexp(df_parse_sav_raw(tidycpp::as_cpp<tidycpp::list>(spec), tidycpp::as_cpp<std::string>(encoding), tidycpp::as_cpp<bool>(user_na), tidycpp::as_cpp<std::vector<std::string>>(cols_skip), tidycpp::as_cpp<long>(n_max), tidycpp::as_cpp<long>(rows_skip), tidycpp::as_cpp<std::string>(name_repair)));
  END_TIDYCPP
}
// DfReader.cpp
tidycpp::list df_parse_por_file(tidycpp::list spec, std::string encoding, bool user_na, std::vector<std::string> cols_skip, long n_max, long rows_skip, std::string name_repair);
extern "C" SEXP _tidycpp_df_parse_por_file(SEXP spec, SEXP encoding, SEXP user_na, SEXP cols_skip, SEXP n_max, SEXP rows_skip, SEXP name_repair) {
  BEGIN_TIDYCPP
    return tidycpp::as_sexp(df_parse_por_file(tidycpp::as_cpp<tidycpp::list>(spec), tidycpp::as_cpp<std::string>(encoding), tidycpp::as_cpp<bool>(user_na), tidycpp::as_cpp<std::vector<std::string>>(cols_skip), tidycpp::as_cpp<long>(n_max), tidycpp::as_cpp<long>(rows_skip), tidycpp::as_cpp<std::string>(name_repair)));
  END_TIDYCPP
}
// DfReader.cpp
tidycpp::list df_parse_por_raw(tidycpp::list spec, std::string encoding, bool user_na, std::vector<std::string> cols_skip, long n_max, long rows_skip, std::string name_repair);
extern "C" SEXP _tidycpp_df_parse_por_raw(SEXP spec, SEXP encoding, SEXP user_na, SEXP cols_skip, SEXP n_max, SEXP rows_skip, SEXP name_repair) {
  BEGIN_TIDYCPP
    return tidycpp::as_sexp(df_parse_por_raw(tidycpp::as_cpp<tidycpp::list>(spec), tidycpp::as_cpp<std::string>(encoding), tidycpp::as_cpp<bool>(user_na), tidycpp::as_cpp<std::vector<std::string>>(cols_skip), tidycpp::as_cpp<long>(n_max), tidycpp::as_cpp<long>(rows_skip), tidycpp::as_cpp<std::string>(name_repair)));
  END_TIDYCPP
}
// DfWriter.cpp
void write_sav_(List data, CharacterVector path, bool compress);
extern "C" SEXP _tidycpp_write_sav_(SEXP data, SEXP path, SEXP compress) {
  BEGIN_TIDYCPP
    write_sav_(tidycpp::as_cpp<List>(data), tidycpp::as_cpp<CharacterVector>(path), tidycpp::as_cpp<bool>(compress));
  return R_NilValue;
  END_TIDYCPP
}
// DfWriter.cpp
void write_dta_(List data, CharacterVector path, int version, RObject label);
extern "C" SEXP _tidycpp_write_dta_(SEXP data, SEXP path, SEXP version, SEXP label) {
  BEGIN_TIDYCPP
    write_dta_(tidycpp::as_cpp<List>(data), tidycpp::as_cpp<CharacterVector>(path), tidycpp::as_cpp<int>(version), tidycpp::as_cpp<RObject>(label));
  return R_NilValue;
  END_TIDYCPP
}
// DfWriter.cpp
void write_sas_(List data, CharacterVector path);
extern "C" SEXP _tidycpp_write_sas_(SEXP data, SEXP path) {
  BEGIN_TIDYCPP
    write_sas_(tidycpp::as_cpp<List>(data), tidycpp::as_cpp<CharacterVector>(path));
  return R_NilValue;
  END_TIDYCPP
}
// DfWriter.cpp
void write_xpt_(List data, CharacterVector path, int version, std::string name);
extern "C" SEXP _tidycpp_write_xpt_(SEXP data, SEXP path, SEXP version, SEXP name) {
  BEGIN_TIDYCPP
    write_xpt_(tidycpp::as_cpp<List>(data), tidycpp::as_cpp<CharacterVector>(path), tidycpp::as_cpp<int>(version), tidycpp::as_cpp<std::string>(name));
  return R_NilValue;
  END_TIDYCPP
}

extern "C" {
/* .Call calls */
extern SEXP _tidycpp_df_parse_dta_file(SEXP, SEXP, SEXP, SEXP, SEXP, SEXP);
extern SEXP _tidycpp_df_parse_dta_raw(SEXP, SEXP, SEXP, SEXP, SEXP, SEXP);
extern SEXP _tidycpp_df_parse_por_file(SEXP, SEXP, SEXP, SEXP, SEXP, SEXP, SEXP);
extern SEXP _tidycpp_df_parse_por_raw(SEXP, SEXP, SEXP, SEXP, SEXP, SEXP, SEXP);
extern SEXP _tidycpp_df_parse_sas_file(SEXP, SEXP, SEXP, SEXP, SEXP, SEXP, SEXP, SEXP);
extern SEXP _tidycpp_df_parse_sas_raw(SEXP, SEXP, SEXP, SEXP, SEXP, SEXP, SEXP, SEXP);
extern SEXP _tidycpp_df_parse_sav_file(SEXP, SEXP, SEXP, SEXP, SEXP, SEXP, SEXP);
extern SEXP _tidycpp_df_parse_sav_raw(SEXP, SEXP, SEXP, SEXP, SEXP, SEXP, SEXP);
extern SEXP _tidycpp_df_parse_xpt_file(SEXP, SEXP, SEXP, SEXP, SEXP);
extern SEXP _tidycpp_df_parse_xpt_raw(SEXP, SEXP, SEXP, SEXP, SEXP);
extern SEXP _tidycpp_write_dta_(SEXP, SEXP, SEXP, SEXP);
extern SEXP _tidycpp_write_sas_(SEXP, SEXP);
extern SEXP _tidycpp_write_sav_(SEXP, SEXP, SEXP);
extern SEXP _tidycpp_write_xpt_(SEXP, SEXP, SEXP, SEXP);
extern SEXP is_tagged_na_(SEXP, SEXP);
extern SEXP na_tag_(SEXP);
extern SEXP tagged_na_(SEXP);

static const R_CallMethodDef CallEntries[] = {
    {"_tidycpp_df_parse_dta_file", (DL_FUNC) &_tidycpp_df_parse_dta_file, 6},
    {"_tidycpp_df_parse_dta_raw",  (DL_FUNC) &_tidycpp_df_parse_dta_raw,  6},
    {"_tidycpp_df_parse_por_file", (DL_FUNC) &_tidycpp_df_parse_por_file, 7},
    {"_tidycpp_df_parse_por_raw",  (DL_FUNC) &_tidycpp_df_parse_por_raw,  7},
    {"_tidycpp_df_parse_sas_file", (DL_FUNC) &_tidycpp_df_parse_sas_file, 8},
    {"_tidycpp_df_parse_sas_raw",  (DL_FUNC) &_tidycpp_df_parse_sas_raw,  8},
    {"_tidycpp_df_parse_sav_file", (DL_FUNC) &_tidycpp_df_parse_sav_file, 7},
    {"_tidycpp_df_parse_sav_raw",  (DL_FUNC) &_tidycpp_df_parse_sav_raw,  7},
    {"_tidycpp_df_parse_xpt_file", (DL_FUNC) &_tidycpp_df_parse_xpt_file, 5},
    {"_tidycpp_df_parse_xpt_raw",  (DL_FUNC) &_tidycpp_df_parse_xpt_raw,  5},
    {"_tidycpp_write_dta_",        (DL_FUNC) &_tidycpp_write_dta_,        4},
    {"_tidycpp_write_sas_",        (DL_FUNC) &_tidycpp_write_sas_,        2},
    {"_tidycpp_write_sav_",        (DL_FUNC) &_tidycpp_write_sav_,        3},
    {"_tidycpp_write_xpt_",        (DL_FUNC) &_tidycpp_write_xpt_,        4},
    {"is_tagged_na_",              (DL_FUNC) &is_tagged_na_,              2},
    {"na_tag_",                    (DL_FUNC) &na_tag_,                    1},
    {"tagged_na_",                 (DL_FUNC) &tagged_na_,                 1},
    {NULL, NULL, 0}
};
}



extern "C" void R_init_haven(DllInfo* dll){
  R_registerRoutines(dll, NULL, CallEntries, NULL, NULL);
  R_useDynamicSymbols(dll, FALSE);
  
}

