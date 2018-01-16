context("test-read-xpt.R")

test_that("can read date/times", {
  x <- as.Date("2018-01-01")
  df <- data.frame(date = x, datetime = as.POSIXct(x))
  path <- tempfile()
  write_xpt(df, path)

  res <- read_xpt(path)
  expect_s3_class(res$date, "Date")
  expect_s3_class(res$datetime, "POSIXct")
})
