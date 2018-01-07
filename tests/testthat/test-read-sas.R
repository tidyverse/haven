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

test_that("value labels read in as same type as vector", {
  df <- read_sas("hadley.sas7bdat", "formats.sas7bcat")

  expect_equal(typeof(df$gender), typeof(attr(df$gender, "labels")))
  expect_equal(typeof(df$workshop), typeof(attr(df$workshop, "labels")))
})

test_that("date times are converted into corresponding R types", {
  df <- read_sas(test_path("datetime.sas7bdat"))
  expect_equal(df$VAR1[1], ISOdatetime(2015, 02, 02, 14, 42, 12, "UTC"))
  expect_equal(df$VAR2[1], as.Date("2015-02-02"))
  expect_equal(df$VAR3[1], as.Date("2015-02-02"))
  expect_equal(df$VAR4[1], as.Date("2015-02-02"))
  expect_equal(df$VAR5[1], hms::hms(52932))
})

test_that("tagged missings are read correctly", {
  x <- read_sas(test_path("tagged-na.sas7bdat"), test_path("tagged-na.sas7bcat"))$x
  expect_equal(na_tag(x), c(rep(NA, 5), "a", "h", "z"))

  labels <- attr(x, "labels")
  expect_equal(na_tag(labels), c("a", "z"))
})
