context("as_factor")

test_that("leaves factors unchanged", {
  f <- factor(letters, ordered = TRUE)
  expect_equal(as_factor(f), f)
})

test_that("converts characters to factors", {
  expect_equal(as_factor(letters), factor(letters))
})
