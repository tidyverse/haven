
context("Importing missing values")

test_that("Pseudo-missings are correctly imported", {
  df <- read_sav("missings.sav", FALSE)[-1]

  expect_equal(is_pseudo_na(df[[1]]), c(FALSE, FALSE, TRUE, TRUE, TRUE))
  expect_equal(is_pseudo_na(df[[2]]), c(TRUE, TRUE, TRUE, FALSE, FALSE))
  expect_equal(is_pseudo_na(df[[3]]), c(FALSE, TRUE, FALSE, FALSE, FALSE))
  expect_equal(is_pseudo_na(df[[4]]), c(FALSE, FALSE, TRUE, FALSE, FALSE))
  expect_equal(is_na(df[[4]]), c(FALSE, TRUE, TRUE, FALSE, TRUE))
  expect_equal(is_pseudo_na(df[[5]]), rep(FALSE, 5))
  expect_equal(is_pseudo_na(df[[6]]), c(FALSE, FALSE, TRUE, TRUE, TRUE))
})
