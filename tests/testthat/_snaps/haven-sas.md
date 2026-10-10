# throws informative error on bad row limit

    Code
      rows_with_limit(1:5)
    Condition
      Error in `read_sas()`:
      ! `n_max` must be a number or `NA`, not an integer vector.
    Code
      rows_with_limit("foo")
    Condition
      Error in `read_sas()`:
      ! `n_max` must be a number or `NA`, not the string "foo".

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

# read_sas checks its inputs

    Code
      read_sas(path, encoding = 1)
    Condition
      Error in `read_sas()`:
      ! `encoding` must be a single string or `NULL`, not the number 1.
    Code
      read_sas(path, skip = -1)
    Condition
      Error in `read_sas()`:
      ! `skip` must be a whole number larger than or equal to 0, not the number -1.

# write_xpt checks its inputs

    Code
      write_xpt(mtcars, path, version = 8.5)
    Condition
      Error in `write_xpt()`:
      ! `version` must be a whole number, not the number 8.5.
    Code
      write_xpt(mtcars, path, name = 1)
    Condition
      Error in `write_xpt()`:
      ! `name` must be a single string or `NULL`, not the number 1.
    Code
      write_xpt(mtcars, path, label = 1)
    Condition
      Error in `write_xpt()`:
      ! `label` must be a single string or `NULL`, not the number 1.
    Code
      write_xpt(mtcars, path, adjust_tz = "yes")
    Condition
      Error in `write_xpt()`:
      ! `adjust_tz` must be `TRUE` or `FALSE`, not the string "yes".
    Code
      write_xpt(1, path)
    Condition
      Error in `write_xpt()`:
      ! `data` must be a data frame, not the number 1.

