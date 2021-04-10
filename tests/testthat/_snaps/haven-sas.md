# invalid files generate informative errors

    Code
      write_xpt(mtcars, file.path(tempdir(), " temp.xpt"))
    Error <simpleError>
      Failed to create file: A provided name contains an illegal character.

