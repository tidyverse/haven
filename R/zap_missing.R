#' Zap special missings to regular R missings
#'
#' This is useful if you want to convert tagged missing values from SAS or
#' Stata, or user-defined missings from SPSS, to regular R \code{NA}.
#'
#' @param x A vector or data frame
#' @export
#' @examples
#' x1 <- labelled(
#'   c(1, 5, tagged_na("a", "b")),
#'   c(Unknown = tagged_na("a"), Refused = tagged_na("b"))
#' )
#' x1
#' zap_missing(x1)
#'
#' x2 <- labelled_spss(
#'   c(1, 2, 1, 99),
#'   c(missing = 99),
#'   na_value = 99
#' )
#' x2
#' zap_missing(x2)
#'
#' # You can also apply to data frames
#' df <- tibble::data_frame(x1, x2, y = 4:1)
#' df
#' zap_missing(df)
zap_missing <- function(x) {
  UseMethod("zap_missing")
}

#' @export
zap_missing.default <- function(x) {
  x
}

#' @export
zap_missing.labelled <- function(x) {
  x[is.na(x)] <- NA

  labels <- attr(x, "labels")
  labels <- labels[!is.na(labels)]
  attr(x, "labels") <- labels

  x
}

#' @export
zap_missing.labelled_spss <- function(x) {
  is.na(x) <- is.na(x)

  attr(x, "na_values") <- NULL
  attr(x, "na_range") <- NULL
  class(x) <- "labelled"

  x
}

#' @export
zap_missing.data.frame <- function(x) {
  x[] <- lapply(x, zap_missing)
  x
}
