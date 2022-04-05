test_that("constructor checks na_value", {
  expect_incompatible_type(labelled_spss(1:10, na_values = "a"))

  expect_snapshot(error = TRUE, {
    labelled_spss(1:10, na_values = "a")
    labelled_spss(1:10, na_values = NA_integer_)
  })
})

test_that("constructor checks na_range", {
  expect_snapshot(error = TRUE, {
    labelled_spss(1:10, na_range = "a")
    labelled_spss(1:10, na_range = 1:3)
    labelled_spss(1:10, na_range = c(2, NA))
    labelled_spss(1:10, na_range = c(2, 1))
  })
})

test_that("printed output is stable", {
  x <- labelled_spss(
    1:5, c("Good" = 1, "Bad" = 5),
    na_values = c(1, 2),
    na_range = c(3, Inf)
  )
  expect_snapshot(x)
})

test_that("subsetting preserves attributes", {
  x <- labelled_spss(
    1:5, c("Good" = 1, "Bad" = 5),
    na_values = c(1, 2),
    na_range = c(3, Inf),
    label = "Rating"
  )
  expect_identical(x, x[])
})

test_that("labels must be unique", {
  expect_error(
    labelled_spss(1, c(female = 1, male = 1), na_values = 9),
    "must be unique"
  )
})

# is.na -------------------------------------------------------------------

test_that("values in na_range flagged as missing", {
  x <- labelled_spss(1:5, c("a" = 1), na_range = c(1, 3))
  expect_equal(is.na(x), c(TRUE, TRUE, TRUE, FALSE, FALSE))
})

test_that("values in na_values flagged as missing", {
  x <- labelled_spss(1:5, c("a" = 1), na_values = c(1, 3, 5))
  expect_equal(is.na(x), c(TRUE, FALSE, TRUE, FALSE, TRUE))
})


# Types -------------------------------------------------------------------

test_that("combining preserves class", {
  expect_s3_class(vec_c(labelled_spss(), labelled_spss()), "haven_labelled_spss")
  expect_s3_class(vec_c(labelled_spss(), labelled_spss(na_values = 1)), "haven_labelled")
  expect_s3_class(vec_c(labelled_spss(na_values = 1), labelled_spss(na_values = 1)), "haven_labelled_spss")
})

test_that("combining is symmetrical w.r.t. data types", {
  expect_incompatible_type(vec_c(labelled_spss(character()), labelled_spss()))
  expect_incompatible_type(vec_c(labelled_spss(), labelled_spss(character())))

  expect_identical(
    vec_c(labelled_spss(integer()), labelled_spss()),
    vec_c(labelled_spss(), labelled_spss(integer()))
  )

  expect_identical(
    vec_c(labelled_spss(), double()),
    vec_c(double(), labelled_spss())
  )
  expect_identical(
    vec_c(labelled_spss(), integer()),
    vec_c(integer(), labelled_spss())
  )

  expect_identical(
    vec_c(labelled_spss(), labelled()),
    vec_c(labelled(), labelled_spss())
  )
})

test_that("can cast labelled_spss to atomic vectors", {
  x_int <- labelled_spss(1:2)
  x_dbl <- labelled_spss(c(1, 2))
  x_chr <- labelled_spss(c("a", "b"))

  expect_identical(vec_cast(x_int, integer()), 1:2)
  expect_identical(vec_cast(x_int, double()), c(1, 2))
  expect_error(vec_cast(x_int, character()), class = "vctrs_error_incompatible_type")

  expect_identical(vec_cast(x_dbl, integer()), 1:2)
  expect_identical(vec_cast(x_dbl, double()), c(1, 2))
  expect_error(vec_cast(x_dbl, character()), class = "vctrs_error_incompatible_type")

  expect_error(vec_cast(x_chr, integer()), class = "vctrs_error_incompatible_type")
  expect_error(vec_cast(x_chr, double()), class = "vctrs_error_incompatible_type")
  expect_identical(vec_cast(x_chr, character()), c("a", "b"))
})

test_that("can cast atomic vectors to labelled_spss", {
  x_int <- labelled_spss(1:2)
  x_dbl <- labelled_spss(c(1, 2))
  x_chr <- labelled_spss(c("a", "b"))

  expect_identical(vec_cast(1:3, x_int), labelled_spss(1:3))
  expect_identical(vec_cast(1:3, x_dbl), labelled_spss(c(1, 2, 3)))
  expect_error(vec_cast(1:3, x_chr), class = "vctrs_error_incompatible_type")

  expect_identical(vec_cast(c(0, 1), x_int), labelled_spss(0:1))
  expect_identical(vec_cast(c(0, 1), x_dbl), labelled_spss(c(0, 1)))
  expect_error(vec_cast(c(0, 1), x_chr), class = "vctrs_error_incompatible_type")

  expect_error(vec_cast("a", x_int), class = "vctrs_error_incompatible_type")
  expect_error(vec_cast("a", x_dbl), class = "vctrs_error_incompatible_type")
  expect_identical(vec_cast("a", x_chr), labelled_spss("a"))
})

test_that("combining preserves label sets", {
  expect_equal(
    vec_c(
      labelled_spss(1, labels = c(Good = 1, Bad = 5)),
      labelled_spss(5, labels = c(Good = 1, Bad = 5)),
    ),
    labelled_spss(c(1, 5), labels = c(Good = 1, Bad = 5))
  )
})

test_that("combining preserves user missing", {
  expect_equal(
    vec_c(
      labelled_spss(1, na_values = c(1, 5)),
      labelled_spss(5, na_values = c(1, 5)),
    ),
    labelled_spss(c(1, 5), na_values = c(1, 5))
  )

  expect_equal(
    vec_c(
      labelled_spss(1, na_range = c(1, 5)),
      labelled_spss(5, na_range = c(1, 5)),
    ),
    labelled_spss(c(1, 5), na_range = c(1, 5))
  )
})

test_that("can combine names", {
  x <- labelled_spss(c(x = 1L))
  expect_named(vec_c(x, x), c("x", "x"))
  expect_named(vec_c(x, c(y = 1L)), c("x", "y"))
})

test_that("take labels from LHS", {
  expect_snapshot_warning({
    expect_equal(
      vec_c(
        labelled_spss(1, labels = c(Good = 1, Bad = 5)),
        labelled_spss(5, labels = c(Bad = 1, Good = 5)),
      ),
      labelled_spss(c(1, 5), labels = c(Good = 1, Bad = 5))
    )
  })

  expect_snapshot_warning({
    expect_equal(
      vec_c(
        labelled_spss(1, labels = c(Good = 1)),
        labelled_spss(5, labels = c(Bad = 1)),
      ),
      labelled_spss(c(1, 5), labels = c(Good = 1))
    )
  })
})

test_that("warn only for conflicting labels", {
  expect_snapshot_warning({
    x <- labelled_spss(1:2, c(Yes = 1, No = 2))
    y <- labelled_spss(1:2, c(Female = 1, Male = 2, Other = 3))
    c(x, y)
  })

  expect_snapshot_warning({
    x <- labelled_spss(1:2, c(a = 1, b = 2, c = 3, d = 4, e = 5, f = 6, g = 7, h = 8, i = 9, j = 10, k = 11))
    y <- labelled_spss(1:2, c(A = 1, B = 2, C = 3, D = 4, E = 5, F = 6, G = 7, H = 8, I = 9, J = 10, K = 11))
    c(x, y)
  })
})

test_that("strip user missing if different", {
  expect_equal(
    vec_c(
      labelled_spss(na_values = 1),
      labelled_spss(na_values = 5),
    ),
    labelled()
  )

  expect_equal(
    vec_c(
      labelled_spss(na_range = c(1, 5)),
      labelled_spss(na_range = c(2, 4)),
    ),
    labelled()
  )

  expect_equal(
    vec_c(
      labelled_spss(na_range = c(1, 5)),
      labelled_spss(na_values = 5),
    ),
    labelled()
  )
})

test_that("combining picks label from the left", {
  expect_equal(
    attr(vec_c(
      labelled_spss(label = "left"),
      labelled_spss(label = "right"),
    ), "label", exact = TRUE),
    "left"
  )
})

test_that("combining with bare vectors results in a labelled_spss()", {
  expect_identical(vec_c(labelled_spss(), 1.1), labelled_spss(1.1))
  expect_identical(vec_c(labelled_spss(integer()), 1.1), labelled_spss(1.1))

  expect_equal(
    vec_c(labelled_spss(labels = c(Good = 1, Bad = 5)), 1, 3, 5),
    labelled_spss(vec_c(1, 3, 5), labels = c(Good = 1, Bad = 5))
  )
})

test_that("casting to labelled_spss throws lossy cast if not safe", {
  expect_incompatible_type(vec_cast("a", labelled_spss()))
  expect_incompatible_type(vec_cast("a", labelled_spss(integer())))
  expect_error(vec_cast(1.1, labelled_spss(integer())), class = "vctrs_error_cast_lossy")
})

test_that("casting to a superset of labels works", {
  expect_equal(
    vec_cast(
      labelled_spss(c(1, 5), c(Good = 1)),
      labelled_spss(labels = c(Good = 1, Bad = 5))
    ),
    labelled_spss(c(1, 5), labels = c(Good = 1, Bad = 5))
  )
})

test_that("casting to a subset of labels works iff labels were unused", {
  expect_equal(
    vec_cast(
      labelled_spss(1, c(Good = 1, Bad = 5)),
      labelled_spss(labels = c(Good = 1))
    ),
    labelled_spss(1, labels = c(Good = 1))
  )
  expect_lossy_cast(vec_cast(
    labelled_spss(c(1, 5), c(Good = 1, Bad = 5)),
    labelled_spss(labels = c(Good = 1))
  ))
})

test_that("casting away labels throws lossy cast", {
  expect_lossy_cast(vec_cast(
    labelled_spss(1, c(Good = 1)),
    labelled_spss(labels = c(Bad = 5))
  ))
})


test_that("casting to a superset of user missing works", {
  expect_equal(
    vec_cast(
      labelled_spss(c(1, 5), na_values = 1),
      labelled_spss(na_values = c(1, 5))
    ),
    labelled_spss(c(1, 5), na_values = c(1, 5))
  )

  expect_equal(
    vec_cast(
      labelled_spss(c(1, 5), na_values = 1),
      labelled_spss(na_range = c(1, 5))
    ),
    labelled_spss(c(1, 5), na_range = c(1, 5))
  )

  expect_equal(
    vec_cast(
      labelled_spss(c(1, 5), na_range = c(2, 4)),
      labelled_spss(na_range = c(1, 5))
    ),
    labelled_spss(c(1, 5), na_range = c(1, 5))
  )
})

test_that("casting to a subset of user missing works iff values were unused", {
  expect_equal(
    vec_cast(
      labelled_spss(1, na_values = c(1, 5)),
      labelled_spss(na_values = 1)
    ),
    labelled_spss(1, na_values = 1)
  )
  expect_lossy_cast(vec_cast(
    labelled_spss(c(1, 5), na_values = c(1, 5)),
    labelled_spss(na_values = 1)
  ))

  expect_equal(
    vec_cast(
      labelled_spss(1, na_range = c(1, 5)),
      labelled_spss(na_range = c(1, 3))
    ),
    labelled_spss(1, na_range = c(1, 3))
  )
  expect_lossy_cast(vec_cast(
    labelled_spss(c(1, 5), na_range = c(1, 5)),
    labelled_spss(na_range = c(1, 3))
  ))

  expect_equal(
    vec_cast(
      labelled_spss(1, na_range = c(1, 5)),
      labelled_spss(na_values = 1)
    ),
    labelled_spss(1, na_values = 1)
  )
  expect_lossy_cast(vec_cast(
    labelled_spss(c(1, 5), na_range = c(1, 5)),
    labelled_spss(na_values = 1)
  ))

  expect_equal(
    vec_cast(
      labelled_spss(1, na_values = c(1, 5)),
      labelled_spss(na_range = c(1, 3))
    ),
    labelled_spss(1, na_range = c(1, 3))
  )
  expect_lossy_cast(vec_cast(
    labelled_spss(c(1, 5), na_values = c(1, 5)),
    labelled_spss(na_range = c(1, 3))
  ))
})

test_that("casting away user missing throws lossy cast", {
  expect_lossy_cast(vec_cast(
    labelled_spss(1, na_values = 1),
    labelled_spss(na_values = 5)
  ))

  expect_lossy_cast(vec_cast(
    labelled_spss(1, na_range = c(1, 3)),
    labelled_spss(na_range = c(5, 7))
  ))

  expect_lossy_cast(vec_cast(
    labelled_spss(1, na_range = c(1, 3)),
    labelled_spss(na_values = 5)
  ))

  expect_lossy_cast(vec_cast(
    labelled_spss(1, na_values = 1),
    labelled_spss(na_range = c(5, 7))
  ))
})

test_that("casting to regular labelled ignores missing values", {
  expect_equal(
    vec_cast(
      labelled_spss(1, na_values = c(1, 5)),
      labelled()
    ),
    labelled(1)
  )
})

test_that("casting away tagged na values throws lossy cast", {
  expect_lossy_cast(vec_cast(
    labelled_spss(tagged_na("a")),
    labelled_spss(integer())
  ))
  expect_incompatible_type(vec_cast(
    labelled_spss(tagged_na("a")),
    labelled_spss(character())
  ))
})

test_that("won't cast labelled_spss numeric to character", {
  expect_incompatible_type(vec_cast(labelled_spss(), character()))
  expect_incompatible_type(vec_cast(labelled_spss(integer()), character()))
})
