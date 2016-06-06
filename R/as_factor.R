#' Convert input to a factor.
#'
#' The base function \code{as.factor()} is not a generic, but this variant
#' is. Methods are provided for factors, character vectors and labelled
#' vectors.
#'
#' @param x Object to coerce to a factor.
#' @param ... Other arguments passed down to method.
#' @export
#' @examples
#' x <- labelled(sample(5, 10, replace = TRUE), c(Bad = 1, Good = 5))
#'
#' # Default method uses values where available
#' as_factor(x)
#' # You can also extract just the labels
#' as_factor(x, "labels")
#' # Or just the values
#' as_factor(x, "values")
#' # Or combine value and label
#' as_factor(x, "both")
as_factor <- function(x, ...) {
  UseMethod("as_factor")
}

#' @rdname as_factor
#' @export
as_factor.factor <- function(x, ...) {
  x
}

#' @rdname as_factor
#' @export
as_factor.character <- function(x, ...) {
  factor(x, ...)
}

#' @param ordered If \code{TRUE} create an ordered (ordinal) factor, if
#'   \code{FALSE} (the default) create a regular (nominal) factor.
#' @param levels How to create the levels of the generated factor:
#'
#'   \itemize{
#'   \item "default": uses labels where available, otherwise the values.
#'   \item "both": like "default", but pastes together the level and value
#'   \item "label": use only the labels; unlabelled values become \code{NA}
#'   \item "values: use only the values
#'   }
#' @rdname as_factor
#' @export
as_factor.labelled <- function(x, levels = c("default", "labels", "values", "both"),
                               ordered = FALSE, ...) {
  levels <- match.arg(levels)
  labels <- attr(x, "labels")

  if (levels == "default" || levels == "both") {
    if (levels == "both") {
      names(labels) <- paste0("[", labels, "] ", names(labels))
    }

    # Replace each value with its label
    levs <- replace_with(sort(unique(x)), unname(labels), names(labels))
    x <- replace_with(x, unname(labels), names(labels))

    factor(x, levels = levs, ordered = ordered)
  } else {
    levs <- unname(labels)
    labs <- switch(levels,
      labels = names(labels),
      values = levs
    )
    factor(x, levs, labels = labs, ordered = ordered)
  }

}

replace_with <- function(x, from, to) {
  matches <- match(x, from)
  ifelse(is.na(matches), as.character(x), to[matches])
}
