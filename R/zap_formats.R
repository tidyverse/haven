#' Remove format attributes
#'
#' To provide some mild support for round-tripping variables between Stata/SPSS
#' and R, haven stores variable formats in an attribute: `format.stata`,
#' `format.spss`, or `format.sas`. If this causes problems for your
#' code, you can get rid of them with `zap_formats`.
#'
#' @param x A vector or data frame.
#' @family zappers
#' @export
zap_formats <- function(x) {
  UseMethod("zap_formats")
}

#' @export
zap_formats.default <- function(x) {
  attr(x, "format.spss") <- NULL
  attr(x, "format.sas") <- NULL
  attr(x, "format.stata") <- NULL

  x
}

#' @export
zap_formats.data.frame <- function(x) {
  x[] <- lapply(x, zap_formats)
  x
}
