context("write_xpt")

test_that("can roundtrip basic types", {
  x <- runif(10)
  expect_equal(roundtrip_var(x, "xpt"), x)
  expect_equal(roundtrip_var(1:10, "xpt"), 1:10)
  expect_equal(roundtrip_var(c(TRUE, FALSE), "xpt"), c(1, 0))
  expect_equal(roundtrip_var(letters, "xpt"), letters)
})

test_that("can roundtrip missing values (as much as possible)", {
  expect_equal(roundtrip_var(NA, "xpt"), NA_integer_)
  expect_equal(roundtrip_var(NA_real_, "xpt"), NA_real_)
  expect_equal(roundtrip_var(NA_integer_, "xpt"), NA_integer_)
  expect_equal(roundtrip_var(NA_character_, "xpt"), "")
})
