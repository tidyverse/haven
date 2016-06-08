context("replace_with")

test_that("updates numeric values", {
  x <- 1:5

  expect_equal(replace_with(x, -1, 5), x)
  expect_equal(replace_with(x, 1,  5), c(5, 2:5))
  expect_equal(replace_with(x, 5,  1), c(1:4, 1))

  expect_equal(replace_with(x, 1:5, rep(1, 5)), rep(1, 5))
})

test_that("udpates tagged NAs", {
  x <- c(tagged_na("a"), 1:3)

  expect_equal(replace_with(x, tagged_na("a"), 0), 0:3)
})
