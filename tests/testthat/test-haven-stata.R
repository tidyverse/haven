# read_stata --------------------------------------------------------------

test_that("stata data types read into expected types (#45)", {
  df <- read_stata(test_path("stata/types.dta"))
  types <- vapply(df, typeof, character(1))

  expect_equal(types, c(
    vfloat = "double",
    vdouble = "double",
    vlong = "double",
    vint = "double",
    vbyte = "double",
    vstr = "character",
    vdate = "double",
    vdatetime = "double"
  ))
})

test_that("Stata %td (date) and %tc (datetime) read into expected classes", {
  df <- read_stata(test_path("stata/types.dta"))

  expect_s3_class(df$vdate, "Date")
  expect_s3_class(df$vdatetime, "POSIXct")
})


test_that("Old %d format read into Date class", {
  df <- zap_formats(read_stata(test_path("stata/datetime-d.dta")))
  expect_equal(df$date, as.Date("2015-11-02"))
})


test_that("tagged double missings are read correctly", {
  x <- read_dta(test_path("stata/tagged-na-double.dta"))$x
  expect_equal(na_tag(x), c(rep(NA, 5), "a", "h", "z"))

  labels <- attr(x, "labels")
  expect_equal(na_tag(labels), c("a", "z"))
})

test_that("tagged integer missings are read correctly", {
  x <- read_dta(test_path("stata/tagged-na-int.dta"))$x
  expect_equal(na_tag(x), c(rep(NA, 5), "a", "h", "z"))

  labels <- attr(x, "labels")
  expect_equal(na_tag(labels), c("a", "z"))
})

test_that("file label and notes stored as attributes", {
  df <- read_dta(test_path("stata/notes.dta"))

  expect_equal(attr(df, "label"), "This is a test dataset.")
  expect_length(attr(df, "notes"), 2)
})

test_that("only selected columns are read", {
  out <- read_dta(test_path("stata/notes.dta"), col_select = "id")
  expect_named(out, "id")
})

test_that("using skip returns correct number of rows", {
  rows_after_skipping <- function(n) {
    nrow(read_dta(test_path("stata/notes.dta"), skip = n))
  }

  n <- rows_after_skipping(0)

  expect_equal(rows_after_skipping(1), n - 1)
  expect_equal(rows_after_skipping(n - 1), 1)
  expect_equal(rows_after_skipping(n + 0), 0)
  expect_equal(rows_after_skipping(n + 1), 0)
})

test_that("can limit the number of rows to read", {
  rows_with_limit <- function(n) {
    nrow(read_dta(test_path("stata/notes.dta"), n_max = n))
  }

  n <- rows_with_limit(Inf)
  expect_equal(rows_with_limit(0), 0)
  expect_equal(rows_with_limit(1), 1)
  expect_equal(rows_with_limit(n), n)
  expect_equal(rows_with_limit(n + 1), n)

  # alternatives for unlimited rows
  expect_equal(rows_with_limit(NA), n)
  expect_equal(rows_with_limit(-1), n)
})

# write_dta ---------------------------------------------------------------

test_that("can roundtrip basic types", {
  x <- runif(10)
  expect_equal(roundtrip_var(x, "dta"), x)
  expect_equal(roundtrip_var(1:10, "dta"), 1:10)
  expect_equal(roundtrip_var(c(TRUE, FALSE), "dta"), c(1, 0))
  expect_equal(roundtrip_var(letters, "dta"), letters)
})

test_that("can roundtrip missing values (as much as possible)", {
  expect_equal(roundtrip_var(NA, "dta"), NA_integer_)
  expect_equal(roundtrip_var(NA_real_, "dta"), NA_real_)
  expect_equal(roundtrip_var(NA_integer_, "dta"), NA_integer_)
  expect_equal(roundtrip_var(NA_character_, "dta"), "")
})

test_that("can roundtrip date times", {
  x1 <- c(as.Date("2010-01-01"), NA)
  expect_equal(roundtrip_var(x1, "dta"), x1)

  # converted to same time in UTC
  x2 <- as.POSIXct("2010-01-01 09:00", tz = "Pacific/Auckland")
  expect_equal(
    roundtrip_var(x2, "dta"),
    as.POSIXct("2010-01-01 09:00", tz = "UTC")
  )

  attr(x2, "label") <- "abc"
  expect_equal(attr(roundtrip_var(x2, "dta"), "label"), "abc")
})

test_that("can roundtrip tagged NAs", {
  x <- c(1, 2, tagged_na("a", "b"), NA)
  expect_equal(roundtrip_var(x, "dta"), x)

  tags <- tagged_na("a", "b")
  y <- labelled(
    c(1, 2, 1, tags[1], tags[2]),
    c("ABC" = tags[1], "DEF" = tags[2])
  )
  expect_equal(roundtrip_var(y, "dta"), y)
})

test_that("infinity gets converted to NA", {
  expect_equal(roundtrip_var(c(Inf, 0, -Inf), "dta"), c(NA, 0, NA))
})

test_that("factors become labelleds", {
  f <- factor(c("a", "b"), levels = letters[1:3])
  rt <- roundtrip_var(f, "dta")

  expect_s3_class(rt, "haven_labelled")
  expect_equal(as.vector(rt), 1:2)
  expect_equal(attr(rt, "labels"), c(a = 1, b = 2, c = 3))
})

test_that("labels are preserved", {
  x <- 1:10
  attr(x, "label") <- "abc"

  expect_equal(attr(roundtrip_var(x, "dta"), "label"), "abc")
})

test_that("labelleds are round tripped", {
  int <- labelled(c(1L, 2L), c(a = 1L, b = 3L))
  num <- labelled(c(1, 2), c(a = 1, b = 3))
  chr <- labelled(c("a", "b"), c(a = "b", b = "a"))

  expect_equal(roundtrip_var(num, "dta"), num)
  # FIXME!
  # expect_equal(roundtrip_var(chr, "dta"), chr)
})

test_that("can write labelled with NULL labels", {
  int <- labelled(c(1L, 2L), NULL)
  num <- labelled(c(1, 2), NULL)
  chr <- labelled(c("a", "b"), NULL)

  expect_equal(roundtrip_var(int, "dta"), c(1L, 2L))
  expect_equal(roundtrip_var(num, "dta"), c(1L, 2L))
  expect_equal(roundtrip_var(chr, "dta"), c("a", "b"))
})

test_that("factors become labelleds", {
  f <- factor(c("a", "b"), levels = letters[1:3])
  rt <- roundtrip_var(f, "dta")

  expect_s3_class(rt, "haven_labelled")
  expect_equal(as.vector(rt), 1:2)
  expect_equal(attr(rt, "labels"), c(a = 1, b = 2, c = 3))
})

test_that("labels are converted to utf-8", {
  labels_utf8 <- c("\u00e9\u00e8", "\u00e0", "\u00ef")
  labels_latin1 <- iconv(labels_utf8, "utf-8", "latin1")

  v_utf8 <- labelled(3:1, setNames(1:3, labels_utf8))
  v_latin1 <- labelled(3:1, setNames(1:3, labels_latin1))

  expect_equal(names(attr(roundtrip_var(v_utf8, "dta"), "labels")), labels_utf8)
  expect_equal(names(attr(roundtrip_var(v_latin1, "dta"), "labels")), labels_utf8)
})

test_that("supports stata version 15", {
  df <- tibble(x = factor(letters), y = runif(26))

  path <- tempfile()
  write_dta(df, path, version = 15)
  df2 <- read_dta(path)

  df2$x <- as_factor(df2$x)
  df2$y <- zap_formats(df2$y)
  expect_equal(df2, df)
})

test_that("can roundtrip file labels", {
  df <- tibble(x = 1)
  expect_null(attr(roundtrip_dta(df), "label"))
  expect_equal(attr(roundtrip_dta(df, label = "abcd"), "label"), "abcd")

  attr(df, "label") <- "abc"
  expect_equal(attr(roundtrip_dta(df), "label"), "abc")
  expect_equal(attr(roundtrip_dta(df, label = "abcd"), "label"), "abcd")
  expect_null(attr(roundtrip_dta(df, label = NULL), "label"))
})

test_that("invalid files generate informative errors", {
  expect_snapshot(error = TRUE, {
    long <- paste(rep("a", 100), collapse = "")
    write_dta(data.frame(x = 1), tempfile(), label = long)

    df <- data.frame(1)
    names(df) <- "x y"
    write_dta(df, tempfile(), version = 13)

    names(df) <- long
    write_dta(df, tempfile(), version = 13)
    write_dta(df, tempfile(), version = 14)
  })
})

test_that("can't write non-integer labels (#401)", {
  expect_snapshot(error = TRUE, {
    df <- data.frame(x = labelled(c(1, 2.5, 3), c("b" = 1.5)))
    write_dta(df, tempfile())
  })
})

test_that("can roundtrip long strings (strL)", {
  long_string <- function(n, m) {
    do.call("paste0", replicate(m, sample(LETTERS, n, TRUE), simplify = FALSE))
  }

  x <- long_string(10, 400)
  expect_equal(roundtrip_var(x, "dta"), x)
  x <- long_string(10, 1000)
  expect_equal(roundtrip_var(x, "dta"), x)
  x <- long_string(10, 3000)
  expect_equal(roundtrip_var(x, "dta"), x)
})
