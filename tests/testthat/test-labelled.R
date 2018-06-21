context("Labelled")

test_that("x must be numeric or character", {
  expect_error(labelled(TRUE), "must be a numeric or a character vector")
})

test_that("x and labels must be compatible", {
  expect_error(labelled(1, "a"), "must be same type")
  expect_error(labelled(1, c(female = 2L, male = 1L)), NA)
  expect_error(labelled(1L, c(female = 2, male = 1)), NA)
})

test_that("labels must have names", {
  expect_error(labelled(1, 1), "must have names")
})

# methods -----------------------------------------------------------------

test_that("printed output is stable", {
  x <- labelled(
    c(1:5, NA, tagged_na("x", "y", "z")),
    c(
      Good = 1,
      Bad = 5,
      "Not Applicable" = tagged_na("x"),
      "Refused to answer" = tagged_na("y")
    )
  )

  expect_output_file(print(x), "labelled-output.txt")
})

test_that("given correct name in data frame", {
  x <- labelled(1:3, c(a = 1))

  expect_named(data.frame(x), "x")
  expect_named(data.frame(y = x), "y")
})

test_that("can convert to factor with using labels with labelled na's", {
  x <- labelled(c(1:2, tagged_na("a")), c(a = 1, c = tagged_na("a")))
  expect_equal(as_factor(x, "labels"), factor(c("a", NA, "c")))
})
