context("zap_labels")

test_that("zap_labels strips labelled attributes", {
  var <- labelled(c(1L, 98L, 99L), c(not_answered = 98L, not_applicable = 99L))
  exp <- c(1L, 98L, 99L)
  expect_equal(zap_labels(var), exp)
})

test_that("zap_labels returns variables not of class('labelled') unmodified", {
  var <- c(1L, 98L, 99L)
  expect_equal(zap_labels(var), var)
})

test_that("zap_labels is applied to every column in data frame", {
  df <- tibble::data_frame(x = 1:10, y = labelled(10:1, c("good" = 1)))
  expect_equal(zap_labels(df)$y, 10:1)
})

test_that("replaces user-defined missings for spss", {
  x <- labelled_spss(1:5, c(a = 1), na_values = c(2, 4))
  expect_equal(zap_labels(x), c(1, NA, 3, NA, 5))
})
