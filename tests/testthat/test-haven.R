test_that("invalid file names generate errors", {
  expect_snapshot(error = TRUE, {
    write_xpt(mtcars, file.path(tempdir(), " temp.xpt"))
  })
})
