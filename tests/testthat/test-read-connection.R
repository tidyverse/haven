context("connections")

test_that("connections are read", {
  file_conn <- file("hadley.sas7bdat")
  expect_identical(read_sas(file_conn), read_sas("hadley.sas7bdat"))
})

test_that("zip files are read", {
  expect_identical(read_sas("hadley.zip"), read_sas("hadley.sas7bdat"))
})
