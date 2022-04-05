test_that("labelled() with no args returns a zero-length vector", {
  expect_length(labelled(), 0)
})

test_that("x must be numeric or character", {
  expect_error(labelled(TRUE), "must be a numeric or a character vector")
})

test_that("x and labels must be compatible", {
  expect_incompatible_type(labelled(1, "a"))
  expect_error(labelled(1, c(female = 2L, male = 1L)), NA)
  expect_error(labelled(1L, c(female = 2, male = 1)), NA)
})

test_that("labels can be NULL", {
  x <- labelled(1:5, NULL)
  expect_equal(x[], x)
})

test_that("labels must have names", {
  expect_error(labelled(1, 1), "must have names")
})

test_that("label must be length 1 character or missing", {
  expect_error(labelled(1, c(female = 1)), NA)
  expect_error(labelled(1, c(female = 1), label = "foo"), NA)
  expect_error(
    labelled(1, c(female = 1), label = 1),
    "character vector of length one"
  )
  expect_error(
    labelled(1, c(female = 1), label = c("foo", "bar")),
    "character vector of length one"
  )
})

test_that("labels must be unique", {
  expect_error(labelled(1, c(female = 1, male = 1)), "must be unique")
})

test_that("have methods used by revdeps", {
  x <- labelled(1:3, labels = c(x = 1L, y = 2L))
  expect_equal(as.character(x), c("1", "2", "3"))
  expect_equal(levels(x), NULL)
})

test_that("arithmetic strips class", {
  int <- labelled(1L)
  dbl <- labelled(2)

  expect_error(int + "x", class = "vctrs_error_incompatible")

  expect_identical(int + dbl, 3)
  expect_identical(int + 1L, 2L)
  expect_identical(1L + int, 2L)
  expect_identical(int + 1, 2)
  expect_identical(1 + int, 2)

  expect_identical(sum(int), 1L)
})


# methods -----------------------------------------------------------------

test_that("implements methods that vctrs requires", {
  x_int <- labelled(1:3)
  expect_equal(median(x_int), 2L)
  expect_equal(quantile(x_int, 0.25), c("25%" = 1.5))
  expect_equal(summary(x_int), summary(1:3))

  x_chr <- labelled(letters[1:3])
  expect_error(median(x_chr), "labelled<character>")
  expect_error(quantile(x_chr), "labelled<character>")
  expect_equal(summary(x_chr), summary(letters[1:3]))
})


# types -------------------------------------------------------------------

test_that("combining is symmetrical w.r.t. data types", {
  expect_incompatible_type(vec_c(labelled(character()), labelled()))
  expect_incompatible_type(vec_c(labelled(), labelled(character())))

  expect_identical(
    vec_c(labelled(integer()), labelled()),
    vec_c(labelled(), labelled(integer()))
  )

  expect_identical(
    vec_c(labelled(), double()),
    vec_c(double(), labelled())
  )
  expect_identical(
    vec_c(labelled(), integer()),
    vec_c(integer(), labelled())
  )
})

test_that("can cast labelled to atomic vectors", {
  x_int <- labelled(1:2)
  x_dbl <- labelled(c(1, 2))
  x_chr <- labelled(c("a", "b"))

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

test_that("can cast atomic vectors to labelled", {
  x_int <- labelled(1:2)
  x_dbl <- labelled(c(1, 2))
  x_chr <- labelled(c("a", "b"))

  expect_identical(vec_cast(1:3, x_int), labelled(1:3))
  expect_identical(vec_cast(1:3, x_dbl), labelled(c(1, 2, 3)))
  expect_error(vec_cast(1:3, x_chr), class = "vctrs_error_incompatible_type")

  expect_identical(vec_cast(c(0, 1), x_int), labelled(0:1))
  expect_identical(vec_cast(c(0, 1), x_dbl), labelled(c(0, 1)))
  expect_error(vec_cast(c(0, 1), x_chr), class = "vctrs_error_incompatible_type")

  expect_error(vec_cast("a", x_int), class = "vctrs_error_incompatible_type")
  expect_error(vec_cast("a", x_dbl), class = "vctrs_error_incompatible_type")
  expect_identical(vec_cast("a", x_chr), labelled("a"))
})

test_that("combining preserves label sets", {
  expect_equal(
    vec_c(
      labelled(1, labels = c(Good = 1, Bad = 5)),
      labelled(5, labels = c(Good = 1, Bad = 5)),
    ),
    labelled(c(1, 5), labels = c(Good = 1, Bad = 5))
  )
})

test_that("can combine names", {
  x <- labelled(c(x = 1L))
  expect_named(vec_c(x, x), c("x", "x"))
  expect_named(vec_c(x, c(y = 1L)), c("x", "y"))
})

test_that("take labels from LHS", {
  expect_snapshot_warning({
    expect_equal(
      vec_c(
        labelled(1, labels = c(Good = 1, Bad = 5)),
        labelled(5, labels = c(Bad = 1, Good = 5)),
      ),
      labelled(c(1, 5), labels = c(Good = 1, Bad = 5))
    )
  })

  expect_snapshot_warning({
    expect_equal(
      vec_c(
        labelled(1, labels = c(Good = 1)),
        labelled(5, labels = c(Bad = 1)),
      ),
      labelled(c(1, 5), labels = c(Good = 1))
    )
  })
})

test_that("warn only for conflicting labels", {
  expect_snapshot_warning({
    x <- labelled(1:2, c(Yes = 1, No = 2))
    y <- labelled(1:2, c(Female = 1, Male = 2, Other = 3))
    c(x, y)
  })

  expect_snapshot_warning({
    x <- labelled(1:2, c(a = 1, b = 2, c = 3, d = 4, e = 5, f = 6, g = 7, h = 8, i = 9, j = 10, k = 11))
    y <- labelled(1:2, c(A = 1, B = 2, C = 3, D = 4, E = 5, F = 6, G = 7, H = 8, I = 9, J = 10, K = 11))
    c(x, y)
  })
})

test_that("combining picks label from the left", {
  expect_equal(
    attr(vec_c(
      labelled(label = "left"),
      labelled(label = "right"),
    ), "label", exact = TRUE),
    "left"
  )
})

test_that("combining with bare vectors results in a labelled()", {
  expect_identical(vec_c(labelled(), 1.1), labelled(1.1))
  expect_identical(vec_c(labelled(integer()), 1.1), labelled(1.1))

  expect_equal(
    vec_c(labelled(labels = c(Good = 1, Bad = 5)), 1, 3, 5),
    labelled(vec_c(1, 3, 5), labels = c(Good = 1, Bad = 5))
  )
})

test_that("casting to labelled throws lossy cast if not safe", {
  expect_incompatible_type(vec_cast("a", labelled()))
  expect_incompatible_type(vec_cast("a", labelled(integer())))
  expect_error(vec_cast(1.1, labelled(integer())), class = "vctrs_error_cast_lossy")
})

test_that("casting to a superset of labels works", {
  expect_equal(
    vec_cast(
      labelled(c(1, 5), c(Good = 1)),
      labelled(labels = c(Good = 1, Bad = 5))
    ),
    labelled(c(1, 5), labels = c(Good = 1, Bad = 5))
  )
})

test_that("casting to a subset of labels works iff labels were unused", {
  expect_equal(
    vec_cast(
      labelled(1, c(Good = 1, Bad = 5)),
      labelled(labels = c(Good = 1))
    ),
    labelled(1, labels = c(Good = 1))
  )
  expect_lossy_cast(vec_cast(
    labelled(c(1, 5), c(Good = 1, Bad = 5)),
    labelled(labels = c(Good = 1))
  ))
})

test_that("casting away labels throws lossy cast", {
  expect_lossy_cast(vec_cast(
    labelled(1, c(Good = 1)),
    labelled(labels = c(Bad = 5))
  ))
})

test_that("casting away tagged na values throws lossy cast", {
  expect_lossy_cast(vec_cast(
    labelled(tagged_na("a")),
    labelled(integer())
  ))
  expect_incompatible_type(vec_cast(
    labelled(tagged_na("a")),
    labelled(character())
  ))
})

test_that("won't cast labelled numeric to character", {
  expect_incompatible_type(vec_cast(labelled(), character()))
  expect_incompatible_type(vec_cast(labelled(integer()), character()))
})

# methods -----------------------------------------------------------------

test_that("printed output is stable", {
  x <- labelled(
    c(1:5, NA, tagged_na("x", "y", "z")),
    c(
      Good = 1,
      Bad = 5,
      "Not Applicable" = tagged_na("x"),
      "Refused to answer" = tagged_na("y")
    )
  )

  expect_snapshot(x)
})

test_that("given correct name in data frame", {
  x <- labelled(1:3, c(a = 1))

  expect_named(data.frame(x), "x")
  expect_named(data.frame(y = x), "y")
})

test_that("can convert to factor with using labels with labelled na's", {
  x <- labelled(c(1:2, tagged_na("a")), c(a = 1, c = tagged_na("a")))
  expect_equal(as_factor(x, "labels"), factor(c("a", NA, "c")))
})
