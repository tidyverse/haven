context("read_sas")

test_that("variable label stored as attributes", {
  df <- read_sas("hadley.sas7bdat")
  expect_equal(attr(df$gender, "label"), NULL)
  expect_equal(attr(df$q1, "label"), "The instructor was well prepared")
})

test_that("value labels parsed from bcat file", {
  df <- read_sas("hadley.sas7bdat", "formats.sas7bcat")

  expect_is(df$gender, "labelled")
  expect_equal(attr(df$gender, "labels"), c(Female = "f", Male = "m"))
  expect_equal(attr(df$workshop, "labels"), c(R = 1, SAS = 2))
})
