context("read_sav")

test_that("variable label stored as attributes", {
  df <- read_sav("variable-label.sav")
  expect_equal(attr(df$sex, "label"), "Gender")
})

