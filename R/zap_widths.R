#' Remove display width attributes
#'
#' To provide some mild support for round-tripping variables between SPSS
#' and R, haven stores display widths in an attribute: `display_width`. If this
#' causes problems for your code, you can get rid of them with `zap_widths`.
#'
#' @param x A vector or data frame.
#' @return The input with `display_width` attributes removed.
#' @family zappers
#' @export
#' @examples
#' x <- labelled(c(1, 2, 3), c(yes = 1, no = 3))
#' attr(x, "display_width") <- 10
#' zap_widths(x)
#'
#' # Also works with data frames
#' df <- tibble::tibble(x = x, y = 4:6)
#' zap_widths(df)
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
