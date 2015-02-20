# Haven

[![Travis-CI Build Status](https://travis-ci.org/hadley/haven.png?branch=master)](https://travis-ci.org/hadley/haven)

Haven allows you to load foreign data formats (SAS, Spss and Stata) in to R by wrapping the fantastic [ReadStat](https://github.com/WizardMac/ReadStat) C library written by [Evan Miller](http://www.evanmiller.org). Haven offers similar functionality to the base foreign package but:

* Can read SAS's binary format (SAS7BDAT). There one other package on CRAN that 
  does that, [sas7bdat](http://cran.r-project.org/web/packages/sas7bdat), but 
  it's implemented in R (not C) so is rather slow compared to haven. Haven
  should also support a wider range of SAS files, and works with SAS7BCAT files.

* It can be faster. Some spss files seem to load about 4x faster, but 
  others load slower. If you have a lot of SPSS files to import, you might
  want to benchmark both and pick the fastest.

* Works with Stata 13 files (foreign only works up to Stata 12).

* Can also write SPSS and Stata files (This is hard to test so if you
  run into any problems, please let me know).

* Can only read the data from the most common statistical packages (SAS, 
  Stata and SPSS).

* You always get a data frame, and labelled vectors are returned as new
  `labelled` class. You can easily coerce to factors or replace labelled values
  with missings as appropriate. If you also use dplyr, you'll notice that
  large data frames are printed in a convenient way.

* Uses underscores instead of dots ;)

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
