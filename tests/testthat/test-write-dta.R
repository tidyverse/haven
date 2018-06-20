context("write_dta")

test_that("can roundtrip basic types", {
  x <- runif(10)
  expect_equal(roundtrip_var(x, "dta"), x)
  expect_equal(roundtrip_var(1:10, "dta"), 1:10)
  expect_equal(roundtrip_var(c(TRUE, FALSE), "dta"), c(1, 0))
  expect_equal(roundtrip_var(letters, "dta"), letters)
})

test_that("can roundtrip missing values (as much as possible)", {
  expect_equal(roundtrip_var(NA, "dta"), NA_integer_)
  expect_equal(roundtrip_var(NA_real_, "dta"), NA_real_)
  expect_equal(roundtrip_var(NA_integer_, "dta"), NA_integer_)
  expect_equal(roundtrip_var(NA_character_, "dta"), "")
})

test_that("can roundtrip date times", {
  x1 <- c(as.Date("2010-01-01"), NA)
  x2 <- as.POSIXct(x1)
  attr(x2, "tzone") <- "UTC"

  expect_equal(roundtrip_var(x1, "dta"), x1)
  expect_equal(roundtrip_var(x2, "dta"), x2)
})

test_that("infinity gets converted to NA", {
  expect_equal(roundtrip_var(c(Inf, 0, -Inf), "dta"), c(NA, 0, NA))
})

test_that("factors become labelleds", {
  f <- factor(c("a", "b"), levels = letters[1:3])
  rt <- roundtrip_var(f, "dta")

  expect_is(rt, "labelled")
  expect_equal(as.vector(rt), 1:2)
  expect_equal(attr(rt, "labels"), c(a = 1, b = 2, c = 3))
})

test_that("labels are preserved", {
  x <- 1:10
  attr(x, "label") <- "abc"

  expect_equal(attr(roundtrip_var(x, "dta"), "label"), "abc")
})

test_that("labelleds are round tripped", {
  int <- labelled(c(1L, 2L), c(a = 1L, b = 3L))
  num <- labelled(c(1, 2), c(a = 1, b = 3))
  chr <- labelled(c("a", "b"), c(a = "b", b = "a"))

  expect_equal(roundtrip_var(int, "dta"), int)
  # FIXME!
  # expect_equal(roundtrip_var(chr, "dta"), chr)
})

test_that("factors become labelleds", {
  f <- factor(c("a", "b"), levels = letters[1:3])
  rt <- roundtrip_var(f, "dta")

  expect_is(rt, "labelled")
  expect_equal(as.vector(rt), 1:2)
  expect_equal(attr(rt, "labels"), c(a = 1, b = 2, c = 3))
})

test_that("labels are converted to utf-8", {
  labels_utf8 <- c("\u00e9\u00e8", "\u00e0", "\u00ef")
  labels_latin1 <- iconv(labels_utf8, "utf-8", "latin1")

  v_utf8 <- labelled(3:1, setNames(1:3, labels_utf8))
  v_latin1 <- labelled(3:1, setNames(1:3, labels_latin1))

  expect_equal(names(attr(roundtrip_var(v_utf8, "dta"), "labels")), labels_utf8)
  expect_equal(names(attr(roundtrip_var(v_latin1, "dta"), "labels")), labels_utf8)
})

test_that("throws error on invalid variable names", {
  df <- data.frame(1)
  names(df) <- "x y"
  expect_error(write_dta(df, tempfile(), version = 13), "not valid Stata variables: `x y`")
})

test_that("can not write labelled non-integers (#343)", {
  df <- data.frame(x = labelled(c(1, 2, 3), c("a" = 1)))
  expect_error(write_dta(df, tempfile()), NA)

  df <- data.frame(x = labelled(c(1.5, 2, 3), c("a" = 1)))
  expect_error(write_dta(df, tempfile()), "supports labelled integers")
})

test_that("supports stata version 15", {
  df <- tibble(x = factor(letters), y = runif(26))

  path <- tempfile()
  write_dta(df, path, version = 15)
  df2 <- read_dta(path)

  df2$x <- as_factor(df2$x)
  df2$y <- zap_formats(df2$y)
  expect_equal(df2, df)
})
