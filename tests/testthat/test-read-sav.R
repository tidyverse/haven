context("read_sav")

test_that("variable label stored as attributes", {
  df <- read_spss("variable-label.sav")
  expect_equal(attr(df$sex, "label"), "Gender")
})

test_that("value labels stored as labelled class", {
  num <- zap_formats(read_spss(test_path("labelled-num.sav")))
  str <- zap_formats(read_spss(test_path("labelled-str.sav")))

  expect_equal(num[[1]], labelled(1, c("This is one" = 1)))
  expect_equal(str[[1]], labelled(c("M", "F"), c(Female = "F", Male = "M")))
})

test_that("value labels read in as same type as vector", {
  df <- read_spss("variable-label.sav")
  num <- read_spss("labelled-num.sav")
  str <- read_spss("labelled-str.sav")

  expect_equal(typeof(df$sex), typeof(attr(df$sex, "labels")))
  expect_equal(typeof(num[[1]]), typeof(attr(num[[1]], "labels")))
  expect_equal(typeof(str[[1]]), typeof(attr(str[[1]], "labels")))
})

test_that("non-ASCII labels converted to utf-8", {
  x <- read_spss("umlauts.sav")[[1]]

  expect_equal(attr(x, "label"), "This is an \u00e4-umlaut")
  expect_equal(names(attr(x, "labels"))[1], "the \u00e4 umlaut")
})

test_that("datetime variables converted to the correct class", {
  df <- read_spss("datetime.sav")
  expect_true(inherits(df$date, "Date"))
  expect_true(inherits(df$date.posix, "POSIXct"))
  expect_true(inherits(df$time, "hms"))
})

test_that("datetime values correctly imported (offset)", {
  df <- read_spss("datetime.sav")
  expect_equal(df$date[1], as.Date("2014-09-22d"))
  expect_equal(df$date.posix[2], as.POSIXct("2014-09-23 15:59:20", tz = "UTC"))
  expect_equal(as.integer(df$time[1]), 43870)
})

test_that("formats roundtrip", {
  df <- tibble::data_frame(
    a = structure(c(1, 1, 2), format.spss = "F1.0"),
    b = structure(4:6, format.spss = "F2.1"),
    c = structure(7:9, format.spss = "N2"),
    d = structure(c("Text", "Text", ""), format.spss = "A100")
  )

  tmp <- tempfile()
  on.exit(unlink(tmp))

  write_sav(df, tmp)
  df2 <- read_sav(tmp)

  expect_equal(df$a, df$a)
  expect_equal(df$b, df$b)
  expect_equal(df$c, df$c)
  expect_equal(df$d, df$d)
})

test_that("widths roundtrip", {
  df <- tibble::data_frame(
    a = structure(c(1, 1, 2), display_width = 10),
    b = structure(4:6, display_width = 11),
    c = structure(7:9, display_width = 12),
    d = structure(c("Text", "Text", ""), display_width = 10)
  )

  tmp <- tempfile()
  on.exit(unlink(tmp))

  write_sav(df, tmp)
  df2 <- read_sav(tmp)

  expect_equal(df$a, df$a)
  expect_equal(df$b, df$b)
  expect_equal(df$c, df$c)
  expect_equal(df$d, df$d)
})

# User-defined missings ---------------------------------------------------

test_that("user-defined missing values read as missing by default", {
  num <- read_spss(test_path("labelled-num-na.sav"))[[1]]
  expect_equal(num[[2]], NA_real_)
})

test_that("user-defined missing values can be preserved", {
  num <- read_spss(test_path("labelled-num-na.sav"), user_na = TRUE)[[1]]

  expect_s3_class(num, "labelled_spss")
  expect_equal(num[[2]], 9)

  expect_equal(attr(num, "na_values"), 9)
  expect_equal(attr(num, "na_range"), NULL)

  num
})

test_that("system missings read as NA", {
  df <- tibble::tibble(x = c(1, NA))
  out <- roundtrip_sav(df)

  expect_identical(df$x, c(1, NA))
})
