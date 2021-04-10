test_that("invalid files generate informative errors", {
  expect_snapshot(error = TRUE, {
    write_xpt(mtcars, file.path(tempdir(), " temp.xpt"))
    write_dta(df <- data.frame(.x = 1), tempfile())
  })
})

test_that("connections are read", {
  file_conn <- file(test_path("hadley.sas7bdat"))
  expect_identical(read_sas(file_conn), read_sas("hadley.sas7bdat"))
})

test_that("zip files are read", {
  expect_identical(read_sas("hadley.zip"), read_sas("hadley.sas7bdat"))
})
