context("tagged_na")

test_that("tagged_na is NA (but not NaN)", {
  x <- tagged_na("a")

  expect_true(is.na(x))
  expect_false(is.nan(x))
})

# tag_na ------------------------------------------------------------------

test_that("can extract value of tagged na", {
  expect_equal(na_tag(tagged_na(letters)), letters)
})

test_that("tag of system NA is NA", {
  expect_equal(na_tag(NA_real_), NA_character_)
})

test_that("tag of non-NA is NA", {
  expect_equal(na_tag(1), NA_character_)
})

# is_tagged_na ------------------------------------------------------------

test_that("regular NA isn't tagged", {
  expect_false(is_tagged_na(NA_real_))
})

test_that("non-missing isn't tagged", {
  expect_false(is_tagged_na(1))
})

test_that("tagged values are tagged", {
  x <- tagged_na(c("a", "z"))
  expect_equal(is_tagged_na(x), c(TRUE, TRUE))
})

test_that("values are checked if required", {
  x <- tagged_na(c("a", "z"))
  expect_equal(is_tagged_na(x, "a"), c(TRUE, FALSE))
})

# character output -----------------------------------------------------------

test_that("format_tagged_na displays tagged NA's specially", {
  x <- c(1, tagged_na("a"), NA)
  expect_equal(format_tagged_na(x), c(
    "    1",
    "NA(a)",
    "   NA"
  ))
})

test_that("print_tagged_na is stable", {
  x <- c(1:100, tagged_na(letters), NA)
  expect_output_file(print_tagged_na(x), "tagged-na.txt")
})
