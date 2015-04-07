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
#' @param levels When coercing a labelled character vector to a factor, you
#'   can choose whether to use the labels or the values as the factor levels.
#' @param ordered If \code{TRUE} for ordinal factors, \code{FALSE} (the default)
#'   for nominal factors.
#' @param ... Ignored
#' @export
#' @examples
#' s1 <- labelled(c("M", "M", "F"), c(Male = "M", Female = "F"))
#' s2 <- labelled(c(1, 1, 2), c(Male = 1, Female = 2))
#'
#' # Unfortunately it's not possible to make as.factor work for labelled objects
#' # so instead use as_factor. This works for all types of labelled vectors.
#' as_factor(s1)
#' as_factor(s1, labels = "values")
#' as_factor(s2)
#'
#' # Often when you have a partially labelled numeric vector, labelled values
#' # are special types of missing. Use XXX to replace labels with missing
#' # values
#' x <- labelled(c(1, 2, 1, 2, 10, 9), c(Unknown = 9, Refused = 10))
#' zap_labels(x)
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

is.labelled <- function(x) inherits(x, "labelled")

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

#' @export
as.data.frame.labelled <- function(x, ...) {
  df <- list(x)
  names(df) <- deparse(substitute(x))
  class(df) <- "data.frame"
  attr(df, "row.names") <- .set_row_names(length(x))

  df
}


#' @rdname labelled
#' @export
as_factor.labelled <- function(x, levels = c("labels", "values"),
                               ordered = FALSE, ...) {
  levels <- match.arg(levels)

  if (is.character(x)) {
    levs <- unname(attr(x, "labels"))
    labs <- switch(levels,
      labels = names(attr(x, "labels")),
      values = levs
    )
    factor(x, levs, labels = labs, ordered = ordered)
  } else {
    labs <- attr(x, "labels")
    factor(match(x, labs), levels = unname(labs), labels = names(labs))
  }

}

#' @export
#' @rdname labelled
zap_labels <- function(x) {
  if (is.labelled(x))
    return(x)

  labelled <- x %in% attr(x, "labels")
  attr(x, "labels") <- NULL
  class(x) <- NULL

  x[labelled] <- NA
  x
}
