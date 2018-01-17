context("test-zap_widths.R")

test_that("can zap width attribute from vector", {
  x <- structure(1:5, display_width = 10)
  y <- zap_widths(x)

  expect_null(attributes(y))
})

test_that("can zap width attribute from vector in data frame", {
  x <- structure(1:5, display_width = 10)
  df <- data.frame(x = x)
  out <- zap_widths(df)

  expect_null(attributes(out$x))
})

