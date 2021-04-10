# invalid files generate informative errors

    Code
      long <- paste(rep("a", 100), collapse = "")
      write_dta(data.frame(x = 1), tempfile(), label = long)
    Error <simpleError>
      Stata data labels must be 80 characters or fewer
    Code
      df <- data.frame(1)
      names(df) <- "x y"
      write_dta(df, tempfile(), version = 13)
    Error <simpleError>
      The following variable names are not valid Stata variables: `x y`
    Code
      names(df) <- long
      write_dta(df, tempfile(), version = 13)
    Error <simpleError>
      The following variable names are not valid Stata variables: `aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa`
    Code
      write_dta(df, tempfile(), version = 14)
    Error <simpleError>
      The following variable names are not valid Stata variables: `aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa`

# can't write non-integer labels (#401)

    Code
      df <- data.frame(x = labelled(c(1, 2.5, 3), c(b = 1.5)))
      write_dta(df, tempfile())
    Error <simpleError>
      Stata only supports labelling with integers.
      Problems: `x`

