test_that("can zap format attributes from vector", {
  rlang::local_options(lifecycle_verbosity = "quiet")
  x <- structure(1:5, format.sas = "F8.", format.spss = "F8.0")
  expect_null(attributes(zap_formats(x)))
})

test_that("can zap format attributes from vector in data frame", {
  rlang::local_options(lifecycle_verbosity = "quiet")
  df <- data.frame(x = structure(1:5, format.stata = "%9.0g"))
  expect_null(attributes(zap_formats(df)$x))
})

test_that("zap_formats() is deprecated", {
  expect_snapshot(. <- zap_formats(1))
})
