# haven 0.2.0.9000

* `is.labelled()` is now exported (#124).

* Recognise stata "%d" format as a date (#130).

* Missing strings values are now read correctly into R (#79)

* `read_por()` now throws an informative error message (#35)

* SAS, SPSS and Stata all allow multiple types of missingness. Now, when 
  reading external data, haven preserves all missing labels. They will be 
  converted to `NA`, when you call `as_factor()`. If you want to preserve
  their original values, use `as_factor(drop_na = FALSE)`. This is supported
  by the new `is_na` argument to `labelled()` which allows you to flag 
  multiple labels as a "missing" (#33). 

* Writer functions use a better test for missing values (#70).

* `read_spss()` now correctly recognises EDATE and JDATE formats as dates (#72).
  `read_stata()` now correctly recognises custom date types (#80).

* Update to latest ReadStat (#65). Includes: 

    * Support for binary (aka Ross) compression for SAS (#31)
    * Support extended ASCII encoding for Stata (#71)
    * Support for Stata 14 files (#75)

* fixed bug in `zap_labels()` which was leaving labelled vectors unchanged
  instead of leaving unlabelled vectors unchanged. (@markriseley, #69).
  
* SPSS's variables whose format is DATE, ADATE, EDATE, JDATE or SDATE are
  now imported as `Date` variables instead of `POSIXct`.

# haven 0.2.0

* fixed a bug in `as_factor.labelled`, which generated <NA>'s and wrong 
  labels for integer labels.

* `zap_labels()` now leaves unlabelled vectors unchanged, making it easier
  to apply to all columns.

* `write_dta()` and `write_sav()` take more care to always write output as
  UTF-8 (#36)

* `write_dta()` and `write_sav()` won't crash if you give them invalid paths,
  and you can now use `~` to refer to your home directory (#37).

* Byte variables are now correctly read into integers (not strings, #45), 
  and missing values are captured correctly (#43).

* Added `read_stata()` as alias to `read_dta()` (#52).

* `read_spss()` uses extension to automatically choose between `read_sav()`
  and `read_por()` (#53)

* Updates from ReadStat. Including fixes for various parsing bugs, more 
  encodings, and better support for large files.

* hms objects deal better with missings when printing.

* Fixed bug causing labels for numeric variables to be read in as
  integers and associated error: ``Error: `x` and `labels` must be same type``

# haven 0.1.1

* Fixed memory initialisation problems found by valgrind.
