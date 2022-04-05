# read_spss ---------------------------------------------------------------

test_that("variable label stored as attributes", {
  df <- read_spss(test_path("spss/variable-label.sav"))
  expect_equal(attr(df$sex, "label"), "Gender")
})

test_that("value labels stored as labelled class", {
  num <- zap_formats(read_spss(test_path("spss/labelled-num.sav")))
  str <- zap_formats(read_spss(test_path("spss/labelled-str.sav")))

  expect_equal(num[[1]], labelled(1, c("This is one" = 1)))
  expect_equal(str[[1]], labelled(c("M", "F"), c(Female = "F", Male = "M")))
})

test_that("value labels read in as same type as vector", {
  df <- read_spss(test_path("spss/variable-label.sav"))
  num <- read_spss(test_path("spss/labelled-num.sav"))
  str <- read_spss(test_path("spss/labelled-str.sav"))

  expect_equal(typeof(df$sex), typeof(attr(df$sex, "labels")))
  expect_equal(typeof(num[[1]]), typeof(attr(num[[1]], "labels")))
  expect_equal(typeof(str[[1]]), typeof(attr(str[[1]], "labels")))
})

test_that("non-ASCII labels converted to utf-8", {
  x <- read_spss(test_path("spss/umlauts.sav"))[[1]]

  expect_equal(attr(x, "label"), "This is an \u00e4-umlaut")
  expect_equal(names(attr(x, "labels"))[1], "the \u00e4 umlaut")
})

test_that("datetime variables converted to the correct class", {
  df <- read_spss(test_path("spss/datetime.sav"))
  expect_true(inherits(df$date, "Date"))
  expect_true(inherits(df$date.posix, "POSIXct"))
  expect_true(inherits(df$time, "hms"))
})

test_that("datetime values correctly imported (offset)", {
  df <- read_spss(test_path("spss/datetime.sav"))
  expect_equal(df$date[1], as.Date("2014-09-22d"))
  expect_equal(df$date.posix[2], as.POSIXct("2014-09-23 15:59:20", tz = "UTC"))
  expect_equal(as.integer(df$time[1]), 43870)
})

test_that("formats roundtrip", {
  df <- tibble::tibble(
    a = structure(c(1, 1, 2), format.spss = "F1.0"),
    b = structure(4:6, format.spss = "F2.1"),
    c = structure(7:9, format.spss = "N2"),
    d = structure(c("Text", "Text", ""), format.spss = "A100")
  )

  tmp <- tempfile()
  on.exit(unlink(tmp))

  write_sav(df, tmp)
  df2 <- read_sav(tmp)

  expect_equal(df$a, df2$a)
  expect_equal(df$b, df2$b)
  expect_equal(df$c, df2$c)
  expect_equal(df$d, df2$d)
})

test_that("widths roundtrip", {
  df <- tibble::tibble(
    a = structure(c(1, 1, 2), display_width = 10),
    b = structure(4:6, display_width = 11),
    c = structure(7:9, display_width = 12),
    d = structure(c("Text", "Text", ""), display_width = 10)
  )

  tmp <- tempfile()
  on.exit(unlink(tmp))

  write_sav(df, tmp)
  df2 <- read_sav(tmp)

  expect_equal(df$a, zap_formats(df2$a))
  expect_equal(df$b, zap_formats(df2$b))
  expect_equal(df$c, zap_formats(df2$c))
  expect_equal(df$d, zap_formats(df2$d))
})

test_that("only selected columns are read", {
  out <- read_spss(test_path("spss/datetime.sav"), col_select = "date")
  expect_named(out, "date")
})

# Row skipping/limiting --------------------------------------------------------

test_that("using skip returns correct number of rows", {
  rows_after_skipping <- function(n) {
    nrow(read_spss(test_path("spss/datetime.sav"), skip = n))
  }

  n <- rows_after_skipping(0)

  expect_equal(rows_after_skipping(1), n - 1)
  expect_equal(rows_after_skipping(n - 1), 1)
  expect_equal(rows_after_skipping(n + 0), 0)
  expect_equal(rows_after_skipping(n + 1), 0)
})

test_that("can limit the number of rows to read", {
  rows_with_limit <- function(n) {
    nrow(read_spss(test_path("spss/datetime.sav"), n_max = n))
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

# User-defined missings ---------------------------------------------------

test_that("user-defined missing values read as missing by default", {
  num <- read_spss(test_path("spss/labelled-num-na.sav"))[[1]]
  expect_equal(vec_data(num)[[2]], NA_real_)
})

test_that("user-defined missing values can be preserved", {
  num <- read_spss(test_path("spss/labelled-num-na.sav"), user_na = TRUE)[[1]]

  expect_s3_class(num, "haven_labelled_spss")
  expect_equal(vec_data(num)[[2]], 9)

  expect_equal(attr(num, "na_values"), 9)
  expect_equal(attr(num, "na_range"), NULL)

  num
})

test_that("system missings read as NA", {
  df <- tibble::tibble(x = c(1, NA))
  out <- roundtrip_sav(df)

  expect_identical(df$x, c(1, NA))
})


# write_sav ---------------------------------------------------------------

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
  expect_equal(roundtrip_var(x1, "sav"), x1)

  # converted to same time in UTC
  x2 <- as.POSIXct("2010-01-01 09:00", tz = "Pacific/Auckland")
  expect_equal(
    roundtrip_var(x2, "sav"),
    as.POSIXct("2010-01-01 09:00", tz = "UTC")
  )

  attr(x2, "label") <- "abc"
  expect_equal(attr(roundtrip_var(x2, "sav"), "label"), "abc")
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

  expect_s3_class(rt, "haven_labelled")
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

  expect_equal(roundtrip_var(num, "sav"), num)
  expect_equal(roundtrip_var(chr, "sav"), chr)
})

test_that("spss labelleds are round tripped", {
  df <- tibble(
    x = labelled_spss(
      c(1, 2, 1, 9, 80, 85, 90),
      labels = c(no = 1, yes = 2, unknown = 9),
      na_values = 9,
      na_range = c(80, 90)
    )
  )

  path <- tempfile()
  write_sav(df, path)

  df2 <- read_sav(path)
  expect_s3_class(df2$x, "haven_labelled")
  expect_equal(as.double(df2$x), c(1, 2, 1, NA, NA, NA, NA))

  df3 <- read_sav(path, user_na = TRUE)
  expect_s3_class(df3$x, "haven_labelled_spss")
  expect_equal(attr(df3$x, "na_values"), attr(df$x, "na_values"))
  expect_equal(attr(df3$x, "na_range"), attr(df$x, "na_range"))
})

test_that("spss integer labelleds are round tripped", {
  df <- tibble(
    x = labelled_spss(
      c(1L, 2L, 1L, 9L, 80L, 85L, 90L),
      labels = c(no = 1, yes = 2, unknown = 9),
      na_values = 9,
      na_range = c(80, 90)
    )
  )

  path <- tempfile()
  write_sav(df, path)

  df2 <- read_sav(path)
  expect_s3_class(df2$x, "haven_labelled")
  expect_equal(as.integer(df2$x), c(1, 2, 1, NA, NA, NA, NA))

  df3 <- read_sav(path, user_na = TRUE)
  expect_s3_class(df3$x, "haven_labelled_spss")
  expect_equal(attr(df3$x, "na_values"), attr(df$x, "na_values"))
  expect_equal(attr(df3$x, "na_range"), attr(df$x, "na_range"))
})


test_that("na_range roundtrips successfully with mismatched type", {
  x_vec <- 1:10
  x_na <- c(1, 10)
  df <- tibble(
    x_int_int   = labelled_spss(as.integer(x_vec), na_range = as.integer(x_na)),
    x_int_real  = labelled_spss(as.integer(x_vec), na_range = as.numeric(x_na)),
    x_real_real = labelled_spss(as.numeric(x_vec), na_range = as.numeric(x_na)),
    x_real_int  = labelled_spss(as.numeric(x_vec), na_range = as.integer(x_na))
  )

  path <- tempfile()
  write_sav(df, path)
  df2 <- read_sav(path, user_na = TRUE)

  expect_equal(attr(df2$x_int_int, "na_range"), attr(df$x_int_int, "na_range"))
  expect_equal(attr(df2$x_int_real, "na_range"), attr(df$x_int_real, "na_range"))
  expect_equal(attr(df2$x_real_real, "na_range"), attr(df$x_real_real, "na_range"))
  expect_equal(attr(df2$x_real_int, "na_range"), attr(df$x_real_int, "na_range"))
})


test_that("spss string labelleds are round tripped", {
  df <- tibble(
    x = labelled_spss(
      c("1", "2", "3", "99"),
      labels = c(one = "1"),
      na_values = "99",
      na_range = c("2", "3")
    )
  )

  path <- tempfile()
  write_sav(df, path)

  df2 <- read_sav(path)
  expect_s3_class(df2$x, "haven_labelled")
  expect_equal(as.character(df2$x), c("1", NA, NA, NA))

  df3 <- read_sav(path, user_na = TRUE)
  expect_s3_class(df3$x, "haven_labelled_spss")
  expect_equal(attr(df3$x, "na_values"), attr(df$x, "na_values"))
  expect_equal(attr(df3$x, "na_range"), attr(df$x, "na_range"))
})


test_that("factors become labelleds", {
  f <- factor(c("a", "b"), levels = letters[1:3])
  rt <- roundtrip_var(f, "sav")

  expect_s3_class(rt, "haven_labelled")
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
  expect_snapshot(error = TRUE, {
    x <- paste(rep("a", 200), collapse = "")
    df <- data.frame(x = factor(x))
    write_sav(df, tempfile())
  })
})

test_that("complain about invalid variable names", {
  expect_snapshot(error = TRUE, {
    df <- data.frame(a = 1, A = 1, b = 1)
    write_sav(df, tempfile())

    names(df) <- c("$var", "A._$@#1", "a.")
    write_sav(df, tempfile())

    names(df) <- c("ALL", "eq", "b")
    write_sav(df, tempfile())

    names(df) <- c(
      paste(rep("a", 65), collapse = ""),
      paste(rep("b", 65), collapse = ""),
      "c"
    )
    write_sav(df, tempfile())
  })

  # Windows fails if this is a snapshot because of issues with unicode support
  expect_error(
    {
      df <- data.frame(a = 1, A = 1)
      names(df) <- c(
        paste(rep("\U044D", 33), collapse = ""),
        paste(rep("\U767E", 22), collapse = "")
      )
      write_sav(df, tempfile())
    },
    regexp = "Variables in `data` must have valid SPSS variable names"
  )
})

# max_level_lengths -------------------------------------------------------

test_that("works with NA levels", {
  x <- factor(c("a", "abc", NA), exclude = NULL)
  expect_equal(max_level_length(x), 3)
})

test_that("works with empty factors", {
  x <- factor(character(), levels = character())
  expect_equal(max_level_length(x), 0)

  x <- factor(character(), levels = c(NA_character_))
  expect_equal(max_level_length(x), 0)
})

# compression roundtrips --------------------------------------------------

test_that("all compression types roundtrip successfully", {
  df <- tibble::tibble(x = 1:10)
  expect_equal(roundtrip_sav(df, compress = "byte"), df)
  expect_equal(roundtrip_sav(df, compress = "none"), df)
  expect_equal(roundtrip_sav(df, compress = "zsav"), df)
})
