test_that("read functions attach file timestamps as POSIXct", {
  df <- tibble::tibble(x = 1:3)
  before <- Sys.time()

  for (ext in c("dta", "sav", "xpt")) {
    path <- tempfile(fileext = paste0(".", ext))
    switch(
      ext,
      dta = write_dta(df, path),
      sav = write_sav(df, path),
      xpt = write_xpt(df, path)
    )
    out <- switch(
      ext,
      dta = read_dta(path),
      sav = read_sav(path),
      xpt = read_xpt(path)
    )

    created <- attr(out, "creation_timestamp")
    modified <- attr(out, "modified_timestamp")
    expect_s3_class(created, "POSIXct")
    expect_s3_class(modified, "POSIXct")
    # dta files only store timestamps with minute resolution
    expect_true(created >= before - 60 && created <= Sys.time() + 60)
    expect_true(modified >= before - 60 && modified <= Sys.time() + 60)
  }
})

test_that("read_sas attaches timestamps and encoding", {
  out <- read_sas(test_path("sas/hadley.sas7bdat"))

  expect_equal(
    attr(out, "creation_timestamp"),
    as.POSIXct("2015-02-09 20:55:12", tz = "UTC")
  )
  expect_equal(
    attr(out, "modified_timestamp"),
    as.POSIXct("2015-02-09 20:55:12", tz = "UTC")
  )
  expect_equal(attr(out, "encoding"), "WINDOWS-1252")
})

test_that("read_sav attaches file encoding", {
  df <- tibble::tibble(x = 1:3)

  path <- tempfile(fileext = ".sav")
  write_sav(df, path)
  expect_equal(attr(read_sav(path), "encoding"), "UTF-8")
})
