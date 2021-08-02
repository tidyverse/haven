#' Zap label
#'
#' @description
#' Removes label, leaving unlabelled vectors as is. Use this if you want to
#' simply drop all `label` attributes from a data frame.
#'
#' This function removes variable labels; use [zap_labels()] to remove value
#' labels.
#' @param x A vector or data frame
#' @family zappers
#' @export
#' @examples
#' x1 <- labelled(1:5, c(good = 1, bad = 5), label = "rating")
#' x1
#' zap_label(x1)
#'
#' x2 <- labelled_spss(c(1:4, 9), label = "rating", na_values = 9)
#' x2
#' zap_label(x2)
#'
#' # zap_label also works with data frames
#' df <- tibble::tibble(x1, x2)
#' lapply(df, identity)         # label not shown in data.frame print
#' zdf <- zap_label(df)
#' lapply(zdf, identity)        # label removed from each column
zap_label <- function(x) {
  UseMethod("zap_label")
}

#' @export
zap_label.default <- function(x) {
  attr(x, "label") <- NULL
  x
}

#' @export
zap_label.data.frame <- function(x) {
  x[] <- lapply(x, zap_label)
  x
}
