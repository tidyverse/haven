# invalid files generate informative errors

    Code
      write_xpt(mtcars, file.path(tempdir(), " temp.xpt"))
    Error <simpleError>
      Failed to create file: A provided name contains an illegal character.
    Code
      write_dta(df <- data.frame(.x = 1), tempfile())
    Error <simpleError>
      Failed to create column `.x`: A provided name contains an illegal character.

