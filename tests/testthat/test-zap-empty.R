context("zap_empty")

test_that("empty strings replaced with missing", {
  x <- c("", "a", NA)
  expect_equal(zap_empty(x), c(NA, "a", NA))
})
