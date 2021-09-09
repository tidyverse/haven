cat_line <- function(...) {
  cat(paste0(..., "\n", collapse = ""))
}

# TODO: Remove once vec_cast() preserves names.
# https://github.com/r-lib/vctrs/issues/623
vec_cast_named <- function(x, to, ...) {
  stats::setNames(vec_cast(x, to, ...), names(x))
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

skip_cols <- function(reader, col_select = NULL, ...) {
  col_select <- enquo(col_select)
  if (quo_is_null(col_select)) {
    return(character())
  }

  cols <- names(reader(..., n_max = 0L))
  sels <- tidyselect::vars_select(cols, !!col_select)

  if (length(sels) == 0) {
    stop("Can't find any columns matching `col_select` in data.", call. = FALSE)
  }

  setdiff(cols, sels)
}

validate_n_max <- function(n) {
  if (!is.numeric(n) && !is.na(n)) {
    stop("`n_max` must be numeric, not ", class(n)[1], ".", call. = FALSE)
  }

  if (length(n) != 1) {
    stop("`n_max` must have length 1, not ", length(n), ".", call. = FALSE)
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
