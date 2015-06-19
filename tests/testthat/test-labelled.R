context("Labelled")

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

test_that("zap_labels replaces labels with NAs for labelled variable", {
    var <- labelled(c(1L, 98L, 99L),  c(not_answered = 98L, not_applicable = 99L))
    exp <- c(1L,NA,NA)
    expect_equal(zap_labels(var), exp)
})

test_that("zap_labels returns variables not of class('labelled') unmodified", {
    var <- c(1L, 98L, 99L)
    expect_equal(zap_labels(var), var)
})
