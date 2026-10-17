#' Remove display width attributes
#'
#' @description
#' `r lifecycle::badge("deprecated")`
#'
#' This function is deprecated because haven now stores display widths in the
#' `haven_meta` attribute (see [meta()]). Please use [zap_meta()] instead.
#'
#' `zap_widths()` only removes the legacy `display_width` attribute.
#'
#' @param x A vector or data frame.
#' @family zappers
#' @export
#' @examples
#' x <- structure(1:3, display_width = 10)
#'
#' # Old:
#' zap_widths(x)
#' # New:
#' zap_meta(x)
zap_widths <- function(x) {
  lifecycle::deprecate_warn("2.6.0", "zap_widths()", "zap_meta()")
  UseMethod("zap_widths")
}

#' @export
zap_widths.default <- function(x) {
  attr(x, "display_width") <- NULL

  x
}

#' @export
zap_widths.data.frame <- function(x) {
  x[] <- lapply(x, zap_widths.default)
  x
}
