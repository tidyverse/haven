# Haven

[![Travis-CI Build Status](https://travis-ci.org/hadley/haven.svg?branch=master)](https://travis-ci.org/hadley/haven)
[![Coverage Status](https://img.shields.io/codecov/c/github/hadley/haven/master.svg)](https://codecov.io/github/hadley/haven?branch=master)
[![CRAN_Status_Badge](http://www.r-pkg.org/badges/version/haven)](http://cran.r-project.org/web/packages/haven)

Haven allows you to load foreign data formats (SAS, Spss and Stata) in to R by wrapping the fantastic [ReadStat](https://github.com/WizardMac/ReadStat) C library written by [Evan Miller](http://www.evanmiller.org). Haven offers similar functionality to the base foreign package but:

* Can read SAS's proprietary binary format (SAS7BDAT). The one other package on
  CRAN that does that, [sas7bdat](http://cran.r-project.org/web/packages/sas7bdat),
  was created to document the reverse-engineering effort. Thus its implementation
  is designed for experimentation, rather than efficiency. Haven is significantly
  faster and should also support a wider range of SAS files (including 
  compressed), and works with SAS7BCAT files.

* It can be faster. Some spss files seem to load about 4x faster, but 
  others load slower. If you have a lot of SPSS files to import, you might
  want to benchmark both and pick the fastest.

* Works with Stata 13 and 14 files (foreign only works up to Stata 12).

* Can also write SPSS and Stata files (This is hard to test so if you
  run into any problems, please let me know).

* Can only read the data from the most common statistical packages (SAS, 
  Stata and SPSS).

* You always get a data frame, date times are converted to corresponding
  R classes and labelled vectors are returned as new `labelled` class. You can 
  easily coerce to factors or replace labelled values with missings as 
  appropriate. If you also use dplyr, you'll notice that large data frames are 
  printed in a convenient way. 

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

## Updating readstat

If you're working on the development version of haven, and you'd like to update the embedded ReadStat library, you can run the following code. It is not necessary if you're just using the package.

```R
tmp <- tempfile()
download.file("https://github.com/WizardMac/ReadStat/archive/master.zip", tmp, 
  method = "wget")
unzip(tmp, exdir = tempdir())

src <- dir(file.path(tempdir(), "ReadStat-master", "src"), "\\.[ch]$", full.name = TRUE)
file.copy(src, "src/", overwrite = TRUE)
unlink(c("src/readstat_rdata.c", "src/readstat_rdata.h"))
```
