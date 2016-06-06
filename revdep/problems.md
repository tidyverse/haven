# Setup

## Platform

|setting  |value                        |
|:--------|:----------------------------|
|version  |R version 3.3.0 (2016-05-03) |
|system   |x86_64, darwin13.4.0         |
|ui       |X11                          |
|language |(EN)                         |
|collate  |en_US.UTF-8                  |
|tz       |America/Chicago              |
|date     |2016-06-06                   |

## Packages

|package   |*  |version    |date       |source                       |
|:---------|:--|:----------|:----------|:----------------------------|
|BH        |   |1.60.0-2   |2016-05-07 |CRAN (R 3.3.0)               |
|covr      |   |2.0.1      |2016-04-06 |CRAN (R 3.3.0)               |
|haven     |   |0.2.0.9000 |2016-06-06 |local (hadley/haven@dfd525c) |
|knitr     |   |1.13       |2016-05-09 |CRAN (R 3.3.0)               |
|Rcpp      |   |0.12.5     |2016-05-14 |CRAN (R 3.3.0)               |
|readr     |   |0.2.2      |2015-10-22 |CRAN (R 3.3.0)               |
|rmarkdown |   |0.9.6      |2016-05-01 |CRAN (R 3.3.0)               |
|testthat  |*  |1.0.2      |2016-04-23 |CRAN (R 3.3.0)               |
|tibble    |   |1.0        |2016-03-23 |CRAN (R 3.3.0)               |

# Check results
2 packages with problems

## ImportExport (1.1)
Maintainer: Isaac Subirana <isubirana@imim.es>

1 error  | 0 warnings | 0 notes

```
checking package dependencies ... ERROR
Package required but not available: ‘RODBC’

Packages suggested but not available for checking:
  ‘shinythemes’ ‘compareGroups’

See section ‘The DESCRIPTION file’ in the ‘Writing R Extensions’
manual.
```

## rio (0.4.6)
Maintainer: Thomas J. Leeper <thosjleeper@gmail.com>

1 error  | 0 warnings | 0 notes

```
checking whether package ‘rio’ can be installed ... ERROR
Installation failed.
See ‘/Users/hadley/Documents/ingest/haven/revdep/checks/rio.Rcheck/00install.out’ for details.
```

