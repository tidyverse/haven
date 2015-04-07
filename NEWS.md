# haven 0.1.1.9000

* `write_dta()` and `write_sav()` take more care to always write output as
  UTF-8 (#36)

* `write_dta()` and `write_sav()` won't crash if you give them invalid paths,
  and you can now use `~` to refer to your home directory (#37).

* Stata byte variables are now correctly read into integers (not strings, #45).

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
