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

# read_dta checks its inputs

    Code
      read_dta(path, encoding = 1)
    Condition
      Error in `read_dta()`:
      ! `encoding` must be a single string or `NULL`, not the number 1.
    Code
      read_dta(path, skip = -1)
    Condition
      Error in `read_dta()`:
      ! `skip` must be a whole number larger than or equal to 0, not the number -1.

# write_dta checks its inputs

    Code
      write_dta(1, path)
    Condition
      Error in `write_dta()`:
      ! `data` must be a data frame, not the number 1.
    Code
      write_dta(mtcars, path, version = 14.5)
    Condition
      Error in `write_dta()`:
      ! `version` must be a whole number, not the number 14.5.
    Code
      write_dta(mtcars, path, label = 1)
    Condition
      Error in `write_dta()`:
      ! `label` must be a single string, not the number 1.
    Code
      write_dta(mtcars, path, strl_threshold = -1)
    Condition
      Error in `write_dta()`:
      ! `strl_threshold` must be a whole number between 0 and 2045, not the number -1.
    Code
      write_dta(mtcars, path, strl_threshold = 2046)
    Condition
      Error in `write_dta()`:
      ! `strl_threshold` must be a whole number between 0 and 2045, not the number 2046.
    Code
      write_dta(mtcars, path, adjust_tz = "yes")
    Condition
      Error in `write_dta()`:
      ! `adjust_tz` must be `TRUE` or `FALSE`, not the string "yes".

