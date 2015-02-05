#' @useDynLib haven
#' @importFrom Rcpp sourceCpp
NULL

#' Read SAS7BDAT files.
#'
#' @param path Path to file
#' @export
#' @examples
#' read_sas("http://crn.cancer.gov/resources/ctcodes-procedures.sas7bdat")
read_sas <- function(path) {
  df_parse_sas(clean_path(path))
}

#' Read SPSS (POR and SAV) files.
#'
#' @param path Path to file
#' @name read_spss
NULL

#' @export
#' @rdname read_spss
read_por <- function(path) {
  df_parse_por(clean_path(path))
}

#' @export
#' @rdname read_spss
read_sav <- function(path) {
  df_parse_sav(clean_path(path))
}


#' Read Stata DTA files.
#'
#' @param path Path to file
#' @export
read_dta <- function(path) {
  df_parse_dta(clean_path(path))
}
