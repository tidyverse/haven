# Haven

[![Travis-CI Build Status](https://travis-ci.org/hadley/haven.png?branch=master)](https://travis-ci.org/hadley/haven)

Load foreign data formats (SAS, Spss and Stata) in to R. This is a binding to [ReadStat](https://github.com/WizardMac/ReadStat).

## Installation

Haven is not yet available on CRAN. You can install the development version directly from github with:

```R
# install.packages("devtools")
devtools::install_github("hadley/haven")
```

## Updating readstat

To get the latest version of readstat into the repo, run this code:

```R
tmp <- tempfile()
download.file("https://github.com/WizardMac/ReadStat/archive/master.zip", tmp, 
  method = "wget")
unzip(tmp, exdir = tempdir())

src <- dir(file.path(tempdir(), "ReadStat-master", "src"), "\\.[ch]$", full.name = TRUE)
file.copy(src, "src/", overwrite = TRUE)
```
