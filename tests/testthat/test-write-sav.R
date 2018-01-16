context("write_sav")

test_that("can roundtrip basic types", {
  x <- runif(10)
  expect_equal(roundtrip_var(x, "sav"), x)
  expect_equal(roundtrip_var(1:10, "sav"), 1:10)
  expect_equal(roundtrip_var(c(TRUE, FALSE), "sav"), c(1, 0))
  expect_equal(roundtrip_var(letters, "sav"), letters)
})

test_that("can roundtrip missing values (as much as possible)", {
  expect_equal(roundtrip_var(NA, "sav"), NA_integer_)
  expect_equal(roundtrip_var(NA_real_, "sav"), NA_real_)
  expect_equal(roundtrip_var(NA_integer_, "sav"), NA_integer_)
  expect_equal(roundtrip_var(NA_character_, "sav"), "")
})

test_that("can roundtrip date times", {
  x1 <- c(as.Date("2010-01-01"), NA)
  x2 <- as.POSIXct(x1)
  attr(x2, "tzone") <- "UTC"

  expect_equal(roundtrip_var(x1, "sav"), x1)
  expect_equal(roundtrip_var(x2, "sav"), x2)
})

test_that("can roundtrip times", {
  x <- hms::hms(c(1, NA, 86400))
  expect_equal(roundtrip_var(x, "sav"), x)
})

test_that("infinity gets converted to NA", {
  expect_equal(roundtrip_var(c(Inf, 0, -Inf), "sav"), c(NA, 0, NA))
})

test_that("factors become labelleds", {
  f <- factor(c("a", "b"), levels = letters[1:3])
  rt <- roundtrip_var(f, "sav")

  expect_is(rt, "labelled")
  expect_equal(as.vector(rt), 1:2)
  expect_equal(attr(rt, "labels"), c(a = 1, b = 2, c = 3))
})

test_that("labels are preserved", {
  x <- 1:10
  attr(x, "label") <- "abc"

  expect_equal(attr(roundtrip_var(x, "sav"), "label"), "abc")
})

test_that("labelleds are round tripped", {
  int <- labelled(c(1L, 2L), c(a = 1L, b = 3L))
  num <- labelled(c(1, 2), c(a = 1, b = 3))
  chr <- labelled(c("a", "b"), c(a = "b", b = "a"))

  expect_equal(roundtrip_var(int, "sav"), int)
  expect_equal(roundtrip_var(num, "sav"), num)
  expect_equal(roundtrip_var(chr, "sav"), chr)
})

test_that("spss labelleds are round tripped", {
  df <- tibble(
    x = labelled_spss(
      c(1, 2, 1, 9),
      labels = c(no = 1, yes = 2, unknown = 9),
      na_values = 9,
      na_range = c(80, 90)
    )
  )

  path <- tempfile()
  write_sav(df, path)

  df2 <- read_sav(path)
  expect_s3_class(df2$x, "labelled")

  df3 <- read_sav(path, user_na = TRUE)
  expect_s3_class(df3$x, "labelled_spss")
  expect_equal(attr(df3$x, "na_values"), attr(df$x, "na_values"))
  expect_equal(attr(df3$x, "na_range"), attr(df$x, "na_range"))
})

test_that("factors become labelleds", {
  f <- factor(c("a", "b"), levels = letters[1:3])
  rt <- roundtrip_var(f, "sav")

  expect_is(rt, "labelled")
  expect_equal(as.vector(rt), 1:2)
  expect_equal(attr(rt, "labels"), c(a = 1, b = 2, c = 3))
})

test_that("labels are converted to utf-8", {
  labels_utf8 <- c("\u00e9\u00e8", "\u00e0", "\u00ef")
  labels_latin1 <- iconv(labels_utf8, "utf-8", "latin1")

  v_utf8 <- labelled(3:1, setNames(1:3, labels_utf8))
  v_latin1 <- labelled(3:1, setNames(1:3, labels_latin1))

  expect_equal(names(attr(roundtrip_var(v_utf8, "sav"), "labels")), labels_utf8)
  expect_equal(names(attr(roundtrip_var(v_latin1, "sav"), "labels")), labels_utf8)
})


test_that("complain about long factor labels", {
  x <- paste(rep("a", 200), collapse = "")
  df <- data.frame(x = factor(x))

  expect_error(roundtrip_sav(df), "levels with <= 120 characters")
})
