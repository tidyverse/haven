# Haven

[![Travis-CI Build Status](https://travis-ci.org/hadley/haven.png?branch=master)](https://travis-ci.org/hadley/haven)

Haven allows you to load foreign data formats (SAS, Spss and Stata) in to R by wrapping the [ReadStat](https://github.com/WizardMac/ReadStat) C library. 

The main driver of haven was to make it easy to read SAS's binary format without SAS installed. There is only one other package on CRAN that does that, [sas7bdat](http://cran.r-project.org/web/packages/sas7bdat), but it's implemented in R (not C) so is rather slow compared to haven. Once the SAS binding was complete, it only took a few minutes to add support for Stata and SPSS. You can also load these file types with the foreign package.

## Installation

Haven is not yet available on CRAN. You can install the development version directly from github with:

```R
# install.packages("devtools")
devtools::install_github("hadley/haven")
```

## Usage

* SAS: `read_sas("path/to/file")`
* SPSS: `read_por("path/to/file")`, `read_sav("path/to/file")`
* Stata: `read_dta("path/to/file")`

## Updating readstat

To get the latest version of readstat into the repo, run this code:

```R
tmp <- tempfile()
download.file("https://github.com/WizardMac/ReadStat/archive/master.zip", tmp, 
  method = "wget")
unzip(tmp, exdir = tempdir())

src <- dir(file.path(tempdir(), "ReadStat-master", "src"), "\\.[ch]$", full.name = TRUE)
file.copy(src, "src/", overwrite = TRUE)
unlink(c("src/readstat_rdata.c", "src/readstat_rdata.h"))
```
