# Haven <img src="logo.png" align="right" />

[![Travis-CI Build Status](https://travis-ci.org/tidyverse/haven.svg?branch=master)](https://travis-ci.org/tidyverse/haven)
[![Coverage Status](https://img.shields.io/codecov/c/github/tidyverse/haven/master.svg)](https://codecov.io/github/tidyverse/haven?branch=master)
[![CRAN_Status_Badge](http://www.r-pkg.org/badges/version/haven)](https://cran.r-project.org/package=haven)

## Overview

Haven allows is designed to write and read data formats used by other statistical packages by wrapping the fantastic [ReadStat](https://github.com/WizardMac/ReadStat) C library written by [Evan Miller](http://www.evanmiller.org). Currently it supports:

* __SAS__: `read_sas()` reads `.sas7bdat` + `.sas7bcat` files and `read_xpt()` 
  SAS transport files (version 5 and version 8). `write_sas()` writes 
  `.sas7bdat` files.
  
* __SPSS__: `read_sav()` reads `.sav` files and `read_por()` reads the 
  older `.por` files. `write_sav()` writes `.sav` files.
  
* __Stata__: `read_dta()` reads `.dta` files (up to version 14). 
  `write_dta()` writes `.dta` files (versions 8-14).

The output objects:

* Are [tibbles](http://github.com/hadley/tibble), which have a better print
  method for very long and very wide files.
  
* Translate value labels into a new `labelled()` class, which preserve the
  original semantics and can easily coerced to factors with `as_factor()`.
  Special missing values are preserved. See `vignette("semantics")` for 
  more details.

* Date/times are converted to R date/time classes. Character vectors are
  not converted to factors.
  
## Installation

```R
# The easiest way to get haven is to install the whole tidyverse:
install.packages("tidyverse")

# Alternatively, install just haven:
install.packages("haven")

# Or the the development version from GitHub:
# install.packages("devtools")
devtools::install_github("tidyverse/haven")
```

## Usage

```R
library(haven)

# SAS
write_sas(mtcars, "mtcars.sas7bdat")
read_sas("mtcars.sas7bdat")

# Stata
write_dta(mtcars, "mtcars.dta")
read_dta("mtcars.dta")

# SPSS
write_sav(mtcars, "mtcars.sav")
read_sav("mtcars.sav")
```
