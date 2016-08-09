# Haven

[![Travis-CI Build Status](https://travis-ci.org/hadley/haven.svg?branch=master)](https://travis-ci.org/hadley/haven)
[![Coverage Status](https://img.shields.io/codecov/c/github/hadley/haven/master.svg)](https://codecov.io/github/hadley/haven?branch=master)
[![CRAN_Status_Badge](http://www.r-pkg.org/badges/version/haven)](http://cran.r-project.org/web/packages/haven)

Haven allows you to load foreign data formats (SAS, SPSS and Stata) in to R by wrapping the fantastic [ReadStat](https://github.com/WizardMac/ReadStat) C library written by [Evan Miller](http://www.evanmiller.org). Haven offers similar functionality to the base foreign package but:

* It reads SPSS files (`.dta` and `.por`), reads Stata 13 and 14 files 
  (foreign only works up to Stata 12), and SAS's proprietary binary format 
  (SAS7BDAT + SAS7BCAT). It does not support many of the now more exotic 
  formats supported by foreign.

* Can also write SPSS, Stata, and SAS files.

* Date times are converted to corresponding R classes and labelled vectors are 
  returned as a new `labelled` class. You can easily coerce to factors or 
  replace labelled values with missings as appropriate. All functions return
  [tibbles](http://github.com/hadley/tibble).

* Uses underscores instead of dots ;)

Haven is still a work in progress so please [file an issue](https://github.com/hadley/haven/issues) if it fails to correctly load a file that you're interested in.

## Installation

```R
# Install the released version from CRAN:
install.packages("haven")

# Install the cutting edge development version from GitHub:
# install.packages("devtools")
devtools::install_github("hadley/haven")
```

## Usage

* SAS: `read_sas("path/to/file")`
* SPSS: `read_sav("path/to/file")`
* Stata: `read_dta("path/to/file")`
