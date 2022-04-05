# invalid files generate informative errors

    Code
      write_xpt(mtcars, file.path(tempdir(), " temp.xpt"))
    Condition
      Error:
      ! Failed to create file: A provided name contains an illegal character.

