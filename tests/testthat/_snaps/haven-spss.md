# complain about long factor labels

    Code
      x <- paste(rep("a", 200), collapse = "")
      df <- data.frame(x = factor(x))
      write_sav(df, tempfile())
    Error <simpleError>
      SPSS only supports levels with <= 120 characters
      Problems: `x`

