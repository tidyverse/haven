max_level_length <- function(x) {
  if (!is.factor(x))
    return(0L)

  max(0L, nchar(levels(x)), na.rm = TRUE)
}

# Adapted from rlang
is_integerish <- function(x) {
  if (!typeof(x) %in% c("double", "integer")) return(FALSE)

  missing_elts <- is.na(x)
  finite_elts <- is.finite(x) | missing_elts
  if (!all(finite_elts)) {
    return(FALSE)
  }

  x_finite <- x[finite_elts & !missing_elts]
  all(x_finite == as.integer(x_finite))
}

cat_line <- function(...) {
  cat(paste0(..., "\n", collapse = ""))
}
