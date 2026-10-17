test_that("can zap width attribute from vector", {
  rlang::local_options(lifecycle_verbosity = "quiet")
  x <- structure(1:5, display_width = 10)
  y <- zap_widths(x)

  expect_null(attributes(y))
})

test_that("can zap width attribute from vector in data frame", {
  rlang::local_options(lifecycle_verbosity = "quiet")
  x <- structure(1:5, display_width = 10)
  df <- data.frame(x = x)
  out <- zap_widths(df)

  expect_null(attributes(out$x))
})

test_that("zap_widths() is deprecated", {
  expect_snapshot(. <- zap_widths(1))
})
