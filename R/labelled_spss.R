#' Labelled vectors for SPSS
#'
#' This class is only used when \code{user_na = TRUE} in
#' \code{\link{read_sav}()}. It is similar to the \code{\link{labelled}} class
#' but it also models SPSS's user-defined missings, which can be up to
#' three distinct values, or for numeric vectors a range.
#'
#' @param na_values A vector of values that should also be considered as missing.
#' @param na_range A numeric vector of length two giving the extents of
#'   the range. Use \code{-Inf} and \code{Inf} if you want the range to be
#'   open ended.
#' @inheritParams labelled
#' @export
#' @examples
#' x1 <- labelled_spss(1:10, c(Good = 1, Bad = 8), na_values = c(9, 10))
#' x1 <- labelled_spss(1:10, c(Good = 1, Bad = 8), na_range = c(9, Inf))
labelled_spss <- function(x, labels, na_values = NULL, na_range = NULL) {
  if (!is.null(na_values)) {
    if (!is_coercible(x, na_values)) {
      stop("`x` and `na_values` must be same type", call. = FALSE)
    }
  }
  if (!is.null(na_range)) {
    if (!is.numeric(na_range) || length(na_range) != 2) {
      stop("`na_range` must be a numeric vector of length two.", call. = FALSE)
    }
  }

  structure(
    labelled(x, labels),
    na_values = na_values,
    na_range = na_range,
    class = c("labelled_spss", "labelled")
  )
}
