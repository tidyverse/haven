#' Create a labelled vector.
#'
#' A labelled vector is a common data structure in other statistical
#' environments, allowing you to assign text labels to specific values.
#' This class makes it possible to import such labelled vectors in to R
#' without loss of fidelity. This class provides few methods, as I
#' expect you'll coerce to a standard R class (e.g. a \code{\link{factor}})
#' soon after importing.
#'
#' \code{as_factor()} converts to a factor.
#'
#' @param x A vector to label. Must be either numeric (integer or double) or
#'   character.
#' @param labels A named vector. The vector should be the same type as
#'   \code{x}. Unlike factors, labels don't need to be exhaustive: only a fraction
#'   of the values might be labelled.
#' @param levels How to create the levels of the generated factor:
#'
#'   \itemize{
#'   \item "default": uses labels where available, otherwise the values.
#'   \item "both": like "default", but pastes together the level and value
#'   \item "label": use only the labels; unlabelled values become \code{NA}
#'   \item "values: use only the values
#'   }
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
#' # Other statistical software supports multiple types of missing values
#' s3 <- labelled(c("M", "M", "F", "X", "N/A"),
#'   c(Male = "M", Female = "F", Refused = "X", "Not applicable" = "N/A")
#' )
#' s3
#' as_factor(s3)
#'
#' # Often when you have a partially labelled numeric vector, labelled values
#' # are special types of missing. Use zap_labels to replace labels with missing
#' # values
#' x <- labelled(c(1, 2, 1, 2, 10, 9), c(Unknown = 9, Refused = 10))
#' zap_labels(x)
labelled <- function(x, labels) {
  if (!is.numeric(x) && !is.character(x)) {
    stop("`x` must be a numeric or a character vector", call. = FALSE)
  }
  if (!is_coercible(x, labels)) {
    stop("`x` and `labels` must be same type", call. = FALSE)
  }
  if (is.null(names(labels))) {
    stop("`labels` must have names", call. = FALSE)
  }

  structure(x,
    labels = labels,
    class = "labelled"
  )
}

is_coercible <- function(x, labels) {
  if (typeof(x) == typeof(labels)) {
    return(TRUE)
  }

  if (is.numeric(x) && is.numeric(labels)) {
    return(TRUE)
  }

  FALSE
}

#' @export
#' @rdname labelled
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
  print(xx, quote = FALSE)
  print_labels(x)

  invisible()
}

#' Print the labels of a labelled vector
#'
#' This is a convenience function, useful to explore the variables of
#' a newly imported dataset.
#' @param x A labelled vector
#' @param name The name of the vector (optional)
#' @export
#' @examples
#' s1 <- labelled(c("M", "M", "F"), c(Male = "M", Female = "F"))
#' s2 <- labelled(c(1, 1, 2), c(Male = 1, Female = 2))
#' labelled_df <- tibble::data_frame(s1, s2)
#'
#' for (var in names(labelled_df)) {
#'   print_labels(labelled_df[[var]], var)
#' }
print_labels <- function(x, name = NULL) {
  if (!is.labelled(x)) {
    stop("x must be a labelled vector", call. = FALSE)
  }
  cat("\nLabels:", name, "\n", sep = "")

  labels <- attr(x, "labels")
  lab_df <- data.frame(value = unname(labels), label = names(labels))
  print(lab_df, row.names = FALSE)

  invisible(x)
}

#' @export
as.data.frame.labelled <- function(x, ...) {
  df <- list(x)
  names(df) <- deparse(substitute(x))
  class(df) <- "data.frame"
  attr(df, "row.names") <- .set_row_names(length(x))

  df
}

#' @param ordered If \code{TRUE} for ordinal factors, \code{FALSE} (the default)
#'   for nominal factors.
#' @rdname labelled
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

label_length <- function(x) {
  if (!is.labelled(x)) {
    0L
  } else {
    max(nchar(names(attr(x, "labels"))))
  }
}
