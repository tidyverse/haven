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
  if (!is.null(na_values) && !vec_is(x, na_values)) {
    abort("`na_values` must be same type as `x`.")
  }
  if (!is.null(na_range)) {
    if (!is.numeric(x)) {
      abort("`na_range` is only applicable for labelled numeric vectors.")
    }
    if (!is.numeric(na_range) || length(na_range) != 2) {
      abort("`na_range` must be a numeric vector of length two.")
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
