context("write")

# writing uses exactly the same path for sav and dta, so don't need
# to test both

test_that("can roundtrip basic types", {
  x <- runif(10)
  expect_equal(roundtrip_var(x), x)
  expect_equal(roundtrip_var(1:10), 1:10)
  expect_equal(roundtrip_var(c(TRUE, FALSE)), c(1, 0))
  expect_equal(roundtrip_var(letters), letters)
})

test_that("can roundtrip missing values (as much as possible)", {
  expect_equal(roundtrip_var(NA), NA_integer_)
  expect_equal(roundtrip_var(NA_real_), NA_real_)
  expect_equal(roundtrip_var(NA_integer_), NA_integer_)
  expect_equal(roundtrip_var(NA_character_), "")
})

test_that("infinity gets converted to NA", {
  expect_equal(roundtrip_var(c(Inf, 0, -Inf)), c(NA, 0, NA))
})

test_that("factors become labelleds", {
  f <- factor(c("a", "b"), levels = letters[1:3])
  rt <- roundtrip_var(f)

  expect_is(rt, "labelled")
  expect_equal(as.vector(rt), 1:2)
  expect_equal(attr(rt, "labels"), c(a = 1, b = 2, c = 3))
})

test_that("labels are preserved", {
  x <- 1:10
  attr(x, "label") <- "abc"

  expect_equal(attr(roundtrip_var(x), "label"), "abc")
})

test_that("labelleds are round tripped", {
  int <- labelled(c(1L, 2L), c(a = 1L, b = 3L))
  num <- labelled(c(1, 2), c(a = 1, b = 3))
  chr <- labelled(c("a", "b"), c(a = "b", b = "a"))

  expect_equal(roundtrip_var(int), int)
  expect_equal(roundtrip_var(num), num)
  expect_equal(roundtrip_var(chr), chr)
})

test_that("factors become labelleds", {
  f <- factor(c("a", "b"), levels = letters[1:3])
  rt <- roundtrip_var(f)

  expect_is(rt, "labelled")
  expect_equal(as.vector(rt), 1:2)
  expect_equal(attr(rt, "labels"), c(a = 1, b = 2, c = 3))
})
