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
  df <- tibble::tibble(x = 1:10, y = labelled(10:1, c("good" = 1)))
  expect_equal(zap_labels(df)$y, 10:1)
})

test_that("replaces user-defined missings for spss", {
  x <- labelled_spss(1:5, c(a = 1), na_values = c(2, 4))
  expect_equal(zap_labels(x), c(1, NA, 3, NA, 5))
})

test_that("keeps user-defined missings for spss if user_na = TRUE", {
  x <- labelled_spss(1:5, c(a = 1), na_values = c(2, 4))
  expect_equal(zap_labels(x, user_na = TRUE), 1:5)

  df <- tibble::tibble(x = 1:10, y = labelled_spss(10:1, c("good" = 1), na_values = 10))
  expect_equal(zap_labels(df, user_na = TRUE)$y, 10:1)
})

test_that("replaces na_range missings for spss", {
  x <- labelled_spss(1:10, c(low = 1), na_range = c(8, 10))
  out <- zap_labels(x)
  expect_equal(out[1:7], 1:7)
  expect_true(all(is.na(out[8:10])))
})

test_that("replaces na_range and na_values missings for spss", {
  x <- labelled_spss(1:10, c(low = 1), na_values = 5, na_range = c(8, 10))
  out <- zap_labels(x)
  expect_true(is.na(out[5]))
  expect_true(all(is.na(out[8:10])))
  expect_equal(out[c(1:4, 6:7)], c(1:4, 6:7))
})

test_that("keeps na_range missings for spss if user_na = TRUE", {
  x <- labelled_spss(1:10, c(low = 1), na_range = c(8, 10))
  out <- zap_labels(x, user_na = TRUE)
  expect_equal(out, 1:10)
})
