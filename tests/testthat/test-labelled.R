context("Labelled")

test_that("can coerce single value labelled vectors into factor", {
  var <- labelled(1L, c(female = 1L, male = 2L))
  expect_equal(as_factor(var), factor("female", levels = c("female", "male")))
})
