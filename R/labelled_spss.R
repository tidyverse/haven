#' Labelled vectors for SPSS
#'
#' This class is only used when `user_na = TRUE` in
#' [read_sav()]. It is similar to the [labelled()] class
#' but it also models SPSS's user-defined missings, which can be up to
#' three distinct values, or for numeric vectors a range.
#'
#' @param na_values A vector of values that should also be considered as missing.
#' @param na_range A numeric vector of length two giving the (inclusive) extents
#'   of the range. Use `-Inf` and `Inf` if you want the range to be
#'   open ended.
#' @inheritParams labelled
#' @export
#' @examples
#' x1 <- labelled_spss(1:10, c(Good = 1, Bad = 8), na_values = c(9, 10))
#' is.na(x1)
#'
#' x2 <- labelled_spss(1:10, c(Good = 1, Bad = 8), na_range = c(9, Inf),
#'                     label = "Quality rating")
#' is.na(x2)
#'
#' # Print data and metadata
#' x2
labelled_spss <- function(x = double(), labels = NULL, na_values = NULL,
                          na_range = NULL, label = NULL) {

  x <- vec_data(x)
  na_values <- vec_cast_named(na_values, x, x_arg = "na_values", to_arg = "x")
  labelled <- labelled(x, labels = labels, label = label)

  new_labelled_spss(
    vec_data(labelled),
    labels = attr(labelled, "labels"),
    na_values = na_values,
    na_range = na_range,
    label = attr(labelled, "label", exact = TRUE)
  )
}

new_labelled_spss <- function(x, labels, na_values, na_range, label) {
  if (any(is.na(na_values))) {
    abort("`na_values` can not contain missing values.")
  }

  if (!is.null(na_range)) {
    type_ok <- (is.character(x) && is.character(na_range)) ||
      (is.numeric(x) && is.numeric(na_range))
    if (!type_ok || length(na_range) != 2) {
      abort("`na_range` must be a vector of length two the same type as `x`.")
    }
    if (any(is.na(na_range))) {
      abort("`na_range` can not contain missing values.")
    }
    if (na_range[1] >= na_range[2]) {
      abort("`na_range` must be in ascending order.")
    }
  }

  new_labelled(x,
    labels = labels,
    label = label,
    na_values = na_values,
    na_range = na_range,
    class = "haven_labelled_spss"
  )
}

#' @export
vec_ptype_full.haven_labelled_spss <- function(x, ...) {
  paste0("labelled_spss<", vec_ptype_full(vec_data(x)), ">")
}

#' @export
obj_print_footer.haven_labelled_spss <- function(x, ...) {
  na_values <- attr(x, "na_values")
  if (!is.null(na_values)) {
    cat_line("Missing values: ", paste(na_values, collapse = ", "))
  }

  na_range <- attr(x, "na_range")
  if (!is.null(na_range)) {
    cat_line("Missing range:  [", paste(na_range, collapse = ", "), "]")
  }

  NextMethod()
}


#' @export
is.na.haven_labelled_spss <- function(x) {
  miss <- NextMethod()
  val <- vec_data(x)

  na_values <- attr(x, "na_values")
  if (!is.null(na_values)) {
    miss <- miss | val %in% na_values
  }

  na_range <- attr(x, "na_range")
  if (!is.null(na_range)) {
    miss <- miss | (val >= na_range[1] & val <= na_range[2])
  }

  miss
}

# Type system -------------------------------------------------------------

# # Import to avoid R CMD check NOTE
# #' @importFrom methods setOldClass
# setOldClass(c("haven_labelled_spss", "haven_labelled", "vctrs_vctr"))

#' @export
vec_ptype2.double.haven_labelled_spss <- function(x, y, ...) {
  data_type <- vec_ptype2(x, vec_data(y), ...)
  new_labelled_spss(
    data_type,
    labels = vec_cast_named(attr(y, "labels"), data_type),
    na_values = vec_cast(attr(y, "na_values"), data_type),
    na_range = attr(y, "na_range"),
    label = attr(y, "label", exact = TRUE)
  )
}
#' @export
vec_ptype2.integer.haven_labelled_spss <- vec_ptype2.double.haven_labelled_spss
#' @export
vec_ptype2.character.haven_labelled_spss <- vec_ptype2.double.haven_labelled_spss
#' @export
vec_ptype2.haven_labelled_spss.double <- function(x, y, ...) vec_ptype2(y, x, ...)
#' @export
vec_ptype2.haven_labelled_spss.integer <- vec_ptype2.haven_labelled_spss.double
#' @export
vec_ptype2.haven_labelled_spss.character <- vec_ptype2.haven_labelled_spss.double

#' @export
vec_ptype2.haven_labelled_spss.haven_labelled_spss <- function(x, y, ..., x_arg = "", y_arg = "") {
  data_type <- vec_ptype2(vec_data(x), vec_data(y), ..., x_arg = x_arg, y_arg = y_arg)

  x_labels <- vec_cast_named(attr(x, "labels"), data_type, x_arg = x_arg)
  y_labels <- vec_cast_named(attr(y, "labels"), data_type, x_arg = y_arg)
  if (!identical(x_labels, y_labels)) {
    # strip labels if not compatible
    return(vec_data(x))
  }

  # Take label from LHS
  label <- attr(x, "label", exact = TRUE) %||% attr(y, "label", exact = TRUE)

  x_na_values <- vec_cast(attr(x, "na_values"), data_type, x_arg = x_arg)
  y_na_values <- vec_cast(attr(y, "na_values"), data_type, x_arg = y_arg)

  # Ignore user defined missings and return a standard haven_labelled if
  # there are mismatches between the missing attributes
  if (!identical(x_na_values, y_na_values) ||
      !identical(attr(x, "na_range"), attr(y, "na_range"))) {
    new_labelled(data_type, labels = x_labels, label = label)
  } else {
    new_labelled_spss(
      data_type,
      labels = x_labels,
      na_values = x_na_values,
      na_range = attr(x, "na_range"),
      label = label)
  }
}

#' @export
vec_ptype2.haven_labelled_spss.haven_labelled <- vec_ptype2.haven_labelled_spss.haven_labelled_spss
#' @export
vec_ptype2.haven_labelled.haven_labelled_spss <- vec_ptype2.haven_labelled_spss.haven_labelled_spss


#' @export
vec_cast.double.haven_labelled_spss <- function(x, to, ...) vec_cast(vec_data(x), to)
#' @export
vec_cast.integer.haven_labelled_spss <- function(x, to, ...) vec_cast(vec_data(x), to)
#' @export
vec_cast.character.haven_labelled_spss <- function(x, to, ...) {
  if (is.character(x)) {
    vec_cast(vec_data(x), to, ...)
  } else {
    stop_incompatible_cast(x, to, ...)
  }
}

#' @export
vec_cast.haven_labelled_spss.haven_labelled_spss <- function(x, to, ..., x_arg = "", to_arg = "") {
  out_data <- vec_cast(vec_data(x), vec_data(to), ..., x_arg = x_arg, to_arg = to_arg)

  x_labels <- attr(x, "labels")
  to_labels <- attr(to, "labels")
  out_labels <- to_labels %||% x_labels

  x_na_values <- attr(x, "na_values")
  to_na_values <- attr(to, "na_values")

  x_na_range <- attr(x, "na_range")
  to_na_range <- attr(to, "na_range")

  out <- labelled_spss(out_data,
    labels = out_labels,
    na_values = to_na_values,
    na_range = to_na_range,
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

  # do any values switch from missing to non-missing?
  if (!is.null(to_na_range) | !is.null(to_na_values)) {
    lossy <- x %in% x_na_values

    if (!is.null(x_na_range))
      lossy <- lossy | (vec_data(x) >= x_na_range[1] & vec_data(x) <= x_na_range[2])

    if (!is.null(to_na_range))
      lossy <- lossy & !(vec_data(x) >= to_na_range[1] & vec_data(x) <= to_na_range[2])
    else if (!is.null(to_na_values))
      lossy <- lossy & !x %in% to_na_values

    maybe_lossy_cast(out, x, to, lossy,
      x_arg = x_arg,
      to_arg = to_arg,
      details = paste0("Values are missing in `", x_arg, "` but not in `", to_arg, "`.")
    )
  }

  out
}

#' @export
vec_cast.haven_labelled_spss.double <- function(x, to, ...) {
  vec_cast.haven_labelled_spss.haven_labelled_spss(x, to, ...)
}
#' @export
vec_cast.haven_labelled_spss.integer <- function(x, to, ...) {
  vec_cast.haven_labelled_spss.haven_labelled_spss(x, to, ...)
}
#' @export
vec_cast.haven_labelled_spss.character <- function(x, to, ...) {
  vec_cast.haven_labelled_spss.haven_labelled_spss(x, to, ...)
}

#' @export
vec_cast.haven_labelled.haven_labelled_spss <- function(x, to, ...) {
  vec_cast.haven_labelled.haven_labelled(x, to, ...)
}

#' @export
vec_cast.haven_labelled_spss.haven_labelled <- function(x, to, ...) {
  vec_cast.haven_labelled_spss.haven_labelled_spss(x, to, ...)
}
