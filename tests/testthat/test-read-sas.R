context("read_sas")

test_that("variable label stored as attributes", {
  df <- read_sas("hadley.sas7bdat")
  expect_equal(attr(df$gender, "label"), NULL)
  expect_equal(attr(df$q1, "label"), "The instructor was well prepared")
})

test_that("value labels parsed from bcat file", {
  df <- read_sas("hadley.sas7bdat", "formats.sas7bcat")

  expect_s3_class(df$gender, "haven_labelled")
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

test_that("can limit the number of rows read", {
  expect_equal(nrow(read_sas(test_path("hadley.sas7bdat"), n_max = 1)), 1L)
  expect_equal(nrow(read_sas(test_path("hadley.sas7bdat"), n_max = 0)), 0L)
})

test_that("only selected columns are read", {
  out <- read_sas(test_path("hadley.sas7bdat"), col_select = c("id", "workshop"))
  expect_equal(names(out), c("id", "workshop"))
})

test_that("can select columns by position", {
  out <- read_sas(test_path("hadley.sas7bdat"), col_select = 2:3)
  expect_equal(names(out), c("workshop", "gender"))
})

test_that("can select columns with select helpers", {
  out <- read_sas(test_path("hadley.sas7bdat"), col_select = c(id, workshop))
  expect_equal(names(out), c("id", "workshop"))

  out <- read_sas(
    test_path("hadley.sas7bdat"),
    col_select = tidyselect::starts_with("q")
  )
  expect_equal(names(out), paste0("q", 1:4))
})

test_that("selecting no columns works", {
  out <- read_sas(test_path("hadley.sas7bdat"), col_select = character())
  expect_equal(ncol(out), 0L)
  expect_equal(nrow(out), 8L)
})

test_that("can select columns with a catalog file", {
  out <- read_sas(
    test_path("hadley.sas7bdat"),
    test_path("formats.sas7bcat"),
    col_select = "workshop"
  )
  expect_equal(names(out), "workshop")
})
