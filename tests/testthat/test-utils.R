context("test-utils.R")


# max_level_lengths -------------------------------------------------------

test_that("works with NA levels", {
  x <- factor(c("a", "abc", NA), exclude = NULL)
  expect_equal(max_level_length(x), 3)
})

test_that("works with empty factors", {
  x <- factor(character(), levels = character())
  expect_equal(max_level_length(x), 0)

  x <- factor(character(), levels = c(NA_character_))
  expect_equal(max_level_length(x), 0)
})
