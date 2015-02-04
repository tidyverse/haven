#' @useDynLib haven
#' @importFrom Rcpp sourceCpp
NULL

#' Read a SAS7BDAT file in R.
#'
#' @param path Path to file
#' @export
#' @examples
#' read_sas("http://crn.cancer.gov/resources/ctcodes-procedures.sas7bdat")
read_sas <- function(path) {
  sas7bdat_df(clean_path(path))
}

clean_path <- function(path) {
  if (grepl("^(http|ftp|https)://", path)) {
    tmp <- tempfile()
    download.file(path, tmp, quiet = TRUE, mode = "wb")
    tmp
  } else {
    normalizePath(path, mustWork = TRUE)
  }
}
