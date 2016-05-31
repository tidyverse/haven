context("Labelled")

test_that("integer and double labels and values are compatible", {
  expect_error(labelled(1, c(female = 2L, male = 1L)), NA)
  expect_error(labelled(1L, c(female = 2, male = 1)), NA)
})

# methods -----------------------------------------------------------------

test_that("printed output is stable", {
  x <- labelled(1:5, c("Good" = 1, "Bad" = 5))
  expect_output_file(print(x), "labelled-output.txt")
})

# zap_labels --------------------------------------------------------------

test_that("zap_labels strips labelled attributes", {
  var <- labelled(c(1L, 98L, 99L),  c(not_answered = 98L, not_applicable = 99L))
  exp <- c(1L,98L,99L)
  expect_equal(zap_labels(var), exp)
})

test_that("zap_labels returns variables not of class('labelled') unmodified", {
  var <- c(1L, 98L, 99L)
  expect_equal(zap_labels(var), var)
})
