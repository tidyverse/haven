context("as_factor")

# Base types --------------------------------------------------------------

test_that("leaves factors unchanged", {
  f <- factor(letters, ordered = TRUE)
  expect_equal(as_factor(f), f)
})

test_that("converts characters to factors", {
  expect_equal(as_factor(letters), factor(letters))
})

test_that("variable label is kept when converting characters to factors (#178)", {
  s1 <- structure(letters, "label" = "letters")
  expect_identical(attr(as_factor(s1), "label"), "letters")
})

# Labelled values ---------------------------------------------------------

test_that("all labels (implicit missing values) are preserved when levels is 'default' or 'both' (#172)", {
  s1 <- labelled(rep(1, 3), c("A" = 1, "B" = 2, "C" = 3))
  exp <- factor(rep("A", 3), levels = c("A", "B", "C"))
  expect_equal(as_factor(s1), exp)

  exp <- factor(rep("[1] A", 3), levels = c("[1] A", "[2] B", "[3] C"))
  expect_equal(as_factor(s1, levels = "both"), exp)
})

test_that("all labels (existing and missing) are sorted by values (#172)", {
  s1 <- labelled(c(1, 4), c("Agree" = 1, "Neutral" = 2, "Disagree" = 3, "Don't know" = 5))
  exp <- factor(c("Agree", "4"), levels = c("Agree", "Neutral", "Disagree", "4", "Don't know"))
  expect_equal(as_factor(s1), exp)
})

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

test_that("converts tagged NAs", {
  s1 <- labelled(c(1:2, tagged_na("a", "b")), c("Apple" = tagged_na("a")))
  exp <- factor(c("1", "2", "Apple", NA))
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

# Variable label
test_that("variable label is kept when converting labelled to factor (#178)", {
  s1 <- labelled(1:3, c("A" = 2))
  attr(s1, "label") <- "labelled"
  expect_identical(attr(as_factor(s1), "label"), "labelled")
})
