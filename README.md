# Haven

[![Travis-CI Build Status](https://travis-ci.org/hadley/haven.png?branch=master)](https://travis-ci.org/hadley/haven)

Haven allows you to load foreign data formats (SAS, Spss and Stata) in to R by wrapping the [ReadStat](https://github.com/WizardMac/ReadStat) C library. Haven offers similar functionaity to the base foreign package but:

* Can read SAS's binary format (SAS7BDAT). There one other package on CRAN that 
  does that, [sas7bdat](http://cran.r-project.org/web/packages/sas7bdat), but 
  it's implemented in R (not C) so is rather slow compared to haven.

* Is considerably faster. For example, casual benchmarks suggest that 
  `haven::read_spss()` is about 4x faster than `foregin::read.spss()`.

* Works with Stata 13 files (foreign only works up to Stata 12).

* Can only read the data from the most common statistical packages (SAS, 
  Stata and SPSS).

* Uses underscores ;)

Haven is still a work in progress so please [file an issue](https://github.com/hadley/haven/issues) if it fails to correctly load a file that you're interested in.

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
