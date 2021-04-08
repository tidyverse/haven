# invalid file names generate errors

    Code
      write_xpt(mtcars, file.path(tempdir(), " temp.xpt"))
    Error <simpleError>
      Writing failure: A provided name contains an illegal character.

