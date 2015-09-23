context("read_sav")

test_that("variable label stored as attributes", {
  df <- read_sav("variable-label.sav")
  expect_equal(attr(df$sex, "label"), "Gender")
})

test_that("value labels stored as labelled class", {
  num <- read_sav("labelled-num.sav")
  str <- read_sav("labelled-str.sav")

  expect_equal(num[[1]], labelled(1, c("This is one" = 1)))
  expect_equal(str[[1]], labelled(c("M", "F"), c(Female = "F", Male = "M")))
})

test_that("value labels read in as same type as vector", {
  df <- read_sav("variable-label.sav")
  num <- read_sav("labelled-num.sav")
  str <- read_sav("labelled-str.sav")

  expect_equal(typeof(df$sex), typeof(attr(df$sex, "labels")))
  expect_equal(typeof(num[[1]]), typeof(attr(num[[1]], "labels")))
  expect_equal(typeof(str[[1]]), typeof(attr(str[[1]], "labels")))
})

test_that("labels as missing values preserved", {
  num <- read_sav("labelled-num-na.sav")[[1]]

  expect_equal(num[[2]], 9)
})

test_that("non-ASCII labels converted to utf-8", {
  x <- read_sav("umlauts.sav")[[1]]

  expect_equal(attr(x, "label"), "This is an \u00e4-umlaut")
  expect_equal(names(attr(x, "labels"))[1], "the \u00e4 umlaut")
})

test_that("datetime variables converted to the correct class", {
  df <- read_sav("datetime.sav")
  expect_true(inherits(df$date, "Date"))
  expect_true(inherits(df$date.posix, "POSIXct"))
  expect_true(inherits(df$time, "hms"))
})

test_that("datetime values correctly imported (offset)", {
  df <- read_sav("datetime.sav")
  expect_equal(df$date[1], as.Date("2014-09-22d"))
  expect_equal(df$date.posix[2], as.POSIXct("2014-09-23 15:59:20", tz="UTC"))
  expect_equal(as.integer(df$time[1]), 43870)
})
