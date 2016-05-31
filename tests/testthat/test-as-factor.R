context("as_factor")

# Base types --------------------------------------------------------------

test_that("leaves factors unchanged", {
  f <- factor(letters, ordered = TRUE)
  expect_equal(as_factor(f), f)
})

test_that("converts characters to factors", {
  expect_equal(as_factor(letters), factor(letters))
})

# Labelled values ---------------------------------------------------------

test_that("all values are preserved", {
  s1 <- labelled(1:3, c("A" = 2))
  exp <- factor(c("1", "A", "3"), levels = c("1", "A", "3"))
  expect_equal(as_factor(s1), exp)
})

test_that("character labelled converts to factor", {
  s1 <- labelled(c("M", "M", "F"), c(Male = "M", Female = "F"))
  exp <- factor(c("Male", "Male", "Female"), levels = c("Female", "Male"))
  expect_equal(as_factor(s1), exp)
})

# Both

test_that("both combines values and levels", {
  s1 <- labelled(2:1, c("A" = 1))
  exp <- factor(c("2", "[1] A"), levels = c("[1] A", "2"))
  expect_equal(as_factor(s1, "both"), exp)
})

# Values

test_that("character labelled uses values when requested", {
  s1 <- labelled(c("M", "M", "F"), c(Male = "M", Female = "F"))
  exp <- factor(c("M", "M", "F"), levels = c("M", "F"))
  expect_equal(as_factor(s1, "values"), exp)
})

# Labels

test_that("labels preserves all label values", {
  var <- labelled(1L, c(female = 1L, male = 2L))
  expect_equal(as_factor(var, "labels"), factor("female", levels = c("female", "male")))
})

test_that("order of labels doesn't matter", {
  var <- labelled(1L, c(female = 2L, male = 1L))
  expect_equal(as_factor(var, "labels"), factor("male", levels = c("female", "male")))
})
