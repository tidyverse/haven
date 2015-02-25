#' Convert empty strings into missing values.
#'
#' @param x A character vector
#' @return A character vector with empty strings replaced by missing values.
#' @export
#' @examples
#' x <- c("a", "", "c")
#' zap_empty(x)
zap_empty <- function(x) {
  stopifnot(is.character(x))

  x[x == ""] <- NA
  x
}
