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

test_that("missing values encoded as NA", {
  num <- read_sav("labelled-num-na.sav")[[1]]

  expect_equal(num[[2]], NA_real_)
})

test_that("non-ASCII labels converted to utf-8", {
  x <- read_sav("umlauts.sav")[[1]]

  expect_equal(attr(x, "label"), "This is an \u00e4-umlaut")
  expect_equal(names(attr(x, "labels"))[1], "the \u00e4 umlaut")
})

