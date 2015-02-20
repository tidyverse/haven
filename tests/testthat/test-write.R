context("write")

# writing uses exactly the same path for sav and dta, so don't need
# to test both

roundtrip <- function(x) {
  tmp <- tempfile()
  write_sav(x, tmp)
  read_sav(tmp)
}

roundtrip_var <- function(x) {
  df <- list(x = x)
  class(df) <- "data.frame"
  attr(df, "row.names") <- .set_row_names(length(x))
  roundtrip(df)$x
}

test_that("can roundtrip basic types", {
  x <- runif(10)
  expect_equal(roundtrip_var(x), x)
  expect_equal(roundtrip_var(1:10), 1:10)
  expect_equal(roundtrip_var(c(TRUE, FALSE)), c(1, 0))
  expect_equal(roundtrip_var(letters), letters)
})

test_that("can roundtrip missing values (as much as possible)", {
  expect_equal(roundtrip_var(NA), NA_integer_)
  expect_equal(roundtrip_var(NA_real_), NA_real_)
  expect_equal(roundtrip_var(NA_integer_), NA_integer_)
  expect_equal(roundtrip_var(NA_character_), "")
})

