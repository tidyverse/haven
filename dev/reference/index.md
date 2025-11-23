# Package index

## Reading and writing

- [`read_dta()`](https://haven.tidyverse.org/dev/reference/read_dta.md)
  [`read_stata()`](https://haven.tidyverse.org/dev/reference/read_dta.md)
  [`write_dta()`](https://haven.tidyverse.org/dev/reference/read_dta.md)
  : Read and write Stata DTA files
- [`read_sas()`](https://haven.tidyverse.org/dev/reference/read_sas.md)
  : Read SAS files
- [`read_sav()`](https://haven.tidyverse.org/dev/reference/read_spss.md)
  [`read_por()`](https://haven.tidyverse.org/dev/reference/read_spss.md)
  [`write_sav()`](https://haven.tidyverse.org/dev/reference/read_spss.md)
  [`read_spss()`](https://haven.tidyverse.org/dev/reference/read_spss.md)
  : Read and write SPSS files
- [`read_xpt()`](https://haven.tidyverse.org/dev/reference/read_xpt.md)
  [`write_xpt()`](https://haven.tidyverse.org/dev/reference/read_xpt.md)
  : Read and write SAS transport files

## Labelled vectors

SAS, SPSS, and Stata share a simply concept of “labelled” vectors, which
are similar to factors but a little more general. The
[`labelled()`](https://haven.tidyverse.org/dev/reference/labelled.md)
class provides a natural representaion in R.

- [`labelled_spss()`](https://haven.tidyverse.org/dev/reference/labelled_spss.md)
  : Labelled vectors for SPSS
- [`labelled()`](https://haven.tidyverse.org/dev/reference/labelled.md)
  [`is.labelled()`](https://haven.tidyverse.org/dev/reference/labelled.md)
  : Create a labelled vector.
- [`print_labels()`](https://haven.tidyverse.org/dev/reference/print_labels.md)
  : Print the labels of a labelled vector
- [`as_factor(`*`<data.frame>`*`)`](https://haven.tidyverse.org/dev/reference/as_factor.md)
  [`as_factor(`*`<haven_labelled>`*`)`](https://haven.tidyverse.org/dev/reference/as_factor.md)
  [`as_factor(`*`<labelled>`*`)`](https://haven.tidyverse.org/dev/reference/as_factor.md)
  : Convert labelled vectors to factors

## Tagged missing values

Both SAS and Stata supported tagged missing values, where a missing
value can have multiple “types”, given by an letter from A through Z.
[`tagged_na()`](https://haven.tidyverse.org/dev/reference/tagged_na.md)
provides a convenient way of representing these types of missing values
in R by taking advantage of the binary representation of `NA`.

- [`tagged_na()`](https://haven.tidyverse.org/dev/reference/tagged_na.md)
  [`na_tag()`](https://haven.tidyverse.org/dev/reference/tagged_na.md)
  [`is_tagged_na()`](https://haven.tidyverse.org/dev/reference/tagged_na.md)
  [`format_tagged_na()`](https://haven.tidyverse.org/dev/reference/tagged_na.md)
  [`print_tagged_na()`](https://haven.tidyverse.org/dev/reference/tagged_na.md)
  : "Tagged" missing values

## Remove attributes

There are a number of SPSS/SAS/Stata features that have no direct
equivalent in R. Haven preserves them so you can choose what do with
them. To simply eliminate them, use one of the zap functions.

- [`zap_empty()`](https://haven.tidyverse.org/dev/reference/zap_empty.md)
  : Convert empty strings into missing values
- [`zap_formats()`](https://haven.tidyverse.org/dev/reference/zap_formats.md)
  : Remove format attributes
- [`zap_label()`](https://haven.tidyverse.org/dev/reference/zap_label.md)
  : Zap variable labels
- [`zap_labels()`](https://haven.tidyverse.org/dev/reference/zap_labels.md)
  : Zap value labels
- [`zap_missing()`](https://haven.tidyverse.org/dev/reference/zap_missing.md)
  : Zap special missings to regular R missings
- [`zap_widths()`](https://haven.tidyverse.org/dev/reference/zap_widths.md)
  : Remove display width attributes
