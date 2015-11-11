// This file was generated by Rcpp::compileAttributes
// Generator token: 10BE3573-1514-4C36-9D1C-5A225CD40393

#include "haven_types.h"
#include <Rcpp.h>

using namespace Rcpp;

// df_parse_sas
List df_parse_sas(const std::string& b7dat, const std::string& b7cat, int normalise_na);
RcppExport SEXP haven_df_parse_sas(SEXP b7datSEXP, SEXP b7catSEXP, SEXP normalise_naSEXP) {
BEGIN_RCPP
    Rcpp::RObject __result;
    Rcpp::RNGScope __rngScope;
    Rcpp::traits::input_parameter< const std::string& >::type b7dat(b7datSEXP);
    Rcpp::traits::input_parameter< const std::string& >::type b7cat(b7catSEXP);
    Rcpp::traits::input_parameter< int >::type normalise_na(normalise_naSEXP);
    __result = Rcpp::wrap(df_parse_sas(b7dat, b7cat, normalise_na));
    return __result;
END_RCPP
}
// df_parse_dta
List df_parse_dta(std::string filename, int normalise_na);
RcppExport SEXP haven_df_parse_dta(SEXP filenameSEXP, SEXP normalise_naSEXP) {
BEGIN_RCPP
    Rcpp::RObject __result;
    Rcpp::RNGScope __rngScope;
    Rcpp::traits::input_parameter< std::string >::type filename(filenameSEXP);
    Rcpp::traits::input_parameter< int >::type normalise_na(normalise_naSEXP);
    __result = Rcpp::wrap(df_parse_dta(filename, normalise_na));
    return __result;
END_RCPP
}
// df_parse_por
List df_parse_por(std::string filename, int normalise_na);
RcppExport SEXP haven_df_parse_por(SEXP filenameSEXP, SEXP normalise_naSEXP) {
BEGIN_RCPP
    Rcpp::RObject __result;
    Rcpp::RNGScope __rngScope;
    Rcpp::traits::input_parameter< std::string >::type filename(filenameSEXP);
    Rcpp::traits::input_parameter< int >::type normalise_na(normalise_naSEXP);
    __result = Rcpp::wrap(df_parse_por(filename, normalise_na));
    return __result;
END_RCPP
}
// df_parse_sav
List df_parse_sav(std::string filename, int normalise_na);
RcppExport SEXP haven_df_parse_sav(SEXP filenameSEXP, SEXP normalise_naSEXP) {
BEGIN_RCPP
    Rcpp::RObject __result;
    Rcpp::RNGScope __rngScope;
    Rcpp::traits::input_parameter< std::string >::type filename(filenameSEXP);
    Rcpp::traits::input_parameter< int >::type normalise_na(normalise_naSEXP);
    __result = Rcpp::wrap(df_parse_sav(filename, normalise_na));
    return __result;
END_RCPP
}
// write_sav_
void write_sav_(List data, std::string path);
RcppExport SEXP haven_write_sav_(SEXP dataSEXP, SEXP pathSEXP) {
BEGIN_RCPP
    Rcpp::RNGScope __rngScope;
    Rcpp::traits::input_parameter< List >::type data(dataSEXP);
    Rcpp::traits::input_parameter< std::string >::type path(pathSEXP);
    write_sav_(data, path);
    return R_NilValue;
END_RCPP
}
// write_dta_
void write_dta_(List data, std::string path);
RcppExport SEXP haven_write_dta_(SEXP dataSEXP, SEXP pathSEXP) {
BEGIN_RCPP
    Rcpp::RNGScope __rngScope;
    Rcpp::traits::input_parameter< List >::type data(dataSEXP);
    Rcpp::traits::input_parameter< std::string >::type path(pathSEXP);
    write_dta_(data, path);
    return R_NilValue;
END_RCPP
}
