# haven 1.1.1

* Update to latest readstat. Includes:

  * SPSS: empty charater columns now read as character (#311)
  * SPSS: now write long strings (#266)
  * Stata: reorder labelled vectors on write (#327)
  * State: `encoding` now affects value labels (#325)
  * SAS: can now write wide/long rows (#272, #335).
  * SAS: can now handle Windows Vietnamese character set (#336)

* `read_por()` and `read_xpt()` now correctly preserve attributes if
  output needs to be reallocated (which is typical behaviour) (#313)

* `read_sas()` recognises date/times format with trailing separator and width
  specifications (#324)
  
* `read_sas()` gains a `catalog_encoding` argument so you can independently
  specify encoding of data and catalog (#312)

* `write_*()` correctly measures lengths of non-ASCII labels (#258): this
  fixes the cryptic error "A provided string value was longer than the 
  available storage size of the specified column."

* `write_dta()` now checks for bad labels in all columns, not just the first
  (#326).

* `write_sav()` no longer fails on empty factors or factors with an `NA`
  level (#301) and writes out more metadata for `labelled_spss` vectors 
  (#334).
 
# haven 1.1.0

* Update to latest readstat. Includes:

  * SAS: support Win baltic code page (#231)
  * SAS: better error messages instead of crashes (#234, #270)
  * SAS: fix "unable to read error" (#271)
  * SPSS: support uppercase time stamps (#230)
  * SPSS: fixes for 252-255 byte strings (#226)
  * SPSS: fixes for 0 byte strings (#245)

* Share `as_factor()` with forcats package (#256)

* `read_sav()` once again correctly returns system defined missings 
  as `NA` (rather than `NaN`) (#223). `read_sav()` and `write_sav()` preserve 
  SPSS's display widths (@ecortens).

* `read_sas()` gains experimental `cols_only` argument to only read in 
  specified columns (#248).

* tibbles are created with `tibble::as_tibble()`, rather than by "hand" (#229).

* `write_sav()` checks that factors don't have levels with >120 
  characters (#262)

* `write_dta()` no longer checks that all value labels are at most 32 
  characters (since this is not a restriction of dta files) (#239).

* All write methds now check that you're trying to write a data frame (#287).

* Add support for reading (`read_xpt()`) and writing (`write_xpt()`) SAS 
  transport files.

* `write_*` functions turn ordered factors into labelled vectors (#285)

# haven 1.0.0

* The ReadStat library is stored in a subdirectory of `src` (#209, @krlmlr).

* Import tibble so that tibbles are printed consistently (#154, @krlmlr).

* Update to latest ReadStat (#65). Includes: 

    * Support for binary (aka Ross) compression for SAS (#31).
    * Support extended ASCII encoding for Stata (#71).
    * Support for Stata 14 files (#75, #212).
    * Support for SPSS value labels with more than 8 characters (#157).
    * More likely to get an error when attempting to create an invalid
      output file (#171).

* Added support for reading and writing variable formats. Similarly to 
  to variable labels, formats are stored as an attribute on the vector.
  Use `zap_formats()` if you want to remove these attributes.
  (@gorcha, #119, #123).
  
* Added support for reading file "label" and "notes". These are not currently
  printed, but are stored in the attributes if you need to access them (#186).
  
* Added support for "tagged" missing values (in Stata these are called 
  "extended" and in SAS these are called "special") which carry an extra
  byte of information: a character label from "a" to "z". The downside of
  this change is that all integer columns are now converted to doubles, 
  to support the encoding of the tag in the payload of a NaN.
  
* New `labelled_spss()` is a subclass of `labelled()` that can model
  user missing values from SPSS. These can either be a set of distinct
  values, or for numeric vectors, a range. `zap_labels()` strips labels,
  and replaces user-defined missing values with `NA`. New `zap_missing()`
  just replaces user-defined missing vlaues with `NA`. 
  
    `labelled_spss()` is potentially dangerous to work with in R because
    base functions don't know about `labelled_spss()` functions so will 
    return the wrong result in the presence of user-defined missing values.
    For this reason, they will only be created by `read_spss()` when
    `user_na = TRUE` (normally user-defined missings are converted to
    NA). 

* `as_factor()` no longer drops the `label` attribute (variable label) when
   used (#177, @itsdalmo).

* Using `as_factor()` with `levels = "default` or `levels = "both"` preserves
  unused labels (implicit missing) when converting (#172, @itsdalmo). Labels 
  (and the resulting factor levels) are always sorted by values.

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
  (#124). Putting a labelled vector in a data frame now generates the correct
  column name (#193).

* `read_dta()` now recognises "%d" and custom date types (#80, #130).
  It also gains an encoding parameter which you can use to override
  the default encoding. This is particularly useful for Stata 13 and below
  which did not store the encoding used in the file (#163). 

* `read_por()` now actually works (#35).

* `read_sav()` now correctly recognises EDATE and JDATE formats as dates (#72).
  Variables with format DATE, ADATE, EDATE, JDATE or SDATE are imported as 
  `Date` variables instead of `POSIXct`. You can now set `user_na = TRUE` to 
  preserve user defined missing values: they will be given class 
  `labelled_spss`.

* `read_dta()`, `read_sas()`, and `read_sav()` have a better test for missing
  string values (#79). They can all read from connections and compressed files
  (@lionel-, #109)

* `read_sas()` gains an encoding parameter to overide the encoding stored
  in the file if it is incorrect (#176). It gets better argument names (#214).

* Added `type_sum()` method for labelled objects so they print nicely in
  tibbles.

* `write_dta()` now verifies that variable names are valid Stata variables
  (#132), and throws an error if you attempt to save a labelled vector that
  is not an integer (#144). You can choose which `version` of Stata's file 
  format to output (#217).

* New `write_sas()` allows you to write data frames out to `sas7bdat` 
  files. This is still somewhat experimental.

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
  
* `print.labelled()` and `print.labelled_spss()` now display the type.

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
