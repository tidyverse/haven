# invalid files generate informative errors

    Code
      write_xpt(mtcars, file.path(tempdir(), " temp.xpt"))
    Condition
      Error:
      ! Failed to create file: A provided name contains an illegal character.

# user width warns appropriately when data is wider than value

    Code
      write_xpt(df, path)
    Condition
      Warning:
      Column `b` contains string values longer than user width 1. Width set to 2 to accommodate.

