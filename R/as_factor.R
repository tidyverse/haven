#' Convert labelled vectors to factors
#'
#' The base function `as.factor()` is not a generic, but [forcats::as_factor()]
#' is. haven provides `as_factor()` methods for [labelled()] and
#' [labelled_spss()] vectors, and data frames. By default, when applied to a
#' data frame, it only affects labelled columns.
#'
#' Includes methods for both class `haven_labelled` and `labelled`
#' for backward compatibility.
#'
#' @param x Object to coerce to a factor.
#' @param ... Other arguments passed down to method.
#' @param only_labelled Only apply to labelled columns?
#' @export
#' @examples
#' x <- labelled(sample(5, 10, replace = TRUE), c(Bad = 1, Good = 5))
#'
#' # Default method uses values where available
#' as_factor(x)
#' # You can also extract just the labels
#' as_factor(x, levels = "labels")
#' # Or just the values
#' as_factor(x, levels = "values")
#' # Or combine value and label
#' as_factor(x, levels = "both")
#'
#' # as_factor() will preserve SPSS missing values from values and ranges
#' y <- labelled_spss(1:10, na_values = c(2, 4), na_range = c(8, 10))
#' as_factor(y)
#' # use zap_missing() first to convert to NAs
#' zap_missing(y)
#' as_factor(zap_missing(y))
#' @importFrom forcats as_factor
#' @export
#' @name as_factor
NULL

#' @rdname as_factor
#' @export
as_factor.data.frame <- function(x, ..., only_labelled = TRUE) {
  if (only_labelled) {
    labelled <- vapply(x, is.labelled, logical(1))
    x[labelled] <- lapply(x[labelled], as_factor, ...)
  } else {
    x[] <- lapply(x, as_factor, ...)
  }

  x
}

#' @param ordered If `TRUE` create an ordered (ordinal) factor, if
#'   `FALSE` (the default) create a regular (nominal) factor.
#' @param levels How to create the levels of the generated factor:
#'
#'   * "default": uses labels where available, otherwise the values.
#'     Labels are sorted by value.
#'   * "both": like "default", but pastes together the level and value
#'   * "label": use only the labels; unlabelled values become `NA`
#'   * "values": use only the values
#' @rdname as_factor
#' @export
as_factor.haven_labelled <- function(x, levels = c("default", "labels", "values", "both"),
                                     ordered = FALSE, ...) {
  levels <- match.arg(levels)
  label <- attr(x, "label", exact = TRUE)
  labels <- attr(x, "labels")

  if (levels %in% c("default", "both")) {
    if (levels == "both") {
      names(labels) <- paste0("[", labels, "] ", names(labels))
    }

    # Replace each value with its label
    vals <- unique(vec_data(x))
    levs <- replace_with(vals, unname(labels), names(labels))
    # Ensure all labels are preserved
    levs <- sort(c(stats::setNames(vals, levs), labels), na.last = TRUE)
    levs <- unique(names(levs))

    x <- replace_with(vec_data(x), unname(labels), names(labels))

    x <- factor(x, levels = levs, ordered = ordered)
  } else if (levels == "labels") {
    levs <- unname(labels)
    labs <- names(labels)
    x <- replace_with(vec_data(x), levs, labs)
    x <- factor(x, unique(labs), ordered = ordered)
  } else if (levels == "values") {
    if (all(x %in% labels)) {
      levels <- unname(labels)
    } else {
      levels <- sort(unique(vec_data(x)))
    }
    x <- factor(vec_data(x), levels, ordered = ordered)
  }

  structure(x, label = label)
}

#' @export
#' @rdname as_factor
as_factor.labelled <- as_factor.haven_labelled

replace_with <- function(x, from, to) {
  stopifnot(length(from) == length(to))

  out <- x
  # First replace regular values
  matches <- match(x, from, incomparables = NA)
  if (anyNA(matches)) {
    out[!is.na(matches)] <- to[matches[!is.na(matches)]]
  } else {
    out <- to[matches]
  }

  # Then tagged missing values
  tagged <- is_tagged_na(x)
  if (!any(tagged)) {
    return(out)
  }

  matches <- match(na_tag(x), na_tag(from), incomparables = NA)

  # Could possibly be faster to use anyNA(matches)
  out[!is.na(matches)] <- to[matches[!is.na(matches)]]
  out
}
