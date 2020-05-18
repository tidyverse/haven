test_that("nice display in tibbles", {
  verify_output(test_path("test-labelled-pillar.txt"), {
    int <- labelled(1:5, labels = c(good = 1L, bad = 5L))
    dbl <- labelled(1:5 / 10, labels = c(good = 0.1, bad = 0.5))
    chr <- labelled(letters[1:5], labels = c(good = "a", bad = "e"))

    tibble(int, dbl, chr)
  })
})
