context("read_sav")

test_that("variable label stored as attributes", {
  df <- read_sav("variable-label.sav")
  expect_equal(attr(df$sex, "label"), "Gender")
})

test_that("value labels stored as labelled class", {
  num <- read_sav("labelled-num.sav")
  str <- read_sav("labelled-str.sav")

  expect_equal(num[[1]], labelled(1, c("This is one" = 1)))
  expect_equal(str[[1]], labelled(c("M", "F"), c(Female = "F", Male = "M")))
})
