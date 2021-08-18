test_that("force_utc doesn't preserve attributes we don't want", {
  x_ct <- as.POSIXct("2010-01-01 09:00", tz = "Pacific/Auckland")
  x_ct_forced <- force_utc(x_ct)
  expect_s3_class(x_ct_forced, "POSIXct")
  expect_null(attr(x_ct_forced, "names"))
  expect_equal(attr(x_ct_forced, "tzone"), "UTC")

  x_lt <- as.POSIXlt("2010-01-01 09:00", tz = "Pacific/Auckland")
  x_lt_forced <- force_utc(x_lt)
  expect_s3_class(x_lt_forced, "POSIXct")
  expect_null(attr(x_lt_forced, "names"))
  expect_equal(attr(x_lt_forced, "tzone"), "UTC")
})
