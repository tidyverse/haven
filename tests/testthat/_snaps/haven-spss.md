# complain about long factor labels

    Code
      x <- paste(rep("a", 200), collapse = "")
      df <- data.frame(x = factor(x))
      write_sav(df, tempfile())
    Condition
      Error:
      ! SPSS only supports levels with <= 120 characters
      Problems: `x`

# complain about invalid variable names

    Code
      df <- data.frame(a = 1, A = 1, b = 1)
      write_sav(df, tempfile())
    Condition
      Error in `validate_sav()`:
      ! SPSS does not allow duplicate variable names.
      i Variable names are case-insensitive in SPSS.
      x Problems: `a`, `A`
    Code
      names(df) <- c("$var", "A._$@#1", "a.")
      write_sav(df, tempfile())
    Condition
      Error in `validate_sav()`:
      ! Variables in `data` must have valid SPSS variable names.
      x Problems: `$var`, `a.`
    Code
      names(df) <- c("ALL", "eq", "b")
      write_sav(df, tempfile())
    Condition
      Error in `validate_sav()`:
      ! Variables in `data` must have valid SPSS variable names.
      x Problems: `ALL`, `eq`
    Code
      names(df) <- c(paste(rep("a", 65), collapse = ""), paste(rep("b", 65),
      collapse = ""), "c")
      write_sav(df, tempfile())
    Condition
      Error in `validate_sav()`:
      ! Variables in `data` must have valid SPSS variable names.
      x Problems: `aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa`, `bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb`
