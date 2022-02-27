# complain about long factor labels

    Code
      x <- paste(rep("a", 200), collapse = "")
      df <- data.frame(x = factor(x))
      write_sav(df, tempfile())
    Error <simpleError>
      SPSS only supports levels with <= 120 characters
      Problems: `x`

# complain about invalid variable names

    Code
      df <- data.frame(a = 1, A = 1)
      write_sav(df, tempfile())
    Error <simpleError>
      SPSS does not allow duplicate variable names. Note that variable names are case-insensitive in SPSS.
      Problems: `a`, `A`
    Code
      names(df) <- c(paste(rep("a", 65), collapse = ""), paste(rep("百", 22),
      collapse = ""))
      write_sav(df, tempfile())
    Error <simpleError>
      The following variable names are not valid SPSS variables: `aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa`, `百百百百百百百百百百百百百百百百百百百百百百`

