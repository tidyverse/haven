#' Remove display width attributes
#'
#' To provide some mild support for round-tripping variables between SPSS
#' and R, haven stores display widths in an attribute: \code{display_width}. If this
#' causes problems for your code, you can get rid of them with \code{zap_widths}.
#'
#' @param x A vector or data frame.
#' @family zappers
#' @export
zap_widths <- function(x) {
  UseMethod("zap_widths")
}

#' @export
zap_widths.default <- function(x) {
  attr(x, "display_width") <- NULL

  x
}

#' @export
zap_widths.data.frame <- function(x) {
  x[] <- lapply(x, zap_widths)
  x
}
