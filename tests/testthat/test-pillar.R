context("pillar printing")

test_that("pillar", {
  expect_known_output(
    print(tibble::tibble(x = labelled(
      1:11,
      c(Good = 1, Bad = 8)
    ))),
    file = "pillar_integer.txt"
  )

  expect_known_output(
    print(tibble::tibble(x = labelled(
      c(rep(c(1.22352, 1000, -345), each = 3), 35, 35),
      c(One = 1.22352, Two =  35, Threeeee = 1000)
    ))),
    file = "pillar_double.txt"
  )

  expect_known_output(
    print(tibble::tibble(x = labelled(
      c(rep("A", 3), rep("B", 3), rep("XXXXXX", 4), NA),
      c(Apple = "A", Banana = "B", Mystery = "XXXXXX")
    ))),
    file = "pillar_string.txt"
  )

  expect_known_output(
    print(tibble::tibble(x = labelled(
      c(1:8, tagged_na("a"), tagged_na("b"), NA),
      c(Good = 1, "Bad" = 8, Refused = tagged_na("b"))
    ))),
    file = "pillar_tagged_na.txt"
  )

  expect_known_output(
    print(tibble::tibble(x = labelled_spss(
      c(1:10, NA),
      c(Good = 1, Bad = 8, Refused = 10),
      c(9, 10)
    ))),
    file = "pillar_spss_integer.txt"
  )
})
