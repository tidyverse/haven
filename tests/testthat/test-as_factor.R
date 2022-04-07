# Base types --------------------------------------------------------------

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

test_that("values preserves order if possible", {
  s1 <- labelled(c("M", "M", "F"), c(Male = "M", Female = "F"))
  exp <- factor(c("M", "M", "F"), levels = c("M", "F"))
  expect_equal(as_factor(s1, "values"), exp)
})

test_that("otherwise falls back to alphabetical", {
  s1 <- labelled(c("M", "M", "F", "G"), c(Male = "M", Female = "F"))
  exp <- factor(c("M", "M", "F", "G"), levels = c("F", "G", "M"))
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

test_that("as_factor labels works with non-unique labels", {
  s1 <- labelled(1:2, c("label" = 1, "label" = 2))
  exp <- factor(c("label", "label"), levels = "label")
  expect_equal(as_factor(s1, "labels"), exp)
})


# Variable label
test_that("variable label is kept when converting labelled to factor (#178)", {
  s1 <- labelled(1:3, c("A" = 2))
  attr(s1, "label") <- "labelled"
  expect_identical(attr(as_factor(s1), "label"), "labelled")
})

# data frames -------------------------------------------------------------

test_that("... passed along", {
  df <- data.frame(x = labelled(2:1, c("A" = 1)))
  out <- as_factor(df, "both")

  expect_equal(levels(out$x), c("[1] A", "2"))
})


# replace_with ------------------------------------------------------------

test_that("updates numeric values", {
  x <- 1:5

  expect_equal(replace_with(x, -1, 5), x)
  expect_equal(replace_with(x, 1, 5), c(5, 2:5))
  expect_equal(replace_with(x, 5, 1), c(1:4, 1))

  expect_equal(replace_with(x, 1:5, rep(1, 5)), rep(1, 5))
})

test_that("udpates tagged NAs", {
  x <- c(tagged_na("a"), 1:3)

  expect_equal(replace_with(x, tagged_na("a"), 0), 0:3)
})
