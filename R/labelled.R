#' Create a labelled vector.
#'
#' A labelled vector is a common data structure in other statistical
#' environments, allowing you to assign text labels to specific values.
#' This class makes it possible to import such labelled vectors in to R
#' without loss of fidelity. This class provides few methods, as I
#' expect you'll coerce to a standard R class (e.g. a [factor()])
#' soon after importing.
#'
#' @param x A vector to label. Must be either numeric (integer or double) or
#'   character.
#' @param labels A named vector or `NULL`. The vector should be the same type
#'   as `x`. Unlike factors, labels don't need to be exhaustive: only a fraction
#'   of the values might be labelled.
#' @param label A short, human-readable description of the vector.
#' @export
#' @examples
#' s1 <- labelled(c("M", "M", "F"), c(Male = "M", Female = "F"))
#' s2 <- labelled(c(1, 1, 2), c(Male = 1, Female = 2))
#' s3 <- labelled(c(1, 1, 2), c(Male = 1, Female = 2),
#'                label="Assigned sex at birth")
#'
#' # Unfortunately it's not possible to make as.factor work for labelled objects
#' # so instead use as_factor. This works for all types of labelled vectors.
#' as_factor(s1)
#' as_factor(s1, levels = "values")
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
labelled <- function(x = double(), labels = NULL, label = NULL) {
  x <- vec_data(x)
  labels <- vec_cast_named(labels, x, x_arg = "labels", to_arg = "x")
  validate_labelled(new_labelled(x, labels = labels, label = label))
}

new_labelled <- function(x = double(), labels = NULL, label = NULL,
                         ..., class = character()) {
  if (!is.numeric(x) && !is.character(x)) {
    abort("`x` must be a numeric or a character vector.")
  }
  if (!is.null(labels) && !vec_is(labels, x)) {
    abort("`labels` must be same type as `x`.")
  }
  if (!is.null(label) && (!is.character(label) || length(label) != 1)) {
    abort("`label` must be a character vector of length one.")
  }

  new_vctr(x,
    labels = labels,
    label = label,
    ...,
    class = c(class, "haven_labelled"),
    inherit_base_type = TRUE
  )
}

validate_labelled <- function(x) {
  labels <- attr(x, "labels")
  if (!is.null(labels) && is.null(names(labels))) {
    abort("`labels` must have names.")
  }
  if (any(duplicated(stats::na.omit(labels)))) {
    abort("`labels` must be unique.")
  }

  x
}

#' @export
as.character.haven_labelled <- function(x, ...) {
  as.character(vec_data(x))
}

#' @export
levels.haven_labelled <- function(x) {
  NULL
}

# TODO: https://github.com/r-lib/vctrs/issues/1108
#' @export
`names<-.haven_labelled` <- function(x, value) {
  attr(x, "names") <- value
  x
}

#' @importFrom stats median
#' @export
median.haven_labelled <- function(x, na.rm = TRUE, ...) {
  if (is.character(x)) {
    abort("Can't compute median of labelled<character>")
  }
  median(vec_data(x), na.rm = TRUE, ...)
}

#' @importFrom stats quantile
#' @export
quantile.haven_labelled <- function(x, ...) {
  if (is.character(x)) {
    abort("Can't compute median of labelled<character>")
  }
  quantile(vec_data(x), ...)
}

#' @export
summary.haven_labelled <- function(object, ...) {
  summary(vec_data(object), ...)
}

# Formatting --------------------------------------------------------------

#' @export
vec_ptype_full.haven_labelled <- function(x, ...) {
  paste0("labelled<", vec_ptype_full(vec_data(x)), ">")
}

#' @export
vec_ptype_abbr.haven_labelled <- function(x, ...) {
  paste0(vec_ptype_abbr(vec_data(x)), "+lbl")
}

#' @export
obj_print_header.haven_labelled <- function(x, ...) {
  cat_line("<", vec_ptype_full(x), "[", vec_size(x), "]>", get_labeltext(x))
  invisible(x)
}

# Convenience function for getting the label with
# with a prefix (if label is not empty), used for
# printing 'label' and 'labelled_spss' vectors
get_labeltext <- function(x, prefix=": ") {
  label = attr(x, "label", exact = TRUE)
  if(!is.null(label)) {
    paste0(prefix, label)
  }
}

#' @export
format.haven_labelled <- function(x, ..., digits = getOption("digits")) {
  if (is.double(x)) {
    format_tagged_na(x, digits = digits)
  } else {
    format(vec_data(x), ...)
  }
}

#' @export
obj_print_footer.haven_labelled <- function(x, ...) {
  print_labels(x)
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
#' labelled_df <- tibble::tibble(s1, s2)
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

  lab_df <- data.frame(value = value, label = names(labels), row.names = NULL)
  print(lab_df, row.names = FALSE)

  invisible(x)
}


# Type system -------------------------------------------------------------

# Import to avoid R CMD check NOTE
#' @importFrom methods setOldClass
setOldClass(c("haven_labelled", "vctrs_vctr"))

#' @export
#' @rdname labelled
is.labelled <- function(x) inherits(x, "haven_labelled")

#' @export
vec_ptype2.double.haven_labelled <- function(x, y, ...) {
  data_type <- vec_ptype2(x, vec_data(y), ...)
  new_labelled(data_type,
    labels = vec_cast_named(attr(y, "labels"), data_type),
    label = attr(y, "label", exact = TRUE)
  )
}
#' @export
vec_ptype2.integer.haven_labelled <- vec_ptype2.double.haven_labelled
#' @export
vec_ptype2.character.haven_labelled <- vec_ptype2.double.haven_labelled
#' @export
vec_ptype2.haven_labelled.double <- function(x, y, ...) vec_ptype2(y, x, ...)
#' @export
vec_ptype2.haven_labelled.integer <- vec_ptype2.haven_labelled.double
#' @export
vec_ptype2.haven_labelled.character <- vec_ptype2.haven_labelled.double

#' @export
vec_ptype2.haven_labelled.haven_labelled <- function(x, y, ..., x_arg = "", y_arg = "") {
  data_type <- vec_ptype2(vec_data(x), vec_data(y), ..., x_arg = x_arg, y_arg = y_arg)

  x_labels <- vec_cast_named(attr(x, "labels"), data_type, x_arg = x_arg)
  y_labels <- vec_cast_named(attr(y, "labels"), data_type, x_arg = y_arg)
  if (!identical(x_labels, y_labels)) {
    # strip labels if not compatible
    return(vec_data(x))
  }

  # Take label from LHS
  label <- attr(x, "label", exact = TRUE) %||% attr(y, "label", exact = TRUE)

  new_labelled(data_type, labels = x_labels, label = label)
}


#' @export
vec_cast.double.haven_labelled <- function(x, to, ...) vec_cast(vec_data(x), to)
#' @export
vec_cast.integer.haven_labelled <- function(x, to, ...) vec_cast(vec_data(x), to)
#' @export
vec_cast.character.haven_labelled <- function(x, to, ...) {
  if (is.character(x)) {
    vec_cast(vec_data(x), to, ...)
  } else {
    stop_incompatible_cast(x, to, ...)
  }
}

#' @export
vec_cast.haven_labelled.haven_labelled <- function(x, to, ..., x_arg = "", to_arg = "") {
  out_data <- vec_cast(vec_data(x), vec_data(to), ..., x_arg = x_arg, to_arg = to_arg)

  x_labels <- attr(x, "labels")
  to_labels <- attr(to, "labels")
  out_labels <- to_labels %||% x_labels

  out <- labelled(out_data,
    labels = out_labels,
    label = attr(x, "label", exact = TRUE)
  )

  # do we lose tagged na values?
  if (is.double(x) && !is.double(out)) {
    lossy <- is_tagged_na(x)
    maybe_lossy_cast(out, x, to, lossy,
      x_arg = x_arg,
      to_arg = to_arg,
      details = "Only doubles can hold tagged na values."
    )
  }

  # do any values become unlabelled?
  if (!is.null(to_labels)) {
    lossy <- x %in% x_labels[!x_labels %in% out_labels]
    maybe_lossy_cast(out, x, to, lossy,
      x_arg = x_arg,
      to_arg = to_arg,
      details = paste0("Values are labelled in `", x_arg, "` but not in `", to_arg, "`.")
    )
  }

  out
}

#' @export
vec_cast.haven_labelled.double <- function(x, to, ...) {
  vec_cast.haven_labelled.haven_labelled(x, to, ...)
}
#' @export
vec_cast.haven_labelled.integer <- function(x, to, ...) {
  vec_cast.haven_labelled.haven_labelled(x, to, ...)
}
#' @export
vec_cast.haven_labelled.character <- function(x, to, ...) {
  vec_cast.haven_labelled.haven_labelled(x, to, ...)
}

# Arithmetic --------------------------------------------------------------

#' Internal vctrs methods
#'
#' @keywords internal
#' @export vec_arith.haven_labelled
#' @method vec_arith haven_labelled
#' @export
vec_arith.haven_labelled <- function(op, x, y, ...) {
  UseMethod("vec_arith.haven_labelled", y)
}
#' @export
#' @method vec_arith.haven_labelled default
vec_arith.haven_labelled.default <- function(op, x, y, ...) {
  stop_incompatible_op(op, x, y)
}
#' @export
#' @method vec_arith.haven_labelled haven_labelled
vec_arith.haven_labelled.haven_labelled <- function(op, x, y, ...) {
  vec_arith_base(op, x, y)
}
#' @export
#' @method vec_arith.haven_labelled numeric
vec_arith.haven_labelled.numeric <- function(op, x, y, ...) {
  vec_arith_base(op, x, y)
}
#' @export
#' @method vec_arith.numeric haven_labelled
vec_arith.numeric.haven_labelled <- function(op, x, y, ...) {
  vec_arith_base(op, x, y)
}

#' @export
vec_math.haven_labelled <- function(.fn, .x, ...) {
  vec_math_base(.fn, .x, ...)
}


