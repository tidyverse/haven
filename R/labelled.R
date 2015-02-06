#' Create a labelled vector.
#'
#' A labelled vector is a common data structure in other statistical
#' environments. This class makes it possible to import such labelled vectors
#' in to without loss of fidelity. This class provides few methods, as I
#' expect you'll coerce to a standard R class (e.g. a \code{\link{factor}})
#' soon after importing.
#'
#' @param x A vector to label. Must be either numeric (integer or double) or
#'   character.
#' @param labels A named vector. The vector should be the same type as
#'   x. Unlike factors, labels don't need to be exhaustive: only a fraction
#'   of the values might be labelled.
#' @export
#' @examples
#' sex <- labelled(c("M", "M", "F"), c(M = "Male", F = "Female"))
labelled <- function(x, labels) {
  if (!is.numeric(x) && !is.character(x)) {
    stop("`x` must be either numeric or a character vector", call. = FALSE)
  }
  if (typeof(x) != typeof(labels)) {
    stop("`x` and `labels` must be same type", call. = FALSE)
  }
  if (is.null(labels)) {
    stop("`labels` must be a named vector", call. = FALSE)
  }

  structure(x,
    labels = labels,
    class = "labelled"
  )
}

#' @export
`[.labelled` <- function(x, ...) {
  labelled(NextMethod(), attr(x, "labels"))
}

#' @export
print.labelled <- function(x, ...) {
  cat("<Labelled>\n")

  xx <- unclass(x)
  attr(xx, "labels") <- NULL
  print(xx)

  cat("Labels:\n")
  print(attr(x, "labels"))

  invisible()
}
