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
