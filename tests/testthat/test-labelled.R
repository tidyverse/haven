context("Labelled")

test_that("pseudo-missings are converted to NA", {
  var <- labelled(c("a", "a", "b"), c(a = "A", b = "B"), NULL, c(TRUE, TRUE, FALSE))
  normalised_var <- labelled(c(NA, NA, "b"), c(a = "A", b = "B"))
  expect_equal(normalise_na(var), normalised_var)

  var_nolabels <- labelled(1:3)
  expect_equal(var_nolabels, normalise_na(var_nolabels))
})

test_that("character labelled converts to factor", {
  s1 <- labelled(c("M", "M", "F"), c(Male = "M", Female = "F"))
  exp <- factor(c("Male", "Male", "Female"), levels = c("Male", "Female"))
  expect_equal(as_factor(s1), exp)
})

test_that("character labelled uses values when requested factor", {
  s1 <- labelled(c("M", "M", "F"), c(Male = "M", Female = "F"))
  exp <- factor(c("M", "M", "F"), levels = c("M", "F"))
  expect_equal(as_factor(s1, "values"), exp)
})

test_that("can coerce single value labelled vectors into factor", {
  var <- labelled(1L, c(female = 1L, male = 2L))
  expect_equal(as_factor(var), factor("female", levels = c("female", "male")))
})

test_that("integer labels that are not equal to vector positions work", {
  var <- labelled(1L, c(female = 2L, male = 1L))
  expect_equal(as_factor(var), factor("male", levels = c("female", "male")))
})

test_that("integer labels that are larger then label list work", {
  var <- labelled(11L, c(female = 11L, male = 12L))
  expect_equal(as_factor(var), factor("female", levels = c("female", "male")))
})

test_that("multiple NA values turned into NA", {
  var <- labelled(
    c("a", "b", "c", "d"),
    c(A = "a", B = "b", C = "c", D = "d"),
    is_na = c(FALSE, TRUE, TRUE, FALSE),
    missings = c(FALSE, TRUE, TRUE, TRUE)
  )
  expect_equal(as_factor(var), factor(c("A", NA, NA, NA)))
})

test_that("multiple NA values preserved if drop_na = FALSE", {
  var <- labelled(
    c("a", "b", "c", "d"),
    c(A = "a", B = "b", C = "c", D = "d"),
    is_na = c(FALSE, TRUE, TRUE, FALSE),
    missings = c(FALSE, TRUE, TRUE, TRUE)
  )
  expect_equal(as_factor(var, drop_na = FALSE), factor(c("A", "B", "C", "D")))
})

test_that("zap_labels replaces labels with NAs for labelled variable", {
  var <- labelled(
    c(1L, 98L, 99L),
    c(not_answered = 98L, not_applicable = 99L)
  )
  exp <- c(1L,NA,NA)
  expect_equal(zap_labels(var), exp)
})

test_that("zap_labels returns variables not of class('labelled') unmodified", {
  var <- c(1L, 98L, 99L)
  expect_equal(zap_labels(var), var)
})

test_that("is_missing() works with non-labelled variables", {
  var <- 1:3
  expect_equal(is_missing(var), rep(FALSE, 3))
})

test_that("all arguments can be NULL", {
  var <- labelled(1:3)
  expected <- structure(1:3, class = "labelled", is_na = logical(0))
  expect_equal(var, expected)
})

test_that("as_factor() works with NULL labels", {
  var_nolabels <- labelled(c("a", "a", "b", "c"))
  expect_equal(as_factor(var_nolabels), factor(var_nolabels))
  expect_equal(as_factor(var_nolabels, levels = "values"), factor(var_nolabels)) 
})

test_that("missings attribute is subsetted as well", {
  var <- labelled(1:4, missings = c(TRUE, FALSE, FALSE, TRUE))
  var <- var[3:4]
  expect_equal(is_missing(var), c(FALSE, TRUE))
})

test_that("missings attribute is correctly implied from is_na", {
  var <- labelled(
    c(-1, 1, 1, 8, 9),
    c(value = 1, missing1 = 8, missing2 = 9),
    c(FALSE, TRUE, TRUE)
  )
  expect_equal(is_missing(var), c(FALSE, FALSE, FALSE, TRUE, TRUE))
})

test_that("labelled() fails when missingness patterns are inconsistent", {
  expect_error(labelled(
    c(-1, 1, 1, 8, 9),
    c(value = 1, missing1 = 8, missing2 = 9),
    c(TRUE, TRUE, TRUE),
    c(TRUE, FALSE, FALSE, TRUE, TRUE)
  ))

  var <- labelled(
    c(-1, 1, 1, 8, 9),
    c(value = 1, missing1 = 8, missing2 = 9),
    c(FALSE, TRUE, TRUE),
    c(TRUE, FALSE, FALSE, TRUE, TRUE)
  )
  expect_equal(is_missing(var), c(TRUE, FALSE, FALSE, TRUE, TRUE))
})
