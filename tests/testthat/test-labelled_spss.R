context("labelled_spss")

test_that("constructor checks na_value", {
  expect_error(labelled_spss(1:10, na_values = "a"), "must be same type")
})

test_that("constructor checks na_range", {
  expect_error(labelled_spss(1:10, na_range = "a"), "must be a numeric vector")
  expect_error(labelled_spss(1:10, na_range = 1:3), "of length two")
})
