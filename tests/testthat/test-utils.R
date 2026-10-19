test_that("datasource() returns paths and raw vectors unchanged", {
  path <- test_path("stata/notes.dta")
  expect_equal(datasource(path), normalizePath(path))

  raw <- as.raw(1:10)
  expect_equal(datasource(raw), raw)
})

test_that("datasource() reads connections", {
  path <- test_path("stata/notes.dta")
  bytes <- readBin(path, "raw", file.size(path))

  expect_equal(datasource(file(path)), bytes)
})

test_that("readers accept connections", {
  path <- test_path("stata/notes.dta")
  expected <- read_dta(path)

  expect_equal(read_dta(file(path)), expected)
})

test_that("datasource() gives useful errors", {
  expect_snapshot(error = TRUE, {
    datasource("doesnt-exist.dta")
    datasource(1)
  })
})

test_that("select_cols() returns positions and zero-based columns to skip", {
  expect_equal(
    select_cols(c("a", "b", "c"), c(c, a)),
    list(select = c(c = 3L, a = 1L), skip = 1L)
  )
  expect_equal(
    select_cols(c("a", "b"), c(x = b)),
    list(select = c(x = 2L), skip = 0L)
  )
})

test_that("select_cols() with NULL selects everything", {
  expect_equal(
    select_cols(c("a", "a"), NULL, "unique_quiet"),
    list(select = c(a...1 = 1L, a...2 = 2L), skip = integer())
  )
})

test_that("select_cols() repairs names before selecting", {
  expect_equal(
    select_cols(c("a", "a"), a...2, "unique_quiet")$select,
    c(a...2 = 2L)
  )
})

test_that("local_progress_bar() respects option and interactivity", {
  expect_null(local_progress_bar(FALSE))

  local_options(rlang_interactive = TRUE)
  expect_type(local_progress_bar(NULL), "character")

  local_options(haven.show_progress = FALSE)
  expect_null(local_progress_bar(NULL))

  local_options(haven.show_progress = TRUE, rlang_interactive = FALSE)
  expect_null(local_progress_bar(NULL))
})
