# haven 0.2.0.9000

* Import tibble so that tibbles are printed consistently (#154, @krlmlr).

* Update to latest ReadStat (#65). Includes: 

    * Support for binary (aka Ross) compression for SAS (#31)
    * Support extended ASCII encoding for Stata (#71)
    * Support for Stata 14 files (#75)
    * Support for SPSS value labels with more than 8 characters (#157)
    * Fix crash when writing `.sav` (#171).

* Added support for reading and writing variable formats. Similarly to 
  to variable labels, formats are stored as an attribute on the vector.
  Use `zap_formats()` if you want to remove these attributes.
  (@gorcha, #119, #123).
  
* Added support for "tagged" missing values (in Stata these are called 
  "extended" and in SAS these are called "special") which carry an extra
  byte of information: a character label from "a" to "z". The downside of
  this change is that all integer columns are now converted to doubles, 
  to support the encoding of the tag in the payload of a NaN.
  
* New `labelled_spss()` is a subclass of `labelled()` that can model
  user missing values from SPSS. These can either be a set of distinct
  values, or for numeric vectors, a range. `zap_labels()` strips labels,
  and replaces user-defined missing values with `NA`.

* `as_factor()` gains a new `levels = "default"` mechanism. This uses the
  labels where present, and otherwise uses the labels. This is now the
  default, as it seems to map better to the semantics of labelled values
  in other statistical packages (#81). You can also use `levels = "both"`
  to combine the value and the label into a single string (#82). It also
  gains a method for data frames, so you can easily convert every labelled
  column to a factor in one function call.

* New `vignette("semantics", package = "haven")` discusses the semantics
  of missing values and labelling in SAS, SPSS, and Stata, and how they
  are translated into R.

* Support for `hms()` has been moved into the hms package (#162).
  Time varibles now have class `c("hms", "difftime")` and a `units` attribute
  with value "secs" (#162).

* `labelled()` is less strict with its checks: you can mix double and integer
  value and labels (#86, #110, @lionel-), and `is.labelled()` is now exported 
  (#124).

* `read_dta()` now recognises "%d" and custom date types (#80, #130).
  It also gains an encoding parameter which you can use to override
  the default encoding. This is particularly useful for Stata 13 and below
  which did not store the encoding used in the file (#163).

* `read_por()` has been removed since it doesn't work and I'm unlikely to have 
  time to make it work in the near future (#35). It also doesn't seem to
  be commonly used in practice.

* `read_sav()` now correctly recognises EDATE and JDATE formats as dates (#72).
  Variables with format DATE, ADATE, EDATE, JDATE or SDATE are imported as 
  `Date` variables instead of `POSIXct`. 

* `read_dta()`, `read_sas()`, and `read_sav()` have a better test for missing
  string values (#79). They can all read from connections and compressed files
  (@lionel-, #109)

* Added `type_sum()` method for labelled objects so they print nicely in
  tibbles.

* `write_dta()` now verifies that variable names are valid Stata variables
  (#132), and throws an error if you attempt to save a labelled vector that
  is not an integer (#144).

* `write_sav()` writes hms variables to SPSS time variables, and the
  "measure" type is set for each variable (#133).

* `write_dta()` and `write_sav()` support writing date and date/times
  (#25, #139, #145). Labelled values are always converted to UTF-8 before 
  being written out (#87). Infinite values are now converted to missing values 
  since SPSS and Stata don't support them (#149). Both use a better test 
  for missing values (#70).
  
* `zap_labels()` has been completely overhauled. It now works 
  (@markriseley, #69), and only drops label attributes; it no longer replaces
  labelled values with `NA`s. It also gains a data frame method that zaps
  the labels from every column.

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
