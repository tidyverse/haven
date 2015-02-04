#' @useDynLib haven
#' @importFrom Rcpp sourceCpp
NULL

#' Read a SAS7BDAT file in R.
#'
#' @param path Path to file
#' @export
read_sas <- function(path) {
  path <- normalizePath(path, mustWork = TRUE)
  sas7bdat_df(path)
}

