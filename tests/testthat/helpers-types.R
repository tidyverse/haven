expect_lossy_cast <- function(...) {
  expect_error(class = "vctrs_error_cast_lossy", ...)
}

expect_incompatible_type <- function(...) {
  expect_error(class = "vctrs_error_incompatible_type", ...)
}
