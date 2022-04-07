# read_sas ----------------------------------------------------------------

test_that("variable label stored as attributes", {
  df <- read_sas(test_path("sas/hadley.sas7bdat"))
  expect_equal(attr(df$gender, "label"), NULL)
  expect_equal(attr(df$q1, "label"), "The instructor was well prepared")
})

test_that("value labels parsed from bcat file", {
  df <- read_sas(test_path("sas/hadley.sas7bdat"), test_path("sas/formats.sas7bcat"))

  expect_s3_class(df$gender, "haven_labelled")
  expect_equal(attr(df$gender, "labels"), c(Female = "f", Male = "m"))
  expect_equal(attr(df$workshop, "labels"), c(R = 1, SAS = 2))
})

test_that("value labels read in as same type as vector", {
  df <- read_sas(test_path("sas/hadley.sas7bdat"), test_path("sas/formats.sas7bcat"))

  expect_equal(typeof(df$gender), typeof(attr(df$gender, "labels")))
  expect_equal(typeof(df$workshop), typeof(attr(df$workshop, "labels")))
})

test_that("date times are converted into corresponding R types", {
  df <- read_sas(test_path("sas/datetime.sas7bdat"))
  expect_equal(df$VAR1[1], ISOdatetime(2015, 02, 02, 14, 42, 12, "UTC"))
  expect_equal(df$VAR2[1], as.Date("2015-02-02"))
  expect_equal(df$VAR3[1], as.Date("2015-02-02"))
  expect_equal(df$VAR4[1], as.Date("2015-02-02"))
  expect_equal(df$VAR5[1], hms::hms(52932))
})

test_that("tagged missings are read correctly", {
  x <- read_sas(test_path("sas/tagged-na.sas7bdat"), test_path("sas/tagged-na.sas7bcat"))$x
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

test_that("connections are read", {
  file_conn <- file(test_path("sas/hadley.sas7bdat"))
  expect_identical(read_sas(file_conn), read_sas("sas/hadley.sas7bdat"))
})

test_that("zip files are read", {
  expect_identical(
    read_sas(test_path("sas/hadley.zip")),
    read_sas(test_path("sas/hadley.sas7bdat"))
  )
})

# Row skipping ------------------------------------------------------------

test_that("using skip returns correct number of rows", {
  rows_after_skipping <- function(n) {
    nrow(read_sas(test_path("sas/hadley.sas7bdat"), skip = n))
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
    nrow(read_sas(test_path("sas/hadley.sas7bdat"), n_max = n))
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
    nrow(read_sas(test_path("sas/hadley.sas7bdat"), n_max = n))
  }

  expect_error(rows_with_limit(1:5), "must have length 1")
  expect_error(rows_with_limit("foo"), "must be <numeric>")
})

# Column selection --------------------------------------------------------

test_that("can select columns to read, with tidyselect semantics", {
  with_col_select <- function(x) {
    read_sas(test_path("sas/hadley.sas7bdat"), col_select = {{ x }})
  }

  full_data <- with_col_select(NULL)
  n_col <- ncol(full_data)

  expect_equal(with_col_select("id"), full_data[, "id"])
  expect_equal(with_col_select(id), full_data[, "id"])
  expect_equal(with_col_select(2:3), full_data[, 2:3])
  expect_equal(with_col_select(tidyselect::last_col()), full_data[, n_col])
})

test_that("throws error on empty column selection", {
  with_col_select <- function(x) {
    read_sas(test_path("sas/hadley.sas7bdat"), col_select = {{ x }})
  }

  expect_error(with_col_select(character()), "Can't find")
  expect_error(with_col_select(tidyselect::starts_with("x")), "Can't find")
})

test_that("can select columns when a catalog file is present", {
  expect_named(
    read_sas(
      test_path("sas/hadley.sas7bdat"),
      test_path("sas/formats.sas7bcat"),
      col_select = "workshop"
    ),
    "workshop"
  )
})

test_that("using cols_only warns about deprecation, but works", {
  expect_warning(
    out <- read_sas(test_path("sas/hadley.sas7bdat"), cols_only = "id"),
    "is deprecated"
  )
  expect_named(out, "id")
})

# write_sas ---------------------------------------------------------------

test_that("can roundtrip basic types", {
  x <- runif(10)
  expect_equal(roundtrip_var(x, "sas"), x)
  expect_equal(roundtrip_var(1:10, "sas"), 1:10)
  expect_equal(roundtrip_var(c(TRUE, FALSE), "sas"), c(1, 0))
  expect_equal(roundtrip_var(letters, "sas"), letters)
})

test_that("can roundtrip missing values (as much as possible)", {
  expect_equal(roundtrip_var(NA, "sas"), NA_integer_)
  expect_equal(roundtrip_var(NA_real_, "sas"), NA_real_)
  expect_equal(roundtrip_var(NA_integer_, "sas"), NA_integer_)
  expect_equal(roundtrip_var(NA_character_, "sas"), "")
})

test_that("can write labelled with NULL labels", {
  int <- labelled(c(1L, 2L), NULL)
  num <- labelled(c(1, 2), NULL)
  chr <- labelled(c("a", "b"), NULL)

  expect_equal(roundtrip_var(int, "sas"), c(1L, 2L))
  expect_equal(roundtrip_var(num, "sas"), c(1, 2))
  expect_equal(roundtrip_var(chr, "sas"), c("a", "b"))
})

test_that("can roundtrip date times", {
  x1 <- c(as.Date("2010-01-01"), NA)
  expect_equal(roundtrip_var(x1, "sas"), x1)

  # converted to same time in UTC
  x2 <- as.POSIXct("2010-01-01 09:00", tz = "Pacific/Auckland")
  expect_equal(
    roundtrip_var(x2, "sas"),
    as.POSIXct("2010-01-01 09:00", tz = "UTC")
  )

  attr(x2, "label") <- "abc"
  expect_equal(attr(roundtrip_var(x2, "sas"), "label"), "abc")
})

test_that("can roundtrip format attribute", {
  df <- data.frame(x = structure(1:5, format.sas = "xyz"))
  path <- tempfile()

  write_sas(df, path)
  out <- read_sas(path)

  expect_equal(df$x, out$x)
})

test_that("infinity gets converted to NA", {
  expect_equal(roundtrip_var(c(Inf, 0, -Inf), "sas"), c(NA, 0, NA))
})

# read_xpt ----------------------------------------------------------------

test_that("can read date/times", {
  x <- as.Date("2018-01-01")
  df <- data.frame(date = x, datetime = as.POSIXct(x))
  path <- tempfile()
  write_xpt(df, path)

  res <- read_xpt(path)
  expect_s3_class(res$date, "Date")
  expect_s3_class(res$datetime, "POSIXct")
})

# write_xpt ---------------------------------------------------------------

test_that("can roundtrip basic types", {
  x <- runif(10)
  expect_equal(roundtrip_var(x, "xpt"), x)
  expect_equal(roundtrip_var(1:10, "xpt"), 1:10)
  expect_equal(roundtrip_var(c(TRUE, FALSE), "xpt"), c(1, 0))
  expect_equal(roundtrip_var(letters, "xpt"), letters)
})

test_that("can roundtrip missing values (as much as possible)", {
  expect_equal(roundtrip_var(NA, "xpt"), NA_integer_)
  expect_equal(roundtrip_var(NA_real_, "xpt"), NA_real_)
  expect_equal(roundtrip_var(NA_integer_, "xpt"), NA_integer_)
  expect_equal(roundtrip_var(NA_character_, "xpt"), "")
})

test_that("invalid files generate informative errors", {
  expect_snapshot(error = TRUE, {
    write_xpt(mtcars, file.path(tempdir(), " temp.xpt"))
  })
})

test_that("can roundtrip file labels", {
  df <- tibble(x = 1)
  expect_null(attr(roundtrip_xpt(df), "label"))
  expect_equal(attr(roundtrip_xpt(df, label = "abcd"), "label"), "abcd")

  attr(df, "label") <- "abc"
  expect_equal(attr(roundtrip_xpt(df), "label"), "abc")
  expect_equal(attr(roundtrip_xpt(df, label = "abcd"), "label"), "abcd")
  expect_null(attr(roundtrip_xpt(df, label = NULL), "label"))
})

test_that("can roundtrip format attribute", {
  df <- tibble(
    char_var = structure("Hello!", format.sas = "$CHAR"),
    long_char = structure("111111111111111", format.sas = "$CHAR10"),
    date_var = structure(Sys.Date(), format.sas = "DATE9"),
    a = structure(100.12345, format.sas = "10.3"),
    b = structure(100.12345, format.sas = "10"),
    c = structure(100.12345, format.sas = "F10.3"),
    d = structure(100.12345, format.sas = "F10"),
    e = structure(100.12345, format.sas = "COMMA10.3")
  )

  path <- tempfile()

  write_xpt(df, path)
  out <- read_xpt(path)

  expect_identical(df, out)
})
