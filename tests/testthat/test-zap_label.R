context("zap_label")

test_that("zap_label strips label but doesn't change other attributes", {
  var <- labelled(c(1L, 98L, 99L), c(not_answered = 98L, not_applicable = 99L), label = "foo")
  var_nolabel <- labelled(c(1L, 98L, 99L), c(not_answered = 98L, not_applicable = 99L))
  expect_identical(zap_label(var), var_nolabel)
})

test_that("zap_label returns variables not of class('labelled') unmodified", {
  var <- c(1L, 98L, 99L)
  expect_equal(zap_labels(var), var)
})

test_that("zap_label is correctly applied to every column in data frame", {
  y1_label <- labelled(10:1, c("good" = 1), label="foo")
  y1_nolabel <- labelled(10:1, c("good" = 1))
  y2_label <- labelled(1:10, c("bad" = 2), label="bar")
  y2_nolabel <- labelled(1:10, c("bad" = 2))

  df <- tibble::tibble(x = 1:10, y1=y1_label, y2=y2_label)
  df_zapped <- zap_label(df)

  expect_equal(ncol(df_zapped), ncol(df))
  expect_identical(df_zapped$y1, y1_nolabel)
  expect_identical(df_zapped$y2, y2_nolabel)
})
