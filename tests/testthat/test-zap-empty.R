test_that("empty strings replaced with missing", {
  x <- c("", "a", NA)
  expect_equal(zap_empty(x), c(NA, "a", NA))
})

test_that("errors on non-character input", {
  expect_error(zap_empty(1:5))
  expect_error(zap_empty(factor("a")))
  expect_error(zap_empty(TRUE))
})

test_that("all empty strings become NA", {
  x <- c("", "", "")
  result <- zap_empty(x)
  expect_equal(length(result), 3)
  expect_true(all(is.na(result)))
  expect_type(result, "character")
})

test_that("no empty strings returns unchanged", {
  x <- c("a", "b", "c")
  expect_equal(zap_empty(x), x)
})

test_that("preserves existing NAs", {
  x <- c("", NA, "a", "", NA)
  expect_equal(zap_empty(x), c(NA, NA, "a", NA, NA))
})

test_that("works with empty input", {
  x <- character(0)
  expect_equal(zap_empty(x), x)
})
