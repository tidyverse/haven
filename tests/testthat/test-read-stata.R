context("read_stata")

test_that("stata data types read into expected types (#45)", {
  df <- read_stata("types.dta")
  types <- vapply(df, typeof, character(1))

  expect_equal(types, c(
    vfloat = "double",
    vdouble = "double",
    vlong = "double",
    vint = "double",
    vbyte = "double",
    vstr = "character",
    vdate = "double",
    vdatetime = "double"
  ))
})

test_that("Stata %td (date) and %tc (datetime) read into expected classes", {
  df <- read_stata("types.dta")

  expect_is(df$vdate, "Date")
  expect_is(df$vdatetime, "POSIXct")
})


test_that("Old %d format read into Date class", {
  df <- zap_formats(read_stata(test_path("datetime-d.dta")))
  expect_equal(df$date, as.Date("2015-11-02"))
})


test_that("tagged double missings are read correctly", {
  x <- read_dta(test_path("tagged-na-double.dta"))$x
  expect_equal(na_tag(x), c(rep(NA, 5), "a", "h", "z"))

  labels <- attr(x, "labels")
  expect_equal(na_tag(labels), c("a", "z"))
})

test_that("tagged integer missings are read correctly", {
  x <- read_dta(test_path("tagged-na-int.dta"))$x
  expect_equal(na_tag(x), c(rep(NA, 5), "a", "h", "z"))

  labels <- attr(x, "labels")
  expect_equal(na_tag(labels), c("a", "z"))
})

test_that("file label and notes stored as attributes", {
  df <- read_dta(test_path("notes.dta"))

  expect_equal(attr(df, "label"), "This is a test dataset.")
  expect_length(attr(df, "notes"), 2)
})

test_that("only selected columns are read", {
  out <- read_dta(test_path("notes.dta"), col_select = "id")
  expect_named(out, "id")
})

# Row skipping ------------------------------------------------------------

test_that("using skip returns correct number of rows", {
  rows_after_skipping <- function(n) {
    nrow(read_dta(test_path("notes.dta"), skip = n))
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
    nrow(read_dta(test_path("notes.dta"), n_max = n))
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
