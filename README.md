# Haven <img src="logo.png" align="right" />

[![Travis-CI Build Status](https://travis-ci.org/tidyverse/haven.svg?branch=master)](https://travis-ci.org/tidyverse/haven)
[![Coverage Status](https://img.shields.io/codecov/c/github/tidyverse/haven/master.svg)](https://codecov.io/github/tidyverse/haven?branch=master)
[![CRAN_Status_Badge](http://www.r-pkg.org/badges/version/haven)](https://cran.r-project.org/package=haven)

Haven allows you to load foreign data formats (SAS, SPSS and Stata) in to R by wrapping the fantastic [ReadStat](https://github.com/WizardMac/ReadStat) C library written by [Evan Miller](http://www.evanmiller.org). 

Key features:

* Reads both types of SPSS files (`.dta` and `.por`), Stata files 
  (up to version 14), and SAS's proprietary binary format 
  (SAS7BDAT + SAS7BCAT).

* Writes SPSS, Stata, and SAS files.

* Date times are converted to corresponding R classes and labelled vectors are 
  returned as a new `labelled` class. You can easily coerce to factors or 
  replace labelled values with missings as appropriate. All functions return
  [tibbles](http://github.com/hadley/tibble).

## Installation

```R
# Install the released version from CRAN:
install.packages("haven")

# Install the cutting edge development version from GitHub:
# install.packages("devtools")
devtools::install_github("hadley/haven")
```

## Usage

```R
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
