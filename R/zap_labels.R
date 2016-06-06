#' Zap labels
#'
#' Removes labels, leaving unlabelled vectors as is. Use this if you want to
#' simply drop all labelling from a data frame.
#'
#' @param x A vector or data frame
#' @family zappers
#' @export
zap_labels <- function(x) {
  UseMethod("zap_labels")
}

#' @export
zap_labels.default <- function(x) {
  x
}

#' @export
zap_labels.labelled <- function(x) {
  attr(x, "labels") <- NULL
  class(x) <- NULL

  x
}

#' @export
zap_labels.data.frame <- function(x) {
  x[] <- lapply(x, zap_labels)
  x
}
