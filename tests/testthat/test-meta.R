test_that("properties default to NULL", {
  x <- meta_var_spss()
  expect_null(x@format)
  expect_null(x@display_width)
  expect_null(x@measure)

  expect_null(meta_df_stata()@created)
})

test_that("abstract classes can't be constructed", {
  expect_error(meta_var(), "abstract")
  expect_error(meta_df(), "abstract")
})

test_that("integer properties accept whole doubles", {
  expect_identical(meta_var_sas(width = 8)@width, 8L)
  expect_identical(meta_df_sas(xpt_version = 5)@xpt_version, 5L)
})

test_that("properties are validated", {
  expect_snapshot(error = TRUE, {
    meta_var_sas(format = c("a", "b"))
    meta_var_sas(width = 0)
    meta_var_sas(width = 1.5)
    meta_var_spss(measure = "interval")
    meta_var_spss(alignment = "middle")
    meta_var_stata(type = "str10")
    meta_df_sas(xpt_version = 6)
    meta_df_stata(version = 16)
    meta_df_spss(notes = NA_character_)
    meta_df_spss(created = Sys.time() + 1:2)
  })
})

test_that("meta() returns NULL when there's no metadata", {
  expect_null(meta(1:3))
  expect_null(meta(data.frame(x = 1)))
})

test_that("meta() with vendor returns empty object when absent", {
  expect_equal(meta(1:3, "sas"), meta_var_sas())
  expect_equal(meta(data.frame(x = 1), "stata"), meta_df_stata())
})

test_that("meta<- sets and removes metadata", {
  x <- 1:3
  meta(x) <- meta_var_sas(format = "DATE9.")
  expect_equal(meta(x), meta_var_sas(format = "DATE9."))

  meta(x) <- NULL
  expect_null(attr(x, "haven_meta"))
})

test_that("can modify a single property", {
  x <- 1:3
  meta(x, "sas")@informat <- "DATE9."
  expect_equal(meta(x), meta_var_sas(informat = "DATE9."))

  df <- data.frame(x = 1)
  meta(df, "spss")@notes <- "a note"
  expect_equal(meta(df), meta_df_spss(notes = "a note"))
})

test_that("meta() and meta<- check vendor and type", {
  x <- 1:3
  meta(x) <- meta_var_sas()
  df <- data.frame(x = 1)

  expect_snapshot(error = TRUE, {
    meta(x, "spss")
    meta(x, "spps")
    meta(x, "stata") <- meta_var_sas()
    meta(x) <- meta_df_sas()
    meta(df) <- meta_var_sas()
    meta(x) <- list()
  })
})

test_that("legacy column attributes are used as fallback", {
  x <- structure(1, format.sas = "DATE9.", width = 8)
  expect_equal(meta(x), meta_var_sas(format = "DATE9.", width = 8))

  x <- structure(1, format.spss = "F8.2", display_width = 10)
  expect_equal(meta(x), meta_var_spss(format = "F8.2", display_width = 10))

  x <- structure(1, display_width = 10)
  expect_equal(meta(x), meta_var_spss(display_width = 10))

  x <- structure(1, format.stata = "%9.0g")
  expect_equal(meta(x), meta_var_stata(format = "%9.0g"))
})

test_that("legacy width needs vendor from argument", {
  x <- structure(1, width = 8)
  expect_null(meta(x))
  expect_equal(meta(x, "sas"), meta_var_sas(width = 8))
})

test_that("legacy data frame attributes use vendor from columns", {
  time <- as.POSIXct("2020-01-01", tz = "UTC")
  df <- structure(
    data.frame(x = structure(1, format.stata = "%9.0g")),
    notes = "a note",
    creation_timestamp = time
  )
  expect_equal(meta(df), meta_df_stata(created = time, notes = "a note"))

  df <- data.frame(x = structure(1, format.sas = "DATE9."))
  expect_equal(meta(df), meta_df_sas())

  df <- structure(data.frame(x = 1), creation_timestamp = time)
  expect_null(meta(df))
  expect_equal(meta(df, "sas"), meta_df_sas(created = time))
})

test_that("legacy vendor mismatch is an error", {
  x <- structure(1, format.sas = "DATE9.")
  expect_snapshot(meta(x, "stata"), error = TRUE)
})

test_that("haven_meta takes precedence over legacy attributes", {
  x <- structure(1, format.sas = "DATE9.")
  meta(x) <- meta_var_sas(format = "DATE10.")
  expect_equal(meta(x)@format, "DATE10.")
})
