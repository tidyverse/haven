context("write_sas")

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

test_that("can roundtrip date times", {
  x1 <- c(as.Date("2010-01-01"), NA)
  x2 <- as.POSIXct(x1)
  attr(x2, "tzone") <- "UTC"

  expect_equal(roundtrip_var(x1, "sas"), x1)
  expect_equal(roundtrip_var(x2, "sas"), x2)
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
