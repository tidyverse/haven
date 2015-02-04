#' @useDynLib haven
#' @importFrom Rcpp sourceCpp
NULL

#' @export
read_sas <- function(path) {
  path <- normalizePath(path, mustWork = TRUE)
  sas7bdat_df(path)
}

