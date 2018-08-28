#' Zap labels
#'
#' Removes labels, leaving unlabelled vectors as is. Use this if you want to
#' simply drop all `labels` from a data frame. Zapping labels from
#' [labelled_spss()] also removes user-defined missing values,
#' replacing all with `NA`s.
#'
#' @param x A vector or data frame
#' @family zappers
#' @export
#' @note This function doesn't remove any `label` attribute(s),
#'       just the `labels` attribute(s). Use \code{\link{zap_label}}
#'       to remove `label` attribute(s).
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
#' df <- tibble::data_frame(x1, x2)
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
