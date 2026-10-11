# string variables can't have missing value ranges

    Code
      write_sav(df, tempfile())
    Condition
      Error:
      ! Failed to create column `x`: The file format does not support missing value ranges for this variable type.

# complain about long factor labels

    Code
      x <- paste(rep("a", 200), collapse = "")
      df <- data.frame(x = factor(x))
      write_sav(df, tempfile())
    Condition
      Error in `write_sav()`:
      ! SPSS only supports levels with <= 120 characters.
      x Problems: `x`

# complain about invalid variable names

    Code
      df <- data.frame(a = 1, A = 1, b = 1)
      write_sav(df, tempfile())
    Condition
      Error in `write_sav()`:
      ! SPSS does not allow duplicate variable names.
      i Variable names are case-insensitive in SPSS.
      x Problems: `a` and `A`
    Code
      names(df) <- c("$var", "A._$@#1", "a.")
      write_sav(df, tempfile())
    Condition
      Error in `write_sav()`:
      ! Variables in `data` must have valid SPSS variable names.
      x Problems: `$var` and `a.`
    Code
      names(df) <- c("ALL", "eq", "b")
      write_sav(df, tempfile())
    Condition
      Error in `write_sav()`:
      ! Variables in `data` must have valid SPSS variable names.
      x Problems: `ALL` and `eq`
    Code
      names(df) <- c(paste(rep("a", 65), collapse = ""), paste(rep("b", 65),
      collapse = ""), "c")
      write_sav(df, tempfile())
    Condition
      Error in `write_sav()`:
      ! Variables in `data` must have valid SPSS variable names.
      x Problems: `aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa` and `bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb`

# read_sav checks its inputs

    Code
      read_sav(path, encoding = 1)
    Condition
      Error in `read_sav()`:
      ! `encoding` must be a single string or `NULL`, not the number 1.
    Code
      read_sav(path, user_na = "yes")
    Condition
      Error in `read_sav()`:
      ! `user_na` must be `TRUE` or `FALSE`, not the string "yes".
    Code
      read_sav(path, skip = -1)
    Condition
      Error in `read_sav()`:
      ! `skip` must be a whole number larger than or equal to 0, not the number -1.

# write_sav checks its inputs

    Code
      write_sav(1, path)
    Condition
      Error in `write_sav()`:
      ! `data` must be a data frame, not the number 1.
    Code
      write_sav(mtcars, path, adjust_tz = "yes")
    Condition
      Error in `write_sav()`:
      ! `adjust_tz` must be `TRUE` or `FALSE`, not the string "yes".

