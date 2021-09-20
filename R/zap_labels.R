#' Zap value labels
#'
#' @description
#' Removes value labels, leaving unlabelled vectors as is. Use this if you
#' want to simply drop all `labels` from a data frame.
#'
#' Zapping labels from [labelled_spss()] also removes user-defined missing
#' values by default, replacing with standard `NA`s. Use the `user_na` argument
#' to override this behaviour.
#'
#' @param x A vector or data frame
#' @param user_na If `FALSE`, the default, `zap_labels()` will convert
#'   [labelled_spss()] user-defined missing values to `NA`. If `TRUE` they
#'   will be treated like normal values.
#' @param ... Other arguments passed down to method.
#' @family zappers
#' @seealso [zap_label()] to remove variable labels.
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
#' # Keep the user defined missing values
#' zap_labels(x2, user_na = TRUE)
#'
#' # zap_labels also works with data frames
#' df <- tibble::tibble(x1, x2)
#' df
#' zap_labels(df)
zap_labels <- function(x, ...) {
  UseMethod("zap_labels")
}

#' @export
zap_labels.default <- function(x, ...) {
  x
}

#' @export
zap_labels.haven_labelled <- function(x, ...) {
  attr(x, "labels") <- NULL
  class(x) <- NULL

  x
}

#' @rdname zap_labels
#' @export
zap_labels.haven_labelled_spss <- function(x, ..., user_na = FALSE) {
  if (isFALSE(user_na)) {
    x[is.na(x)] <- NA
  }

  attr(x, "labels") <- NULL
  attr(x, "na_values") <- NULL
  attr(x, "na_range") <- NULL
  class(x) <- NULL

  x
}


#' @export
zap_labels.data.frame <- function(x, ...) {
  x[] <- lapply(x, zap_labels, ...)
  x
}
