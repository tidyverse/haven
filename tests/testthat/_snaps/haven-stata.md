# invalid files generate informative errors

    Code
      long <- paste(rep("a", 100), collapse = "")
      write_dta(data.frame(x = 1), tempfile(), label = long)
    Condition
      Error in `write_dta()`:
      ! `label` must be 80 characters or fewer.
    Code
      df <- data.frame(1)
      names(df) <- "x y"
      write_dta(df, tempfile(), version = 13)
    Condition
      Error in `write_dta()`:
      ! Variables in `data` must have valid Stata variable names.
      x Problems: `x y`
    Code
      names(df) <- long
      write_dta(df, tempfile(), version = 13)
    Condition
      Error in `write_dta()`:
      ! Variables in `data` must have valid Stata variable names.
      x Problems: `aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa`
    Code
      write_dta(df, tempfile(), version = 14)
    Condition
      Error in `write_dta()`:
      ! Variables in `data` must have valid Stata variable names.
      x Problems: `aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa`

# can't write non-integer labels (#401)

    Code
      df <- data.frame(x = labelled(c(1, 2.5, 3), c(b = 1.5)))
      write_dta(df, tempfile())
    Condition
      Error in `write_dta()`:
      ! Stata only supports labelling with integer variables.
      x Problems: `x`

