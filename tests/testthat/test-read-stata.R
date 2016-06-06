context("read_stata")

test_that("stata data types read into expected types (#45)", {
  df <- read_stata("types.dta")
  types <- vapply(df, typeof, character(1))

  expect_equal(types, c(
    vfloat = "double",
    vdouble = "double",
    vlong = "integer",
    vint = "integer",
    vbyte = "integer",
    vstr = "character",
    vdate = "integer",
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
