context("labelled_spss")

test_that("constructor checks na_value", {
  expect_error(labelled_spss(1:10, na_values = "a"), "must be same type")
})

test_that("constructor checks na_range", {
  expect_error(labelled_spss(1:10, na_range = "a"), "must be a numeric vector")
  expect_error(labelled_spss(1:10, na_range = 1:3), "of length two")
})

# is.na -------------------------------------------------------------------

test_that("values in na_range flagged as missing", {
  x <- labelled_spss(1:5, c("a" = 1), na_range = c(1, 3))
  expect_equal(is.na(x), c(TRUE, TRUE, TRUE, FALSE, FALSE))
})

test_that("values in na_values flagged as missing", {
  x <- labelled_spss(1:5, c("a" = 1), na_values = c(1, 3, 5))
  expect_equal(is.na(x), c(TRUE, FALSE, TRUE, FALSE, TRUE))
})
