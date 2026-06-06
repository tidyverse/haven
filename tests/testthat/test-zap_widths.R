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

test_that("zap_widths removes display_width from labelled vector", {
  x <- labelled(1:5, c(good = 1, bad = 5))
  attr(x, "display_width") <- 10
  out <- zap_widths(x)

  expect_null(attr(out, "display_width"))
  expect_s3_class(out, "haven_labelled")
  expect_equal(attr(out, "labels"), c(good = 1, bad = 5))
})

test_that("zap_widths removes display_width from labelled_spss vector", {
  x <- labelled_spss(1:5, c(good = 1), na_values = 5)
  attr(x, "display_width") <- 10
  out <- zap_widths(x)

  expect_null(attr(out, "display_width"))
  expect_s3_class(out, "haven_labelled_spss")
  expect_equal(attr(out, "na_values"), 5)
})

test_that("zap_widths preserves other attributes", {
  x <- structure(1:5, display_width = 10, format.spss = "F8.2", label = "myvar")
  out <- zap_widths(x)

  expect_null(attr(out, "display_width"))
  expect_equal(attr(out, "format.spss"), "F8.2")
  expect_equal(attr(out, "label"), "myvar")
})

test_that("zap_widths works with data frame containing multiple columns", {
  df <- tibble::tibble(
    a = structure(1:3, display_width = 10),
    b = structure(4:6, display_width = 20, label = "var b"),
    c = 7:9
  )
  out <- zap_widths(df)

  expect_null(attr(out$a, "display_width"))
  expect_null(attr(out$b, "display_width"))
  expect_equal(attr(out$b, "label"), "var b")
  expect_equal(out$c, 7:9)
})

test_that("zap_widths preserves existing NAs", {
  x <- structure(c(1, NA, 3), display_width = 10)
  out <- zap_widths(x)

  expect_null(attr(out, "display_width"))
  expect_equal(out, c(1, NA, 3))
})
