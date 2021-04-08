test_that("invalid files generate informative errors", {
  expect_snapshot(error = TRUE, {
    write_xpt(mtcars, file.path(tempdir(), " temp.xpt"))
    write_dta(df <- data.frame(.x = 1), tempfile())
  })
})
