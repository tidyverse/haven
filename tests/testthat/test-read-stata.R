context("read_stata")

test_that("stata data types read into expected types (#45)", {
  df <- read_stata("types.dta")
  types <- vapply(df, typeof, character(1))

  expect_equal(types, c(
    vfloat = "double",
    vdouble = "double",
    vlong = "integer",
    vint = "integer",
    vbyte = "integer"
  ))
})
