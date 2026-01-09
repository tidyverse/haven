# Changelog

## haven (development version)

- When a string variable has a date and/or time format `read_*()`
  functions now warn and treat the variable as a plain string instead of
  throwing an error
  ([\#747](https://github.com/tidyverse/haven/issues/747)). This should
  not normally occur, but has been observed in files produced by 3rd
  party software.

- `write_*()` functions now take into account the width of value labels
  when calculating string variable widths. Previously it was possible to
  create value label sets with values that were wider than the string
  variable, causing issues when reading files
  ([\#537](https://github.com/tidyverse/haven/issues/537)).

- [`write_xpt()`](https://haven.tidyverse.org/dev/reference/read_xpt.md)
  now errors if variable names are too long for the XPT version: 8 bytes
  for version 5, 32 bytes for version 8. Previously, long names were
  silently truncated for version 5 and error messaging was inconsistent
  ([\#771](https://github.com/tidyverse/haven/issues/771)).

- [`read_sas()`](https://haven.tidyverse.org/dev/reference/read_sas.md)
  and
  [`read_xpt()`](https://haven.tidyverse.org/dev/reference/read_xpt.md)
  now supports all datetime, date and time formats
  ([\#780](https://github.com/tidyverse/haven/issues/780)) and correctly
  recognises DATEAMPM as datetime
  ([\#789](https://github.com/tidyverse/haven/issues/789)).

- Fix bug that caused
  [`write_xpt()`](https://haven.tidyverse.org/dev/reference/read_xpt.md)
  to fail when writing tagged `NA` values
  ([\#755](https://github.com/tidyverse/haven/issues/755)).

- Tagged `NA` values are now silently converted to lower case when they
  are created in R with
  [`tagged_na()`](https://haven.tidyverse.org/dev/reference/tagged_na.md)
  or checked with
  [`is_tagged_na()`](https://haven.tidyverse.org/dev/reference/tagged_na.md).
  These are case insensitive when reading and writing, so forcing lower
  case removes any potential ambiguity.

- Character limit checks in
  [`write_xpt()`](https://haven.tidyverse.org/dev/reference/read_xpt.md)
  now check for the number of bytes instead of the number of characters
  ([\#746](https://github.com/tidyverse/haven/issues/746)).

## haven 2.5.5

CRAN release: 2025-05-30

- Updated ReadStat to fix stricter gcc diagnostics.

## haven 2.5.4

CRAN release: 2023-11-30

- Fix for upcoming R-devel change.

## haven 2.5.3

CRAN release: 2023-06-30

- Fix for upcoming R-devel change.

## haven 2.5.2

CRAN release: 2023-02-28

- Updated to ReadStat 1.1.9.

  - Fix various SAS catalog file reading bugs
    ([\#529](https://github.com/tidyverse/haven/issues/529),
    [\#653](https://github.com/tidyverse/haven/issues/653),
    [\#680](https://github.com/tidyverse/haven/issues/680),
    [\#696](https://github.com/tidyverse/haven/issues/696),
    [\#705](https://github.com/tidyverse/haven/issues/705)).
  - Increase maximum SAS page file size to 16MiB
    ([\#697](https://github.com/tidyverse/haven/issues/697)).
  - Ignore invalid SAV timestamp strings
    ([\#683](https://github.com/tidyverse/haven/issues/683)).
  - Fix compiler warnings
    ([\#707](https://github.com/tidyverse/haven/issues/707)).

- The experimental
  [`write_sas()`](https://haven.tidyverse.org/dev/reference/write_sas.md)
  function has been deprecated
  ([\#224](https://github.com/tidyverse/haven/issues/224)). The sas7bdat
  file format is complex and undocumented, and as such writing SAS files
  is not officially supported by ReadStat.
  [`write_xpt()`](https://haven.tidyverse.org/dev/reference/read_xpt.md)
  should be used instead - it produces files in the SAS transport
  format, which has limitations but will be reliably read by SAS.

- `write_*()` functions gain a new `adjust_tz` argument to allow more
  control over time zone conversion for date-time variables
  ([\#702](https://github.com/tidyverse/haven/issues/702)). Thanks to
  [@jmobrien](https://github.com/jmobrien) for the detailed issue and
  feedback.

  Stata, SPSS and SAS do not have a concept of time zone. Since haven
  2.4.0 date-time values in non-UTC time zones are implicitly converted
  when writing to ensure the time displayed in Stata/SPSS/SAS will match
  the time displayed to the user in R (see
  [\#555](https://github.com/tidyverse/haven/issues/555)). This is the
  behaviour when `adjust_tz = TRUE` (the default). Although this is in
  line with general user expectations it can cause issues when the time
  zone is important, for e.g. when looking at differences between time
  points, since the underlying numeric data is changed to preserve the
  displayed time. Use `adjust_tz = FALSE` to write the time as the
  corresponding UTC value, which will appear different to the user but
  preserves the underlying numeric data.

- `write_*()` functions previously returned the data frame with minor
  alterations made to date-time variables. These functions now invisibly
  return the original input data frame unchanged
  ([@jmobrien](https://github.com/jmobrien),
  [\#702](https://github.com/tidyverse/haven/issues/702)).

- Fix bug in string variable width calculation that treated `NA` values
  as width

  2.  `NA` values are now treated as blanks for width calculations
      ([\#699](https://github.com/tidyverse/haven/issues/699)).

## haven 2.5.1

CRAN release: 2022-08-22

- All
  [`labelled()`](https://haven.tidyverse.org/dev/reference/labelled.md)
  vectors now have left-aligned column headers when printing in tibbles
  for better alignment with labels
  ([\#676](https://github.com/tidyverse/haven/issues/676)).

- `write_*()` now accept functions as well as strings in the
  `.name_repair` argument in line with the documentation. Previously
  they only supported string values
  ([\#684](https://github.com/tidyverse/haven/issues/684)).

- [`write_sav()`](https://haven.tidyverse.org/dev/reference/read_spss.md)
  variable name validation no longer treats all non-ASCII characters as
  invalid ([\#689](https://github.com/tidyverse/haven/issues/689)).

## haven 2.5.0

CRAN release: 2022-04-15

### New author

- @gorcha is now a haven author in recognition of his significant and
  sustained contributions.

### File writing improvements

- All `write_` functions can now write custom variable widths by setting
  the `width` attribute
  ([\#650](https://github.com/tidyverse/haven/issues/650)).

- When writing files, the minimum width for character variables is
  now 1. This fixes issues with statistical software reading blank
  character variables with width 0
  ([\#650](https://github.com/tidyverse/haven/issues/650)).

- [`write_dta()`](https://haven.tidyverse.org/dev/reference/read_dta.md)
  now uses strL when strings are too long to be stored in an str#
  variable ([\#437](https://github.com/tidyverse/haven/issues/437)).
  strL is used when strings are longer than 2045 characters by default,
  which matches Stata’s behaviour, but this can be reduced with the
  `strl_threshold` argument.

- [`write_xpt()`](https://haven.tidyverse.org/dev/reference/read_xpt.md)
  can now write dataset labels with the `label` argument, which defaults
  to the `label` attribute of the input data frame, if present
  ([\#562](https://github.com/tidyverse/haven/issues/562)).

- [`write_sav()`](https://haven.tidyverse.org/dev/reference/read_spss.md)
  now checks for case-insensitive duplicate variable names
  ([@juansebastianl](https://github.com/juansebastianl),
  [\#641](https://github.com/tidyverse/haven/issues/641)) and verifies
  that variable names are valid SPSS variables.

- The `compress` argument for
  [`write_sav()`](https://haven.tidyverse.org/dev/reference/read_spss.md)
  now supports all 3 SPSS compression modes specified as a character
  string - “byte”, “none” and “zsav”
  ([\#614](https://github.com/tidyverse/haven/issues/614)). `TRUE` and
  `FALSE` can be used for backwards compatibility, and correspond to the
  “zsav” and “none” options respectively.

- [`write_sav()`](https://haven.tidyverse.org/dev/reference/read_spss.md)
  successfully writes user missing values and ranges for
  [`labelled()`](https://haven.tidyverse.org/dev/reference/labelled.md)
  integer vectors
  ([\#596](https://github.com/tidyverse/haven/issues/596)).

- POSIXct and POSIXlt values with no time component (e.g. “2010-01-01”)
  were being converted to `NA` when attempting to convert the output
  timezone to UTC. These now output successfully
  ([\#634](https://github.com/tidyverse/haven/issues/634)).

- Fix bug in output timezone conversion that was causing variable labels
  and other variable attributes to disappear
  ([\#624](https://github.com/tidyverse/haven/issues/624)).

### Other improvements and fixes

- Updated to ReadStat 1.1.8 RC.

  - Fix bug when writing formats to XPT files
    ([\#650](https://github.com/tidyverse/haven/issues/650)).
  - Fix off by one error in indexing for strL variables
    ([\#437](https://github.com/tidyverse/haven/issues/437)).

- [`labelled()`](https://haven.tidyverse.org/dev/reference/labelled.md)
  vectors now throw a warning when combining two vectors with
  conflicting labels
  ([\#667](https://github.com/tidyverse/haven/issues/667)).

- [`zap_labels()`](https://haven.tidyverse.org/dev/reference/zap_labels.md)
  gains a `user_na` argument to control whether user-defined missing
  values are converted to `NA` or left as is
  ([\#638](https://github.com/tidyverse/haven/issues/638)).

- vctrs casting and coercion generics now do less work when working with
  two identical
  [`labelled()`](https://haven.tidyverse.org/dev/reference/labelled.md)
  vectors. This significantly improves performance when working with
  [`labelled()`](https://haven.tidyverse.org/dev/reference/labelled.md)
  vectors in grouped data frames
  ([\#658](https://github.com/tidyverse/haven/issues/658)).

- Errors and warnings now use `cli_abort()` and `cli_warning()`
  ([\#661](https://github.com/tidyverse/haven/issues/661)).

### Dependency changes

- R 3.4 is now the minimum supported version, in line with [tidyverse
  policy](https://www.tidyverse.org/blog/2019/04/r-version-support/).

- cli \>= 3.0.0 has been added to Imports to support new error
  messaging.

- lifecycle has been added to Imports, and is now used to manage
  deprecations.

## haven 2.4.3

CRAN release: 2021-08-04

- Fix build failure on Solaris.

## haven 2.4.2

CRAN release: 2021-08-02

- Updated to ReadStat 1.1.7 RC
  ([\#620](https://github.com/tidyverse/haven/issues/620)).

- [`read_dta()`](https://haven.tidyverse.org/dev/reference/read_dta.md)
  no longer crashes if it sees StrL variables with missing values
  ([@gorcha](https://github.com/gorcha),
  [\#594](https://github.com/tidyverse/haven/issues/594),
  [\#600](https://github.com/tidyverse/haven/issues/600),
  [\#608](https://github.com/tidyverse/haven/issues/608)).

- [`write_dta()`](https://haven.tidyverse.org/dev/reference/read_dta.md)
  now correctly handles “labelled”-class numeric (double) variables that
  don’t have value labels ([@jmobrien](https://github.com/jmobrien),
  [\#606](https://github.com/tidyverse/haven/issues/606),
  [\#609](https://github.com/tidyverse/haven/issues/609)).

- [`write_dta()`](https://haven.tidyverse.org/dev/reference/read_dta.md)
  now allows variable names up to 32 characters
  ([@sbae](https://github.com/sbae),
  [\#605](https://github.com/tidyverse/haven/issues/605)).

- Can now correctly combine
  [`labelled_spss()`](https://haven.tidyverse.org/dev/reference/labelled_spss.md)
  with identical labels ([@gorcha](https://github.com/gorcha),
  [\#599](https://github.com/tidyverse/haven/issues/599)).

## haven 2.4.1

CRAN release: 2021-04-23

- Fix buglet when combining
  [`labelled()`](https://haven.tidyverse.org/dev/reference/labelled.md)
  with identical labels.

## haven 2.4.0

CRAN release: 2021-04-14

### New features

- [`labelled_spss()`](https://haven.tidyverse.org/dev/reference/labelled_spss.md)
  gains full vctrs support thanks to the hard work of
  [@gorcha](https://github.com/gorcha)
  ([\#527](https://github.com/tidyverse/haven/issues/527),
  [\#534](https://github.com/tidyverse/haven/issues/534),
  [\#538](https://github.com/tidyverse/haven/issues/538),
  [\#557](https://github.com/tidyverse/haven/issues/557)). This means
  that they should now work seamlessly in dplyr 1.0.0, tidyr 1.0.0 and
  other packages that use vctrs.

- [`labelled()`](https://haven.tidyverse.org/dev/reference/labelled.md)
  vectors are more permissive when concatenating; output labels will be
  a combination of the left-hand and the right-hand side, preferring
  values assigned to the left-hand side
  ([\#543](https://github.com/tidyverse/haven/issues/543)).

- Date-times are no longer forced to UTC, but instead converted to the
  equivalent UTC
  ([\#555](https://github.com/tidyverse/haven/issues/555)). This should
  ensure that you see the same date-time in R and in Stata/SPSS/SAS.

### Minor improvements and bug fixes

- Updated to ReadStat 1.1.5. Most importantly this includes support for
  SAS binary compression.

- `as_factor(levels = "values")` preserves values of unlabelled elements
  ([\#570](https://github.com/tidyverse/haven/issues/570)).

- [`labelled_spss()`](https://haven.tidyverse.org/dev/reference/labelled_spss.md)
  is a little stricter: it prevents `na_range` and `na_value` from
  containing missing values, and ensures that `na_range` is in the
  correct order
  ([\#574](https://github.com/tidyverse/haven/issues/574)).

- [`read_spss()`](https://haven.tidyverse.org/dev/reference/read_spss.md)
  now reads NA values and ranges of character variables
  ([\#409](https://github.com/tidyverse/haven/issues/409)).

- [`write_dta()`](https://haven.tidyverse.org/dev/reference/read_dta.md)
  now correctly writes tagged NAs (including tagged NAs in labels)
  ([\#583](https://github.com/tidyverse/haven/issues/583)) and once
  again validates length of variables names
  ([\#485](https://github.com/tidyverse/haven/issues/485)).

- `write_*()` now validate file and variable metadata with ReadStat.
  This should prevent many invalid files from being written
  ([\#408](https://github.com/tidyverse/haven/issues/408)).
  Additionally, validation failures now provide more details about the
  source of the problem (e.g. the column name of the problem)
  ([\#463](https://github.com/tidyverse/haven/issues/463)).

- `write_sav(compress = FALSE)` now uses SPSS bytecode compression
  instead of the rarely-used uncompressed mode. `compress = TRUE`
  continues to use the newer (and not universally supported, but more
  compact) zlib format ([@oliverbock](https://github.com/oliverbock),
  [\#544](https://github.com/tidyverse/haven/issues/544)).

## haven 2.3.1

CRAN release: 2020-06-01

- Add missing methods so
  [`median()`](https://rdrr.io/r/stats/median.html),
  [`quantile()`](https://rdrr.io/r/stats/quantile.html) and
  [`summary()`](https://rdrr.io/r/base/summary.html) work once more
  ([\#520](https://github.com/tidyverse/haven/issues/520)).

- Add missing cast methods
  ([\#522](https://github.com/tidyverse/haven/issues/522)).

## haven 2.3.0

CRAN release: 2020-05-24

- [`labelled()`](https://haven.tidyverse.org/dev/reference/labelled.md)
  gains the necessary support to work seemlessly in dplyr 1.0.0, tidyr
  1.0.0, and other packages that use vctrs
  ([@mikmart](https://github.com/mikmart),
  [\#496](https://github.com/tidyverse/haven/issues/496)).

- [`labelled()`](https://haven.tidyverse.org/dev/reference/labelled.md)
  vectors now explicitly inherit from the corresponding base types
  (e.g. integer, double, or character)
  ([\#509](https://github.com/tidyverse/haven/issues/509)).

- ReadStat update, including
  [`read_sas()`](https://haven.tidyverse.org/dev/reference/read_sas.md)
  supports for “any” encoding
  ([\#482](https://github.com/tidyverse/haven/issues/482)), and fixes
  for compiler warnings.

## haven 2.2.0

CRAN release: 2019-11-08

### Partial reading

Thanks to the hard work of [@mikmart](https://github.com/mikmart), all
`read_*()` functions gain three new arguments that allow you to read in
only part of a large file:

- `col_select`: selects columns to read with a tidyselect interface
  ([\#248](https://github.com/tidyverse/haven/issues/248)).
- `skip`: skips rows before reading data
  ([\#370](https://github.com/tidyverse/haven/issues/370)).
- `n_max`: limits the number of rows to read.

This also brings with it a deprecation: `cols_only` in
[`read_sas()`](https://haven.tidyverse.org/dev/reference/read_sas.md)
has been deprecated in favour of the new `col_select` argument.

### Minor improvements and bug fixes

- [`as_factor()`](https://haven.tidyverse.org/dev/reference/as_factor.md)
  allows non-unique labels when `levels = "label"`. This fixes a
  particularly annoying printing bug
  ([\#424](https://github.com/tidyverse/haven/issues/424),
  [@gergness](https://github.com/gergness))

- [`read_sas()`](https://haven.tidyverse.org/dev/reference/read_sas.md)
  now supports (IS\|E\|B)8601(DT\|DA\|TM) date/time formats
  ([@mikmart](https://github.com/mikmart)).

- All `write_` functions gain a `.name_repair` argument that controls
  what happens when the input dataset has repeated column names
  ([\#436](https://github.com/tidyverse/haven/issues/436)).

- All `write_` functions can now write labelled vectors with `NULL`
  labels ([\#442](https://github.com/tidyverse/haven/issues/442)).

- [`write_dta()`](https://haven.tidyverse.org/dev/reference/read_dta.md)
  can now write dataset labels with the `label` argument, which defaults
  to the `label` attribute of the input data frame, if present
  ([@gorcha](https://github.com/gorcha),
  [\#449](https://github.com/tidyverse/haven/issues/449)).

- [`write_dta()`](https://haven.tidyverse.org/dev/reference/read_dta.md)
  works better with Stata 15, thanks to updated ReadStat
  ([\#461](https://github.com/tidyverse/haven/issues/461))

## haven 2.1.1

CRAN release: 2019-07-04

- Fixes for R CMD check

## haven 2.1.0

CRAN release: 2019-02-19

### Improved labelling

`labelled` objects get pretty printing that shows the labels and NA
values when inside of a `tbl_df`. Turn this behaviour off with behavior
using `option(haven.show_pillar_labels = FALSE)`
([\#340](https://github.com/tidyverse/haven/issues/340),
[@gergness](https://github.com/gergness)).

[`labelled()`](https://haven.tidyverse.org/dev/reference/labelled.md)
and
[`labelled_spss()`](https://haven.tidyverse.org/dev/reference/labelled_spss.md)
now allow `NULL` labels. This makes both classes more flexible, allowing
you to use them for their other attributes
([\#219](https://github.com/tidyverse/haven/issues/219)).

[`labelled()`](https://haven.tidyverse.org/dev/reference/labelled.md)
tests that value labels are unique
([@larmarange](https://github.com/larmarange),
[\#364](https://github.com/tidyverse/haven/issues/364))

### Minor improvements and bug fixes

- [`as_factor()`](https://haven.tidyverse.org/dev/reference/as_factor.md):

  - Is faster when input doesn’t contain any missing values
    ([@hughparsonage](https://github.com/hughparsonage)).
  - Added `labelled` method for backward compatbility
    ([\#414](https://github.com/tidyverse/haven/issues/414)).
  - `data.frame` method now correctly passes `...` along
    ([\#407](https://github.com/tidyverse/haven/issues/407),
    [@zkamvar](https://github.com/zkamvar)).

- [`write_dta()`](https://haven.tidyverse.org/dev/reference/read_dta.md)
  now checks that the labelled values are integers, not the values
  themselves ([\#401](https://github.com/tidyverse/haven/issues/401)).

- Updated to latest ReadStat from
  [@evanmiller](https://github.com/evanmiller):

  - [`read_por()`](https://haven.tidyverse.org/dev/reference/read_spss.md)
    can now read files from SPSS 25
    ([\#412](https://github.com/tidyverse/haven/issues/412))
  - [`read_por()`](https://haven.tidyverse.org/dev/reference/read_spss.md)
    now uses base-30 instead of base-10 for the exponent
    ([\#413](https://github.com/tidyverse/haven/issues/413))
  - [`read_sas()`](https://haven.tidyverse.org/dev/reference/read_sas.md)
    can read zero column file
    ([\#420](https://github.com/tidyverse/haven/issues/420))
  - [`read_sav()`](https://haven.tidyverse.org/dev/reference/read_spss.md)
    reads long strings
    ([\#381](https://github.com/tidyverse/haven/issues/381))
  - [`read_sav()`](https://haven.tidyverse.org/dev/reference/read_spss.md)
    has greater memory limit allowing it to read more labels
    ([\#418](https://github.com/tidyverse/haven/issues/418))
  - [`read_spss()`](https://haven.tidyverse.org/dev/reference/read_spss.md)
    reads long variable labels
    ([\#422](https://github.com/tidyverse/haven/issues/422))
  - [`write_sav()`](https://haven.tidyverse.org/dev/reference/read_spss.md)
    no longer creates incorrect column names when \>10k columns
    ([\#410](https://github.com/tidyverse/haven/issues/410))
  - [`write_sav()`](https://haven.tidyverse.org/dev/reference/read_spss.md)
    no longer crashes when writing long label names
    ([\#395](https://github.com/tidyverse/haven/issues/395))

## haven 2.0.0

CRAN release: 2018-11-21

### BREAKING CHANGES

- [`labelled()`](https://haven.tidyverse.org/dev/reference/labelled.md)
  and
  [`labelled_spss()`](https://haven.tidyverse.org/dev/reference/labelled_spss.md)
  now produce objects with class “haven_labelled” and
  “haven_labelled_spss”. Previously, the “labelled” class name clashed
  with the labelled class defined by Hmisc
  ([\#329](https://github.com/tidyverse/haven/issues/329)).

  Unfortunately I couldn’t come up with a way to fix this problem except
  to change the class name; it seems reasonable that haven should be the
  one to change names given that Hmisc has been around much longer. This
  will require some changes to packages that use haven, but shouldn’t
  affect user code.

### Minor improvements

- [`labelled()`](https://haven.tidyverse.org/dev/reference/labelled.md)
  and
  [`labelled_spss()`](https://haven.tidyverse.org/dev/reference/labelled_spss.md)
  now support adding the `label` attribute to the resulting object. The
  `label` is a short, human-readable description of the object, and is
  now also used when printing, and can be easily removed using the new
  [`zap_label()`](https://haven.tidyverse.org/dev/reference/zap_label.md)
  function. ([\#362](https://github.com/tidyverse/haven/issues/362),
  [@huftis](https://github.com/huftis))

  Previously, the `label` attribute was supported both when reading and
  writing SPSS files, but it was not possible to actually create objects
  in R having the `label` attribute using the constructors
  [`labelled()`](https://haven.tidyverse.org/dev/reference/labelled.md)
  or
  [`labelled_spss()`](https://haven.tidyverse.org/dev/reference/labelled_spss.md).

## haven 1.1.2

CRAN release: 2018-06-27

- haven can read and write non-ASCII paths in R 3.5
  ([\#371](https://github.com/tidyverse/haven/issues/371)).

- `labelled_spss` objects preserve their attributes when subsetted
  ([\#360](https://github.com/tidyverse/haven/issues/360),
  [@gergness](https://github.com/gergness)).

- [`read_sav()`](https://haven.tidyverse.org/dev/reference/read_spss.md)
  gains an `encoding` argument to override the encoding stored in the
  file ([\#305](https://github.com/tidyverse/haven/issues/305)).
  [`read_sav()`](https://haven.tidyverse.org/dev/reference/read_spss.md)
  can now read `.zsav` files
  ([\#338](https://github.com/tidyverse/haven/issues/338)).

- `write_*()` functions now invisibly return the input data frame (as
  documented) ([\#349](https://github.com/tidyverse/haven/issues/349),
  [@austensen](https://github.com/austensen)).

- [`write_dta()`](https://haven.tidyverse.org/dev/reference/read_dta.md)
  allows non-ASCII variable labels for version 14 and above
  ([\#383](https://github.com/tidyverse/haven/issues/383)). It also uses
  a less strict check for integers so that a labelled double containing
  only integer values can written
  ([\#343](https://github.com/tidyverse/haven/issues/343)).

- [`write_sav()`](https://haven.tidyverse.org/dev/reference/read_spss.md)
  produces `.zsav` files when `compress = TRUE`
  ([\#338](https://github.com/tidyverse/haven/issues/338)).

- [`write_xpt()`](https://haven.tidyverse.org/dev/reference/read_xpt.md)
  can now set the “member” name, which defaults to the file name san
  extension ([\#328](https://github.com/tidyverse/haven/issues/328)).

- Update to latest readstat.

  - Fixes out of memory error
    ([\#342](https://github.com/tidyverse/haven/issues/342))
  - Now supports reading and writing stata 15 files
    ([\#339](https://github.com/tidyverse/haven/issues/339))
  - Negative integer labelled values were tagged as missing
    ([\#367](https://github.com/tidyverse/haven/issues/367))

- Fix for when
  [`as_factor()`](https://haven.tidyverse.org/dev/reference/as_factor.md)
  with option `levels="labels"` is used on tagged NAs
  ([\#340](https://github.com/tidyverse/haven/issues/340),
  [@gergness](https://github.com/gergness))

## haven 1.1.1

CRAN release: 2018-01-18

- Update to latest readstat. Includes:

  - SPSS: empty charater columns now read as character
    ([\#311](https://github.com/tidyverse/haven/issues/311))
  - SPSS: now write long strings
    ([\#266](https://github.com/tidyverse/haven/issues/266))
  - Stata: reorder labelled vectors on write
    ([\#327](https://github.com/tidyverse/haven/issues/327))
  - State: `encoding` now affects value labels
    ([\#325](https://github.com/tidyverse/haven/issues/325))
  - SAS: can now write wide/long rows
    ([\#272](https://github.com/tidyverse/haven/issues/272),
    [\#335](https://github.com/tidyverse/haven/issues/335)).
  - SAS: can now handle Windows Vietnamese character set
    ([\#336](https://github.com/tidyverse/haven/issues/336))

- [`read_por()`](https://haven.tidyverse.org/dev/reference/read_spss.md)
  and
  [`read_xpt()`](https://haven.tidyverse.org/dev/reference/read_xpt.md)
  now correctly preserve attributes if output needs to be reallocated
  (which is typical behaviour)
  ([\#313](https://github.com/tidyverse/haven/issues/313))

- [`read_sas()`](https://haven.tidyverse.org/dev/reference/read_sas.md)
  recognises date/times format with trailing separator and width
  specifications
  ([\#324](https://github.com/tidyverse/haven/issues/324))

- [`read_sas()`](https://haven.tidyverse.org/dev/reference/read_sas.md)
  gains a `catalog_encoding` argument so you can independently specify
  encoding of data and catalog
  ([\#312](https://github.com/tidyverse/haven/issues/312))

- `write_*()` correctly measures lengths of non-ASCII labels
  ([\#258](https://github.com/tidyverse/haven/issues/258)): this fixes
  the cryptic error “A provided string value was longer than the
  available storage size of the specified column.”

- [`write_dta()`](https://haven.tidyverse.org/dev/reference/read_dta.md)
  now checks for bad labels in all columns, not just the first
  ([\#326](https://github.com/tidyverse/haven/issues/326)).

- [`write_sav()`](https://haven.tidyverse.org/dev/reference/read_spss.md)
  no longer fails on empty factors or factors with an `NA` level
  ([\#301](https://github.com/tidyverse/haven/issues/301)) and writes
  out more metadata for `labelled_spss` vectors
  ([\#334](https://github.com/tidyverse/haven/issues/334)).

## haven 1.1.0

CRAN release: 2017-07-09

- Update to latest readstat. Includes:

  - SAS: support Win baltic code page
    ([\#231](https://github.com/tidyverse/haven/issues/231))
  - SAS: better error messages instead of crashes
    ([\#234](https://github.com/tidyverse/haven/issues/234),
    [\#270](https://github.com/tidyverse/haven/issues/270))
  - SAS: fix “unable to read error”
    ([\#271](https://github.com/tidyverse/haven/issues/271))
  - SPSS: support uppercase time stamps
    ([\#230](https://github.com/tidyverse/haven/issues/230))
  - SPSS: fixes for 252-255 byte strings
    ([\#226](https://github.com/tidyverse/haven/issues/226))
  - SPSS: fixes for 0 byte strings
    ([\#245](https://github.com/tidyverse/haven/issues/245))

- Share
  [`as_factor()`](https://haven.tidyverse.org/dev/reference/as_factor.md)
  with forcats package
  ([\#256](https://github.com/tidyverse/haven/issues/256))

- [`read_sav()`](https://haven.tidyverse.org/dev/reference/read_spss.md)
  once again correctly returns system defined missings as `NA` (rather
  than `NaN`) ([\#223](https://github.com/tidyverse/haven/issues/223)).
  [`read_sav()`](https://haven.tidyverse.org/dev/reference/read_spss.md)
  and
  [`write_sav()`](https://haven.tidyverse.org/dev/reference/read_spss.md)
  preserve SPSS’s display widths
  ([@ecortens](https://github.com/ecortens)).

- [`read_sas()`](https://haven.tidyverse.org/dev/reference/read_sas.md)
  gains experimental `cols_only` argument to only read in specified
  columns ([\#248](https://github.com/tidyverse/haven/issues/248)).

- tibbles are created with
  [`tibble::as_tibble()`](https://tibble.tidyverse.org/reference/as_tibble.html),
  rather than by “hand”
  ([\#229](https://github.com/tidyverse/haven/issues/229)).

- [`write_sav()`](https://haven.tidyverse.org/dev/reference/read_spss.md)
  checks that factors don’t have levels with \>120 characters
  ([\#262](https://github.com/tidyverse/haven/issues/262))

- [`write_dta()`](https://haven.tidyverse.org/dev/reference/read_dta.md)
  no longer checks that all value labels are at most 32 characters
  (since this is not a restriction of dta files)
  ([\#239](https://github.com/tidyverse/haven/issues/239)).

- All write methds now check that you’re trying to write a data frame
  ([\#287](https://github.com/tidyverse/haven/issues/287)).

- Add support for reading
  ([`read_xpt()`](https://haven.tidyverse.org/dev/reference/read_xpt.md))
  and writing
  ([`write_xpt()`](https://haven.tidyverse.org/dev/reference/read_xpt.md))
  SAS transport files.

- `write_*` functions turn ordered factors into labelled vectors
  ([\#285](https://github.com/tidyverse/haven/issues/285))

## haven 1.0.0

CRAN release: 2016-09-23

- The ReadStat library is stored in a subdirectory of `src`
  ([\#209](https://github.com/tidyverse/haven/issues/209),
  [@krlmlr](https://github.com/krlmlr)).

- Import tibble so that tibbles are printed consistently
  ([\#154](https://github.com/tidyverse/haven/issues/154),
  [@krlmlr](https://github.com/krlmlr)).

- Update to latest ReadStat
  ([\#65](https://github.com/tidyverse/haven/issues/65)). Includes:

  - Support for binary (aka Ross) compression for SAS
    ([\#31](https://github.com/tidyverse/haven/issues/31)).
  - Support extended ASCII encoding for Stata
    ([\#71](https://github.com/tidyverse/haven/issues/71)).
  - Support for Stata 14 files
    ([\#75](https://github.com/tidyverse/haven/issues/75),
    [\#212](https://github.com/tidyverse/haven/issues/212)).
  - Support for SPSS value labels with more than 8 characters
    ([\#157](https://github.com/tidyverse/haven/issues/157)).
  - More likely to get an error when attempting to create an invalid
    output file
    ([\#171](https://github.com/tidyverse/haven/issues/171)).

- Added support for reading and writing variable formats. Similarly to
  to variable labels, formats are stored as an attribute on the vector.
  Use
  [`zap_formats()`](https://haven.tidyverse.org/dev/reference/zap_formats.md)
  if you want to remove these attributes.
  ([@gorcha](https://github.com/gorcha),
  [\#119](https://github.com/tidyverse/haven/issues/119),
  [\#123](https://github.com/tidyverse/haven/issues/123)).

- Added support for reading file “label” and “notes”. These are not
  currently printed, but are stored in the attributes if you need to
  access them ([\#186](https://github.com/tidyverse/haven/issues/186)).

- Added support for “tagged” missing values (in Stata these are called
  “extended” and in SAS these are called “special”) which carry an extra
  byte of information: a character label from “a” to “z”. The downside
  of this change is that all integer columns are now converted to
  doubles, to support the encoding of the tag in the payload of a NaN.

- New
  [`labelled_spss()`](https://haven.tidyverse.org/dev/reference/labelled_spss.md)
  is a subclass of
  [`labelled()`](https://haven.tidyverse.org/dev/reference/labelled.md)
  that can model user missing values from SPSS. These can either be a
  set of distinct values, or for numeric vectors, a range.
  [`zap_labels()`](https://haven.tidyverse.org/dev/reference/zap_labels.md)
  strips labels, and replaces user-defined missing values with `NA`. New
  [`zap_missing()`](https://haven.tidyverse.org/dev/reference/zap_missing.md)
  just replaces user-defined missing values with `NA`.

  [`labelled_spss()`](https://haven.tidyverse.org/dev/reference/labelled_spss.md)
  is potentially dangerous to work with in R because base functions
  don’t know about
  [`labelled_spss()`](https://haven.tidyverse.org/dev/reference/labelled_spss.md)
  functions so will return the wrong result in the presence of
  user-defined missing values. For this reason, they will only be
  created by
  [`read_spss()`](https://haven.tidyverse.org/dev/reference/read_spss.md)
  when `user_na = TRUE` (normally user-defined missings are converted to
  NA).

- [`as_factor()`](https://haven.tidyverse.org/dev/reference/as_factor.md)
  no longer drops the `label` attribute (variable label) when used
  ([\#177](https://github.com/tidyverse/haven/issues/177),
  [@itsdalmo](https://github.com/itsdalmo)).

- Using
  [`as_factor()`](https://haven.tidyverse.org/dev/reference/as_factor.md)
  with `levels = "default` or `levels = "both"` preserves unused labels
  (implicit missing) when converting
  ([\#172](https://github.com/tidyverse/haven/issues/172),
  [@itsdalmo](https://github.com/itsdalmo)). Labels (and the resulting
  factor levels) are always sorted by values.

- [`as_factor()`](https://haven.tidyverse.org/dev/reference/as_factor.md)
  gains a new `levels = "default"` mechanism. This uses the labels where
  present, and otherwise uses the labels. This is now the default, as it
  seems to map better to the semantics of labelled values in other
  statistical packages
  ([\#81](https://github.com/tidyverse/haven/issues/81)). You can also
  use `levels = "both"` to combine the value and the label into a single
  string ([\#82](https://github.com/tidyverse/haven/issues/82)). It also
  gains a method for data frames, so you can easily convert every
  labelled column to a factor in one function call.

- New
  [`vignette("semantics", package = "haven")`](https://haven.tidyverse.org/dev/articles/semantics.md)
  discusses the semantics of missing values and labelling in SAS, SPSS,
  and Stata, and how they are translated into R.

- Support for `hms()` has been moved into the hms package
  ([\#162](https://github.com/tidyverse/haven/issues/162)). Time
  varibles now have class `c("hms", "difftime")` and a `units` attribute
  with value “secs”
  ([\#162](https://github.com/tidyverse/haven/issues/162)).

- [`labelled()`](https://haven.tidyverse.org/dev/reference/labelled.md)
  is less strict with its checks: you can mix double and integer value
  and labels ([\#86](https://github.com/tidyverse/haven/issues/86),
  [\#110](https://github.com/tidyverse/haven/issues/110),
  [@lionel-](https://github.com/lionel-)), and
  [`is.labelled()`](https://haven.tidyverse.org/dev/reference/labelled.md)
  is now exported
  ([\#124](https://github.com/tidyverse/haven/issues/124)). Putting a
  labelled vector in a data frame now generates the correct column name
  ([\#193](https://github.com/tidyverse/haven/issues/193)).

- [`read_dta()`](https://haven.tidyverse.org/dev/reference/read_dta.md)
  now recognises “%d” and custom date types
  ([\#80](https://github.com/tidyverse/haven/issues/80),
  [\#130](https://github.com/tidyverse/haven/issues/130)). It also gains
  an encoding parameter which you can use to override the default
  encoding. This is particularly useful for Stata 13 and below which did
  not store the encoding used in the file
  ([\#163](https://github.com/tidyverse/haven/issues/163)).

- [`read_por()`](https://haven.tidyverse.org/dev/reference/read_spss.md)
  now actually works
  ([\#35](https://github.com/tidyverse/haven/issues/35)).

- [`read_sav()`](https://haven.tidyverse.org/dev/reference/read_spss.md)
  now correctly recognises EDATE and JDATE formats as dates
  ([\#72](https://github.com/tidyverse/haven/issues/72)). Variables with
  format DATE, ADATE, EDATE, JDATE or SDATE are imported as `Date`
  variables instead of `POSIXct`. You can now set `user_na = TRUE` to
  preserve user defined missing values: they will be given class
  `labelled_spss`.

- [`read_dta()`](https://haven.tidyverse.org/dev/reference/read_dta.md),
  [`read_sas()`](https://haven.tidyverse.org/dev/reference/read_sas.md),
  and
  [`read_sav()`](https://haven.tidyverse.org/dev/reference/read_spss.md)
  have a better test for missing string values
  ([\#79](https://github.com/tidyverse/haven/issues/79)). They can all
  read from connections and compressed files
  ([@lionel-](https://github.com/lionel-),
  [\#109](https://github.com/tidyverse/haven/issues/109))

- [`read_sas()`](https://haven.tidyverse.org/dev/reference/read_sas.md)
  gains an encoding parameter to overide the encoding stored in the file
  if it is incorrect
  ([\#176](https://github.com/tidyverse/haven/issues/176)). It gets
  better argument names
  ([\#214](https://github.com/tidyverse/haven/issues/214)).

- Added `type_sum()` method for labelled objects so they print nicely in
  tibbles.

- [`write_dta()`](https://haven.tidyverse.org/dev/reference/read_dta.md)
  now verifies that variable names are valid Stata variables
  ([\#132](https://github.com/tidyverse/haven/issues/132)), and throws
  an error if you attempt to save a labelled vector that is not an
  integer ([\#144](https://github.com/tidyverse/haven/issues/144)). You
  can choose which `version` of Stata’s file format to output
  ([\#217](https://github.com/tidyverse/haven/issues/217)).

- New
  [`write_sas()`](https://haven.tidyverse.org/dev/reference/write_sas.md)
  allows you to write data frames out to `sas7bdat` files. This is still
  somewhat experimental.

- [`write_sav()`](https://haven.tidyverse.org/dev/reference/read_spss.md)
  writes hms variables to SPSS time variables, and the “measure” type is
  set for each variable
  ([\#133](https://github.com/tidyverse/haven/issues/133)).

- [`write_dta()`](https://haven.tidyverse.org/dev/reference/read_dta.md)
  and
  [`write_sav()`](https://haven.tidyverse.org/dev/reference/read_spss.md)
  support writing date and date/times
  ([\#25](https://github.com/tidyverse/haven/issues/25),
  [\#139](https://github.com/tidyverse/haven/issues/139),
  [\#145](https://github.com/tidyverse/haven/issues/145)). Labelled
  values are always converted to UTF-8 before being written out
  ([\#87](https://github.com/tidyverse/haven/issues/87)). Infinite
  values are now converted to missing values since SPSS and Stata don’t
  support them ([\#149](https://github.com/tidyverse/haven/issues/149)).
  Both use a better test for missing values
  ([\#70](https://github.com/tidyverse/haven/issues/70)).

- [`zap_labels()`](https://haven.tidyverse.org/dev/reference/zap_labels.md)
  has been completely overhauled. It now works
  ([@markriseley](https://github.com/markriseley),
  [\#69](https://github.com/tidyverse/haven/issues/69)), and only drops
  label attributes; it no longer replaces labelled values with `NA`s. It
  also gains a data frame method that zaps the labels from every column.

- `print.labelled()` and `print.labelled_spss()` now display the type.

## haven 0.2.0

CRAN release: 2015-04-09

- fixed a bug in `as_factor.labelled`, which generated ’s and wrong
  labels for integer labels.

- [`zap_labels()`](https://haven.tidyverse.org/dev/reference/zap_labels.md)
  now leaves unlabelled vectors unchanged, making it easier to apply to
  all columns.

- [`write_dta()`](https://haven.tidyverse.org/dev/reference/read_dta.md)
  and
  [`write_sav()`](https://haven.tidyverse.org/dev/reference/read_spss.md)
  take more care to always write output as UTF-8
  ([\#36](https://github.com/tidyverse/haven/issues/36))

- [`write_dta()`](https://haven.tidyverse.org/dev/reference/read_dta.md)
  and
  [`write_sav()`](https://haven.tidyverse.org/dev/reference/read_spss.md)
  won’t crash if you give them invalid paths, and you can now use `~` to
  refer to your home directory
  ([\#37](https://github.com/tidyverse/haven/issues/37)).

- Byte variables are now correctly read into integers (not strings,
  [\#45](https://github.com/tidyverse/haven/issues/45)), and missing
  values are captured correctly
  ([\#43](https://github.com/tidyverse/haven/issues/43)).

- Added
  [`read_stata()`](https://haven.tidyverse.org/dev/reference/read_dta.md)
  as alias to
  [`read_dta()`](https://haven.tidyverse.org/dev/reference/read_dta.md)
  ([\#52](https://github.com/tidyverse/haven/issues/52)).

- [`read_spss()`](https://haven.tidyverse.org/dev/reference/read_spss.md)
  uses extension to automatically choose between
  [`read_sav()`](https://haven.tidyverse.org/dev/reference/read_spss.md)
  and
  [`read_por()`](https://haven.tidyverse.org/dev/reference/read_spss.md)
  ([\#53](https://github.com/tidyverse/haven/issues/53))

- Updates from ReadStat. Including fixes for various parsing bugs, more
  encodings, and better support for large files.

- hms objects deal better with missings when printing.

- Fixed bug causing labels for numeric variables to be read in as
  integers and associated error:
  `` Error: `x` and `labels` must be same type ``

## haven 0.1.1

CRAN release: 2015-03-03

- Fixed memory initialisation problems found by valgrind.
