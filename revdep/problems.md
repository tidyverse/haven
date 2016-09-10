# Setup

## Platform

|setting  |value                        |
|:--------|:----------------------------|
|version  |R version 3.3.1 (2016-06-21) |
|system   |x86_64, darwin13.4.0         |
|ui       |RStudio (1.0.7)              |
|language |(EN)                         |
|collate  |en_US.UTF-8                  |
|tz       |America/New_York             |
|date     |2016-09-10                   |

## Packages

|package   |*  |version    |date       |source                           |
|:---------|:--|:----------|:----------|:--------------------------------|
|BH        |   |1.60.0-2   |2016-05-07 |CRAN (R 3.3.0)                   |
|covr      |   |2.2.1      |2016-08-10 |cran (@2.2.1)                    |
|haven     |*  |1.0.0      |2016-09-10 |local (hadley/haven@NA)          |
|hms       |   |0.2-1      |2016-09-10 |Github (rstats-db/hms@80b2bfc)   |
|knitr     |   |1.14       |2016-08-13 |cran (@1.14)                     |
|Rcpp      |   |0.12.7     |2016-09-05 |cran (@0.12.7)                   |
|readr     |   |1.0.0      |2016-08-03 |cran (@1.0.0)                    |
|rmarkdown |   |1.0        |2016-07-08 |cran (@1.0)                      |
|testthat  |*  |1.0.2.9000 |2016-07-19 |Github (hadley/testthat@46d15da) |
|tibble    |   |1.2-12     |2016-09-10 |Github (hadley/tibble@6d2bb08)   |

# Check results
3 packages with problems

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
.import.rio_sas7bdat: possible error in read_sas(b7dat = file, ...):
  unused argument (b7dat = file)
.import.rio_sav: possible error in read_sav(path = file): unused
  argument (path = file)
```

## tidyverse (1.0.0)
Maintainer: Hadley Wickham <hadley@rstudio.com>  
Bug reports: https://github.com/hadley/tidyverse/issues

1 error  | 0 warnings | 0 notes

```
checking package dependencies ... ERROR
Packages required but not available: ‘forcats’ ‘modelr’

See section ‘The DESCRIPTION file’ in the ‘Writing R Extensions’
manual.
```

