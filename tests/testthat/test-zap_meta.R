test_that("removes haven_meta and legacy attributes from vector", {
  x <- structure(
    1:3,
    format.sas = "F8.",
    format.spss = "F8.0",
    format.stata = "%9.0g",
    display_width = 10L,
    width = 8L
  )
  meta(x) <- meta_var_sas(format = "F8.")

  expect_null(attributes(zap_meta(x)))
})

test_that("preserves labels", {
  x <- labelled(1:3, c(a = 1), label = "lab")
  meta(x) <- meta_var_spss(measure = "nominal")
  expect_equal(zap_meta(x), labelled(1:3, c(a = 1), label = "lab"))
})

test_that("removes metadata from data frame and its columns", {
  x <- structure(1:3, format.sas = "F8.")
  meta(x) <- meta_var_sas(format = "F8.")
  df <- structure(
    tibble::tibble(x = x),
    notes = "note",
    creation_timestamp = Sys.time(),
    modified_timestamp = Sys.time()
  )
  meta(df) <- meta_df_sas(table_name = "DATA")

  expect_equal(zap_meta(df), tibble::tibble(x = 1:3))
})
