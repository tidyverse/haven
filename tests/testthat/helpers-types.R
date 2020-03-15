expect_lossy_cast <- function(x, to, ...) {
  expect_error(class = "vctrs_error_cast_lossy", vec_cast(x, to), ...)
}
