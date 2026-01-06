# invalid files generate informative errors

    Code
      write_xpt(mtcars, file.path(tempdir(), " temp.xpt"))
    Condition
      Error:
      ! Failed to create file: A provided name contains an illegal character.

# write_xpt validates variable name length

    Code
      write_xpt(df_bad_v5, path, version = 5, name = "test")
    Condition
      Error in `write_xpt()`:
      ! Variable names must be 8 bytes or fewer for SAS transport format version 5.
      x Problems: `X12345678`

---

    Code
      write_xpt(df_multi_v5, path, version = 5, name = "test")
    Condition
      Error in `write_xpt()`:
      ! Variable names must be 8 bytes or fewer for SAS transport format version 5.
      x Problems: `X1234567_ABC` and `X1234567_XYZ`

---

    Code
      write_xpt(df_bad_v8, path, version = 8, name = "test")
    Condition
      Error in `write_xpt()`:
      ! Variable names must be 32 bytes or fewer for SAS transport format version 8.
      x Problems: `X12345678901234567890123456789012`

# user width warns appropriately when data is wider than value

    Code
      write_xpt(df, path)
    Condition
      Warning:
      Column `b` contains string values longer than user width 1. Width set to 2 to accommodate.

