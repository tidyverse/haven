# haven

## Overview

Haven enables R to read and write various data formats used by other
statistical packages by wrapping the fantastic
[ReadStat](https://github.com/WizardMac/ReadStat) C library written by
[Evan Miller](https://www.evanmiller.org). Haven is part of the
[tidyverse](https://www.tidyverse.org/). Currently it supports:

- **SAS**:
  [`read_sas()`](https://haven.tidyverse.org/dev/reference/read_sas.md)
  reads `.sas7bdat` + `.sas7bcat` files and
  [`read_xpt()`](https://haven.tidyverse.org/dev/reference/read_xpt.md)
  reads SAS transport files (versions 5 and 8).
  [`write_xpt()`](https://haven.tidyverse.org/dev/reference/read_xpt.md)
  writes SAS transport files (versions 5 and 8).

- **SPSS**:
  [`read_sav()`](https://haven.tidyverse.org/dev/reference/read_spss.md)
  reads `.sav` files and
  [`read_por()`](https://haven.tidyverse.org/dev/reference/read_spss.md)
  reads the older `.por` files.
  [`write_sav()`](https://haven.tidyverse.org/dev/reference/read_spss.md)
  writes `.sav` files.

- **Stata**:
  [`read_dta()`](https://haven.tidyverse.org/dev/reference/read_dta.md)
  reads `.dta` files (up to version 15).
  [`write_dta()`](https://haven.tidyverse.org/dev/reference/read_dta.md)
  writes `.dta` files (versions 8-15).

The output objects:

- Are [tibbles](https://github.com/tidyverse/tibble), which have a
  better print method for very long and very wide files.

- Translate value labels into a new
  [`labelled()`](https://haven.tidyverse.org/dev/reference/labelled.md)
  class, which preserves the original semantics and can easily be
  coerced to factors with
  [`as_factor()`](https://haven.tidyverse.org/dev/reference/as_factor.md).
  Special missing values are preserved. See
  [`vignette("semantics")`](https://haven.tidyverse.org/dev/articles/semantics.md)
  for more details.

- Dates and times are converted to R date/time classes. Character
  vectors are not converted to factors.

## Installation

``` r
# The easiest way to get haven is to install the whole tidyverse:
install.packages("tidyverse")

# Alternatively, install just haven:
install.packages("haven")
```

## Usage

``` r
library(haven)

# SAS
read_sas("mtcars.sas7bdat")
write_xpt(mtcars, "mtcars.xpt")

# SPSS
read_sav("mtcars.sav")
write_sav(mtcars, "mtcars.sav")

# Stata
read_dta("mtcars.dta")
write_dta(mtcars, "mtcars.dta")
```

## Related work

- [foreign](https://cran.r-project.org/package=foreign) reads from SAS
  XPORT, SPSS, and Stata (up to version 12) files.

- [readstat13](https://cran.r-project.org/package=readstata13) reads
  from and writes to all Stata file format versions.

- [sas7bdat](https://cran.r-project.org/package=sas7bdat) reads from
  SAS7BDAT files.

## Code of Conduct

Please note that the haven project is released with a [Contributor Code
of Conduct](https://haven.tidyverse.org/CODE_OF_CONDUCT.html). By
contributing to this project, you agree to abide by its terms.
