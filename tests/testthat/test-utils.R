test_that("datasource() returns paths and raw vectors unchanged", {
  path <- test_path("stata/notes.dta")
  expect_equal(datasource(path), normalizePath(path))

  raw <- as.raw(1:10)
  expect_equal(datasource(raw), raw)
})

test_that("datasource() reads connections and compressed files", {
  path <- test_path("stata/notes.dta")
  bytes <- readBin(path, "raw", file.size(path))

  expect_equal(datasource(file(path)), bytes)

  gz <- tempfile(fileext = ".gz")
  con <- gzfile(gz, "wb")
  writeBin(bytes, con)
  close(con)
  expect_equal(datasource(gz), bytes)

  zip <- tempfile(fileext = ".zip")
  utils::zip(zip, path, flags = "-jq")
  expect_equal(datasource(zip), bytes)
})

test_that("readers accept connections and compressed files", {
  path <- test_path("stata/notes.dta")
  expected <- read_dta(path)

  expect_equal(read_dta(file(path)), expected)

  gz <- tempfile(fileext = ".dta.gz")
  con <- gzfile(gz, "wb")
  writeBin(readBin(path, "raw", file.size(path)), con)
  close(con)
  expect_equal(read_dta(gz), expected)
})

test_that("datasource() gives useful errors", {
  expect_snapshot(error = TRUE, {
    datasource("doesnt-exist.dta")
    datasource(1)
  })
})
