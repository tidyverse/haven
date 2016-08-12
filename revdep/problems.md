# Setup

## Platform

|setting  |value                        |
|:--------|:----------------------------|
|version  |R version 3.3.0 (2016-05-03) |
|system   |x86_64, darwin13.4.0         |
|ui       |RStudio (0.99.1283)          |
|language |(EN)                         |
|collate  |en_US.UTF-8                  |
|tz       |America/Chicago              |
|date     |2016-08-12                   |

## Packages

|package   |*  |version    |date       |source                           |
|:---------|:--|:----------|:----------|:--------------------------------|
|BH        |   |1.60.0-2   |2016-05-07 |CRAN (R 3.3.0)                   |
|covr      |   |2.2.1      |2016-08-10 |cran (@2.2.1)                    |
|haven     |   |0.2.1.9000 |2016-08-12 |local (hadley/haven@NA)          |
|hms       |   |0.2-1      |2016-07-19 |Github (rstats-db/hms@98d14cf)   |
|knitr     |   |1.13       |2016-05-09 |CRAN (R 3.3.0)                   |
|Rcpp      |   |0.12.6     |2016-07-19 |cran (@0.12.6)                   |
|readr     |   |1.0.0      |2016-08-03 |cran (@1.0.0)                    |
|rmarkdown |   |1.0        |2016-07-08 |cran (@1.0)                      |
|testthat  |*  |1.0.2.9000 |2016-07-19 |Github (hadley/testthat@46d15da) |
|tibble    |   |1.1-5      |2016-08-10 |Github (hadley/tibble@dbe66cf)   |

# Check results
2 packages with problems

## ImportExport (1.1)
Maintainer: Isaac Subirana <isubirana@imim.es>

1 error  | 0 warnings | 0 notes

```
checking package dependencies ... ERROR
Package required but not available: ‘RODBC’

Package suggested but not available for checking: ‘compareGroups’

See section ‘The DESCRIPTION file’ in the ‘Writing R Extensions’
manual.
```

## rio (0.4.12)
Maintainer: Thomas J. Leeper <thosjleeper@gmail.com>

2 errors | 1 warning  | 1 note 

```
checking examples ... ERROR
Running examples in ‘rio-Ex.R’ failed
The error most likely occurred in:

> base::assign(".ptime", proc.time(), pos = "CheckExEnv")
> ### Name: convert
> ### Title: Convert from one file format to another
> ### Aliases: convert
> 
> ### ** Examples
> 
> # create a file to convert
> export(iris, "iris.dta")
Error: The following variable names are not valid Stata variables: `Sepal.Length`, `Sepal.Width`, `Petal.Length`, `Petal.Width`
Execution halted

checking tests ... ERROR
Running the tests in ‘tests/test-all.R’ failed.
Last 13 lines of output:
  3: as.expectation(exp, ..., srcref = srcref) at /private/tmp/RtmpQjBlGo/devtoolscfb138fb2e0/hadley-testthat-46d15da/R/expectation.R:33
  4: identical(as.vector(object), TRUE) at /private/tmp/RtmpQjBlGo/devtoolscfb138fb2e0/hadley-testthat-46d15da/R/expectation.R:112
  5: as.vector(object)
  6: is.data.frame(import("http://www.stata-press.com/data/r13/auto.dta"))
  7: import("http://www.stata-press.com/data/r13/auto.dta")
  8: .import(file = file, ...)
  9: .import.rio_dta(file = file, ...)
  10: convert_attributes(read_dta(path = file))
  11: read_dta(path = file)
  
  DONE ===========================================================================
  Error: Test failures
  Execution halted

checking re-building of vignette outputs ... WARNING
Error in re-building vignettes:
  ...
Quitting from lines 56-62 (rio.Rmd) 
Error: processing vignette 'rio.Rmd' failed with diagnostics:
The following variable names are not valid Stata variables: `Sepal.Length`, `Sepal.Width`, `Petal.Length`, `Petal.Width`
Execution halted


checking R code for possible problems ... NOTE
.import.rio_dta: possible error in read_dta(path = file): unused
  argument (path = file)
.import.rio_sav: possible error in read_sav(path = file): unused
  argument (path = file)
```

