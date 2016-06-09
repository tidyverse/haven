context("zap_missing")

test_that("strips na tags", {
  x1 <- labelled(tagged_na("a", "b"), c(a = tagged_na("a"), b = 1))
  x2 <- zap_missing(x1)

  expect_equal(na_tag(x2), c(NA_character_, NA))
  expect_equal(attr(x2, "labels"), c(b = 1))
})

test_that("converts user-defined missings", {
  x1 <- labelled_spss(c(1, 2, 99), c(missing = 99), na_values = 99)
  x2 <- zap_missing(x1)

  expect_equal(x2[[3]], NA_real_)
  expect_s3_class(x2, "labelled")
})

test_that("converts data frame", {
  x1 <- labelled(tagged_na("a", "b"), c(a = tagged_na("a"), b = 1))

  df1 <- tibble::data_frame(x1 = 1, x2 = 2:1)
  df2 <- zap_missing(df1)

  expect_equal(na_tag(df1$x1), c(NA_character_, NA))
  expect_equal(df1$x2, df2$x2)
})
