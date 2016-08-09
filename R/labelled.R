#' Create a labelled vector.
#'
#' A labelled vector is a common data structure in other statistical
#' environments, allowing you to assign text labels to specific values.
#' This class makes it possible to import such labelled vectors in to R
#' without loss of fidelity. This class provides few methods, as I
#' expect you'll coerce to a standard R class (e.g. a \code{\link{factor}})
#' soon after importing.
#'
#' @param x A vector to label. Must be either numeric (integer or double) or
#'   character.
#' @param labels A named vector. The vector should be the same type as
#'   \code{x}. Unlike factors, labels don't need to be exhaustive: only a fraction
#'   of the values might be labelled.
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
print.labelled <- function(x, ..., digits = getOption("digits")) {
  cat("<Labelled ", typeof(x), ">\n", sep = "")

  if (is.double(x)) {
    print_tagged_na(x, digits = digits)
  } else {
    xx <- x
    attributes(xx) <- NULL
    print.default(xx, quote = FALSE)
  }

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
  labels <- attr(x, "labels")
  if (length(labels) == 0) {
    return(invisible(x))
  }

  cat("\nLabels:", name, "\n", sep = "")

  value <- if (is.double(labels)) format_tagged_na(labels) else unname(labels)

  lab_df <- data.frame(value = value, label = names(labels))
  print(lab_df, row.names = FALSE)

  invisible(x)
}

#' @export
as.data.frame.labelled <- function(x, ...) {
  df <- list(x)
  class(df) <- "data.frame"
  attr(df, "row.names") <- .set_row_names(length(x))

  df
}

label_length <- function(x) {
  if (!is.labelled(x)) {
    0L
  } else {
    max(nchar(names(attr(x, "labels"))))
  }
}

#' @export
#' @importFrom tibble type_sum
type_sum.labelled <- function(x) {
  paste0(tibble::type_sum(unclass(x)), "+lbl")
}
