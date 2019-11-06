#' Zap labels
#'
#' @description
#' Removes labels, leaving unlabelled vectors as is. Use this if you want to
#' simply drop all `labels` from a data frame. Zapping labels from
#' [labelled_spss()] also removes user-defined missing values,
#' replacing all with `NA`s.
#'
#' This function removes value labels; use [zap_label()] to remove variable
#' labels.
#'
#' @param x A vector or data frame
#' @family zappers
#' @export
#' @examples
#' x1 <- labelled(1:5, c(good = 1, bad = 5))
#' x1
#' zap_labels(x1)
#'
#' x2 <- labelled_spss(c(1:4, 9), c(good = 1, bad = 5), na_values = 9)
#' x2
#' zap_labels(x2)
#'
#' # zap_labels also works with data frames
#' df <- tibble::tibble(x1, x2)
#' df
#' zap_labels(df)
zap_labels <- function(x) {
  UseMethod("zap_labels")
}

#' @export
zap_labels.default <- function(x) {
  x
}

#' @export
zap_labels.haven_labelled <- function(x) {
  attr(x, "labels") <- NULL
  class(x) <- NULL

  x
}

#' @export
zap_labels.haven_labelled_spss <- function(x) {
  x[is.na(x)] <- NA

  attr(x, "labels") <- NULL
  attr(x, "na_values") <- NULL
  attr(x, "na_range") <- NULL
  class(x) <- NULL

  x
}


#' @export
zap_labels.data.frame <- function(x) {
  x[] <- lapply(x, zap_labels)
  x
}
