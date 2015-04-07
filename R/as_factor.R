#' Convert input to a factor.
#'
#' The base function \code{as.factor()} is not a generic, but this variant
#' is. Methods are provided for factors, character vectors and labelled
#' vectors.
#'
#' @param x Object to coerce to a factor.
#' @param ... Other arguments passed down to method.
#' @export
as_factor <- function(x, ...) {
  UseMethod("as_factor")
}

#' @export
as_factor.factor <- function(x, ...) {
  x
}

#' @export
as_factor.character <- function(x, ...) {
  factor(x, ...)
}
