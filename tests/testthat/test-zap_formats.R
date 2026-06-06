test_that("zap_formats strips format attributes from vector", {
  x <- structure(1:5, format.spss = "F8.2", format.sas = "BEST12", format.stata = "%9.0g")
  y <- zap_formats(x)

  expect_null(attr(y, "format.spss"))
  expect_null(attr(y, "format.sas"))
  expect_null(attr(y, "format.stata"))
  expect_equal(y, 1:5)
})

test_that("zap_formats returns vectors without format attributes unmodified", {
  x <- 1:5
  expect_equal(zap_formats(x), x)
})

test_that("zap_formats preserves other attributes", {
  x <- structure(1:5, format.spss = "F8.2", label = "test", display_width = 10)
  y <- zap_formats(x)

  expect_null(attr(y, "format.spss"))
  expect_equal(attr(y, "label"), "test")
  expect_equal(attr(y, "display_width"), 10)
})

test_that("zap_formats is applied to every column in data frame", {
  x <- structure(1:3, format.spss = "F8.2")
  y <- structure(c("a", "b", "c"), format.stata = "%9s")
  df <- data.frame(x = x, y = y, stringsAsFactors = FALSE)

  out <- zap_formats(df)

  expect_null(attr(out$x, "format.spss"))
  expect_null(attr(out$y, "format.stata"))
  expect_equal(out$x, 1:3)
  expect_equal(out$y, c("a", "b", "c"))
})

test_that("zap_formats works with labelled vectors", {
  x <- labelled_spss(1:5, c(good = 1, bad = 5), na_values = 9, label = "rating")
  attr(x, "format.spss") <- "F8.2"

  y <- zap_formats(x)

  expect_null(attr(y, "format.spss"))
  expect_s3_class(y, "haven_labelled_spss")
  expect_equal(attr(y, "labels"), c(good = 1, bad = 5))
  expect_equal(attr(y, "label", exact = TRUE), "rating")
  expect_equal(attr(y, "na_values"), 9)
})
