# datasource() gives useful errors

    Code
      datasource("doesnt-exist.dta")
    Condition
      Error:
      ! 'doesnt-exist.dta' does not exist.
    Code
      datasource(1)
    Condition
      Error:
      ! `file` must be a single string, not the number 1.

