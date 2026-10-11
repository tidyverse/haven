test_that("show_progress() respects option and interactivity", {
  local_options(rlang_interactive = TRUE)
  expect_equal(show_progress(), TRUE)

  local_options(haven.show_progress = FALSE)
  expect_equal(show_progress(), FALSE)

  local_options(haven.show_progress = TRUE, rlang_interactive = FALSE)
  expect_equal(show_progress(), FALSE)
})
