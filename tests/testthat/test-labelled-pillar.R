test_that("nice display in tibbles", {
  expect_snapshot({
    int <- labelled(1:5, labels = c(good = 1L, bad = 5L))
    dbl <- labelled(1:5 / 10, labels = c(good = 0.1, bad = 0.5))
    chr <- labelled(letters[1:5], labels = c(good = "a", bad = "e"))

    tibble(int, dbl, chr)
  })
})

test_that("pillar", {
  expect_snapshot({
    x <- labelled(1:11, c(Good = 1, Bad = 8))
    tibble::tibble(x)
  })

  expect_snapshot({
    x <- labelled(
      c(rep(c(1.22352, 1000, -345), each = 3), 35, 35),
      c(One = 1.22352, Two = 35, Threeeee = 1000)
    )
    tibble::tibble(x)
  })

  expect_snapshot({
    x <- labelled(
      c(rep("A", 3), rep("B", 3), rep("XXXXXX", 4), NA),
      c(Apple = "A", Banana = "B", Mystery = "XXXXXX")
    )
    tibble::tibble(x)
  })

  expect_snapshot({
    x <- labelled(
      c(1:8, tagged_na("a"), tagged_na("b"), NA),
      c(Good = 1, "Bad" = 8, Refused = tagged_na("b"))
    )
    tibble::tibble(x)
  })

  expect_snapshot({
    x <- labelled_spss(
      c(1:10, NA),
      c(Good = 1, Bad = 8, Refused = 10),
      c(9, 10)
    )
    tibble::tibble(x)
  })
})
