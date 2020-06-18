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

test_that("default name repair can be overridden", {
  df <- data.frame(1:3, 1:3)
  colnames(df) <- c("id", "id")

  path <- tempfile()
  write_sas(df, path)

  expect_message(read_sas(path), "id...1")
  expect_message(read_sas(path, .name_repair = "minimal"), NA)
})

# Row skipping ------------------------------------------------------------

test_that("using skip returns correct number of rows", {
  rows_after_skipping <- function(n) {
    nrow(read_sas(test_path("hadley.sas7bdat"), skip = n))
  }

  n <- rows_after_skipping(0)

  expect_equal(rows_after_skipping(1), n - 1)
  expect_equal(rows_after_skipping(n - 1), 1)
  expect_equal(rows_after_skipping(n + 0), 0)
  expect_equal(rows_after_skipping(n + 1), 0)
})


# Row limiting ------------------------------------------------------------

test_that("can limit the number of rows to read", {
  rows_with_limit <- function(n) {
    nrow(read_sas(test_path("hadley.sas7bdat"), n_max = n))
  }

  n <- rows_with_limit(Inf)
  expect_equal(rows_with_limit(0), 0)
  expect_equal(rows_with_limit(1), 1)
  expect_equal(rows_with_limit(n), n)
  expect_equal(rows_with_limit(n + 1), n)

  # alternatives for unlimited rows
  expect_equal(rows_with_limit(NA), n)
  expect_equal(rows_with_limit(-1), n)
})

test_that("throws informative error on bad row limit", {
  rows_with_limit <- function(n) {
    nrow(read_sas(test_path("hadley.sas7bdat"), n_max = n))
  }

  expect_error(rows_with_limit(1:5), "must have length 1")
  expect_error(rows_with_limit("foo"), "must be numeric")
})


# Column selection --------------------------------------------------------

test_that("can select columns to read, with tidyselect semantics", {
  with_col_select <- function(x) {
    read_sas(test_path("hadley.sas7bdat"), col_select = {{ x }})
  }

  full_data <- with_col_select(NULL)
  n_col <- ncol(full_data)

  expect_equivalent(with_col_select("id"), full_data[, "id"])
  expect_equivalent(with_col_select(id), full_data[, "id"])
  expect_equivalent(with_col_select(2:3), full_data[, 2:3])
  expect_equivalent(with_col_select(tidyselect::last_col()), full_data[, n_col])
})

test_that("throws error on empty column selection", {
  with_col_select <- function(x) {
    read_sas(test_path("hadley.sas7bdat"), col_select = {{ x }})
  }

  expect_error(with_col_select(character()), "Can't find")
  expect_error(with_col_select(tidyselect::starts_with("x")), "Can't find")
})


# Column selection (SAS specific) -----------------------------------------

test_that("can select columns when a catalog file is present", {
  expect_named(
    read_sas(
      test_path("hadley.sas7bdat"),
      test_path("formats.sas7bcat"),
      col_select = "workshop"
    ),
    "workshop"
  )
})

test_that("using cols_only warns about deprecation, but works", {
  out <- expect_warning(
    read_sas(test_path("hadley.sas7bdat"), cols_only = "id"),
    "is deprecated"
  )
  expect_named(out, "id")
})
