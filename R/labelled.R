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
#' @param label_na Optionally, a logical vector that should be as long as
#'   \code{labels} and indicate which labels correspond to
#'   pseudo-missing values.
#' @param x_na Optionally, a logical vector that should be as long
#'   as \code{x} and indicate which values are pseudo-missing. This
#'   missingness pattern can be broader than the one implied by
#'   \code{label_na}. An error is issued when the patterns are not
#'   consistent with each other.
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
#' # Other statistical software supports pseudo-missing values in
#' # addition to regular NAs. You can specify which labels define
#' # pseudo-missing values with a logical vector.
#' s3 <- labelled(
#'   c("M", "M", "F", "X", "N/A"),
#'   labels = c(Male = "M", Female = "F", Refused = "X", "Not applicable" = "N/A"),
#'   label_na = c(FALSE, FALSE, TRUE, TRUE)
#' )
#' s3
#' as_factor(s3)
#'
#' # Alternatively, you can specify missingness with `x_na`
#' s4 <- labelled(
#'   c("M", "M", "F", "X", "N/A"),
#'   x_na = c(FALSE, FALSE, FALSE, TRUE, TRUE)
#' )
#' s4
#'
#' # When you supply both, it is possible to declare values not
#' # mentioned in `labels` as missing. In this case make sure the
#' # patterns are consistent or labelled() will fail
#' s5 <- labelled(
#'   c("Z", "M", "M", "F", "X", "N/A"),
#'   labels = c(Male = "M", Female = "F", Refused = "X", "Not applicable" = "N/A"),
#'   label_na = c(FALSE, FALSE, TRUE, TRUE),
#'   x_na = c(TRUE, FALSE, FALSE, FALSE, TRUE, TRUE)
#' )
#' s5
#'
#' # Often when you have a partially labelled numeric vector, labelled values
#' # are special types of missing. Use XXX to replace labels with missing
#' # values
#' x <- labelled(c(1, 2, 1, 2, 10, 9), c(Unknown = 9, Refused = 10))
#' zap_labels(x)
labelled <- function(x, labels = NULL, label_na = NULL, x_na = NULL) {
  if (!is.numeric(x) && !is.character(x)) {
    stop("`x` must be either numeric or a character vector", call. = FALSE)
  }
  if (!is.null(labels) && typeof(x) != typeof(labels)) {
    stop("`x` and `labels` must be same type", call. = FALSE)
  }

  if (!is.null(x_na) && (!is.logical(x_na) || length(x_na) != length(x))) {
    stop("`x_na` must be a logical vector the same length as `x`",
      call. = FALSE)
  }
  if (is.null(label_na)) {
    label_na <- rep(FALSE, length(labels))
  } else {
    if (!is.logical(label_na) || length(label_na) != length(labels)) {
      stop("`label_na` must be a logical vector the same length as `labels`",
        call. = FALSE)
    }
    missings <- x %in% labels[label_na]
    if (is.null(x_na)) {
      x_na <- missings
    } else {
      check_missingness(x_na, missings)
    }
  }

  structure(x,
    labels = labels,
    label_na = label_na,
    x_na = x_na,
    class = "labelled"
  )
}

check_missingness <- function(x_na, labels_missings) {
  if (any(labels_missings[x_na == FALSE])) {
    stop("Inconsitent missing patterns. Check `x_na` and `label_na`",
      call. = FALSE)
  }
}

is.labelled <- function(x) inherits(x, "labelled")

#' @export
`[.labelled` <- function(x, ...) {
  x_na <- attr(x, "x_na")[...]
  labelled(NextMethod(), attr(x, "labels"), attr(x, "label_na"), x_na)
}

#' @export
print.labelled <- function(x, ...) {
  cat("<Labelled>\n")

  xx <- unclass(x)
  attr(xx, "labels") <- NULL
  attr(xx, "label_na") <- NULL
  attr(xx, "x_na") <- NULL
  print(xx, quote = FALSE)

  na_n <- sum(attr(x, "x_na"))
  if (na_n > 0) {
    cat(paste0("\n", na_n, " pseudo-missing values\n"))
  }

  cat("\nLabels: ")
  if (is.null(attr(x, "labels"))) {
    cat("NULL\n")
  } else {
    cat("\n")
    labels <- attr(x, "labels")
    lab_df <- data.frame(value = unname(labels), label = names(labels), label_na = attr(x, "label_na"))
    print(lab_df, row.names = FALSE)
  }

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


#' @param ordered If \code{TRUE} for ordinal factors, \code{FALSE} (the default)
#'   for nominal factors.
#' @param drop_na If \code{TRUE}, the default, all types are missing value are
#'   converted into \code{NA}. If \code{FALSE}, missing values will be left as
#'   their original codes.
#' @rdname labelled
#' @export
as_factor.labelled <- function(x, levels = c("labels", "values"),
                               ordered = FALSE, drop_na = TRUE, ...) {
  levels <- match.arg(levels)
  if (is.null(attr(x, "labels"))) {
    labels <- autoname(unique(as.character(x)))
    label_na <- rep(FALSE, length(labels))
  } else {
    labels <- attr(x, "labels")
    label_na <- attr(x, "label_na")
  }
  x_na <- attr(x, "x_na")

  if (drop_na) {
    if (!is.null(x_na)) {
      nonmissing_labels <- unique(x[!x_na])
      labels_pos <- vapply(nonmissing_labels, match, numeric(1), labels)
      labels <- labels[na.omit(labels_pos)]
    } else if (!is.null(labels)){
      labels <- labels[!label_na]
    }
    x <- normalise_na(x)
  }

  if (is.character(x)) {
    levs <- unname(labels)
    labs <- switch(levels,
      labels = names(labels),
      values = levs
    )
    factor(x, levs, labels = labs, ordered = ordered)
  } else {
    factor(x, levels = unname(labels), labels = names(labels))
  }
}

#' @export
#' @rdname labelled
zap_labels <- function(x) {
  if (!is.labelled(x))
    return(x)

  labelled <- x %in% attr(x, "labels")
  attr(x, "labels") <- NULL
  attr(x, "label_na") <- NULL
  class(x) <- NULL

  x[labelled] <- NA
  x
}

#' @export
#' @rdname labelled
is_missing <- function(x) {
  if (is.labelled(x) && !is.null(attr(x, "x_na"))) {
    attr(x, "x_na")
  } else {
    rep_len(FALSE, length(x))
  }
}

#' @export
#' @rdname labelled
normalise_na <- function(x) {
  if (is.labelled(x)) {
    missings <- is_missing(x)
    attr(x, "x_na") <- NULL
    x[missings] <- NA
  }

  x
}

#' @export
#' @rdname labelled
normalize_na <- normalise_na
