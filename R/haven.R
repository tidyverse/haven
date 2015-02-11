#' @useDynLib haven
#' @importFrom Rcpp sourceCpp
NULL

#' Read SAS files.
#'
#' This supports both b7dat files and the accompanying b7cat files
#' that are SAS's equivalent of factor labels.
#'
#' @param b7dat,b7cat Path to data and catalog files. If the path is a URL, the
#'   file will be first download to a temporary location before reading.
#' @return A data frame with additional "tbl_df" and "tbl" classes, which
#'   improve printing if dplyr is loaded.
#' @export
#' @examples
#' read_sas("http://crn.cancer.gov/resources/ctcodes-procedures.sas7bdat")
read_sas <- function(b7dat, b7cat = NULL) {
  df_parse_sas(clean_path(b7dat), clean_path(b7cat))
}

#' Read SPSS (POR and SAV) files.
#'
#' Variable labels are stored in the "label" attribute in each column.
#'
#' @param path Path to data If the path is a URL, the file will be first
#'   downloaded to a temporary location before reading.
#' @return A data frame with additional "tbl_df" and "tbl" classes, which
#'   improve printing if dplyr is loaded.
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
#' @inheritParams read_spss
#' @return A data frame with additional "tbl_df" and "tbl" classes, which
#'   improve printing if dplyr is loaded.
#' @export
read_dta <- function(path) {
  df_parse_dta(clean_path(path))
}
