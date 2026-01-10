cat_line <- function(...) {
  cat(paste0(..., "\n", collapse = ""))
}

# TODO: Remove once vec_cast() preserves names.
# https://github.com/r-lib/vctrs/issues/623
vec_cast_named <- function(x, to, ...) {
  stats::setNames(vec_cast(x, to, ...), names(x))
}

combine_labels <- function(x_labels, y_labels, x_arg, y_arg) {
  x_common <- x_labels[x_labels %in% y_labels]
  y_common <- y_labels[y_labels %in% x_labels]

  if (length(x_common) > 0) {
    x_common <- x_common[order(x_common)]
    y_common <- y_common[order(y_common)]

    problems <- x_common[names(x_common) != names(y_common)]
    if (length(problems) > 0) {
      problems <- cli::cli_vec(problems, list(vec_trunc = 10))

      cli_warn(c(
        "{.var {x_arg}} and {.var {y_arg}} have conflicting value labels.",
        i = "Labels for these values will be taken from {.var {x_arg}}.",
        x = "Values: {.val {problems}}"
      ))
    }
  }

  c(x_labels, y_labels[!y_labels %in% x_labels])
}

force_utc <- function(x) {
  if (identical(attr(x, "tzone"), "UTC")) {
    x
  } else {
    x_attr <- attributes(x)
    x <- format(x, usetz = FALSE, format = "%Y-%m-%d %H:%M:%S")
    x <- as.POSIXct(x, tz = "UTC", format = "%Y-%m-%d %H:%M:%S")
    attr_miss <- setdiff(names(x_attr), c(names(attributes(x)), "names"))
    attributes(x)[attr_miss] <- x_attr[attr_miss]
    x
  }
}

select_cols <- function(reader, col_select = NULL, ..., call = caller_env()) {
  col_select <- enquo(col_select)
  if (quo_is_null(col_select)) {
    return(list(pos = NULL, skip = integer()))
  }

  cols <- names(reader(..., n_max = 0L))
  data <- as.list(setNames(seq_along(cols), cols))

  pos <- tidyselect::eval_select(
    col_select,
    data = data,
    allow_rename = TRUE,
    allow_empty = FALSE,
    allow_predicates = FALSE,
    error_call = call
  )

  list(
    select = pos,
    skip = as.integer(setdiff(seq_along(cols), pos) - 1L)
  )
}

output_cols <- function(data, cols, name_repair, call = caller_env()) {
  if (is.null(cols$select)) {
    set_names(data, vctrs::vec_as_names(names(data), repair = name_repair, call = call))
  } else {
    set_names(data[rank(cols$select)], names(cols$select))
  }
}

validate_n_max <- function(n, call = caller_env()) {
  if (!is.numeric(n) && !is.na(n)) {
    cli_abort("{.arg n_max} must be {.cls numeric}, not {.cls {class(n)[1]}}.", call = call)
  }

  if (length(n) != 1) {
    cli_abort("{.arg n_max} must have length 1, not {length(n)}.", call = call)
  }

  if (is.na(n) || is.infinite(n) || n < 0) {
    return(-1L)
  }

  as.integer(n)
}

adjust_tz <- function(df) {
  datetime <- vapply(df, inherits, "POSIXt", FUN.VALUE = logical(1))
  df[datetime] <- lapply(df[datetime], force_utc)
  df
}

var_names <- function(data, i) {
  names(data)[i]
}
