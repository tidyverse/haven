# haven 0.1.1.9000

* Added `read_stata()` as alias to `read_dta()` (#52).

* `read_spss()` uses extension to automatically choose between `read_sav()`
  and `read_por()` (#53)

* Updates from ReadStat. Including fixes for various parsing bugs, more 
  encodings, and better support for large files.

* hms objects deal better with missings when printing.

* Fixed bug causing labels for numeric variables to be read in as
  integers and associated error:
  ``Error: `x` and `labels` must be same type``

# haven 0.1.1

* Fixed memory initialisation problems found by valgrind.
