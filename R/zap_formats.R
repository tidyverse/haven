#' Remove format attributes
#'
#' @description
#' `r lifecycle::badge("deprecated")`
#'
#' This function is deprecated because haven now stores formats in the
#' `haven_meta` attribute (see [meta()]). Please use [zap_meta()] instead.
#'
#' `zap_formats()` only removes the legacy `format.stata`, `format.spss`, and
#' `format.sas` attributes.
#'
#' @param x A vector or data frame.
#' @family zappers
#' @export
#' @examples
#' x <- structure(1:3, format.sas = "BEST12.")
#'
#' # Old:
#' zap_formats(x)
#' # New:
#' zap_meta(x)
zap_formats <- function(x) {
  lifecycle::deprecate_warn("2.6.0", "zap_formats()", "zap_meta()")
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
  x[] <- lapply(x, zap_formats.default)
  x
}
