# BEACH

Version: 1.2.1

## In both

*   checking whether package ‘BEACH’ can be installed ... ERROR
    ```
    Installation failed.
    See ‘/Users/hadley/Documents/ingest/haven/revdep/checks.noindex/BEACH/new/BEACH.Rcheck/00install.out’ for details.
    ```

## Installation

### Devel

```
* installing *source* package ‘BEACH’ ...
** package ‘BEACH’ successfully unpacked and MD5 sums checked
** R
** inst
** byte-compile and prepare package for lazy loading
Error : .onLoad failed in loadNamespace() for 'rJava', details:
  call: dyn.load(file, DLLpath = DLLpath, ...)
  error: unable to load shared object '/Users/hadley/Documents/ingest/haven/revdep/library.noindex/BEACH/rJava/libs/rJava.so':
  dlopen(/Users/hadley/Documents/ingest/haven/revdep/library.noindex/BEACH/rJava/libs/rJava.so, 6): Library not loaded: /Library/Java/JavaVirtualMachines/jdk-9.jdk/Contents/Home/lib/server/libjvm.dylib
  Referenced from: /Users/hadley/Documents/ingest/haven/revdep/library.noindex/BEACH/rJava/libs/rJava.so
  Reason: image not found
ERROR: lazy loading failed for package ‘BEACH’
* removing ‘/Users/hadley/Documents/ingest/haven/revdep/checks.noindex/BEACH/new/BEACH.Rcheck/BEACH’

```
### CRAN

```
* installing *source* package ‘BEACH’ ...
** package ‘BEACH’ successfully unpacked and MD5 sums checked
** R
** inst
** byte-compile and prepare package for lazy loading
Error : .onLoad failed in loadNamespace() for 'rJava', details:
  call: dyn.load(file, DLLpath = DLLpath, ...)
  error: unable to load shared object '/Users/hadley/Documents/ingest/haven/revdep/library.noindex/BEACH/rJava/libs/rJava.so':
  dlopen(/Users/hadley/Documents/ingest/haven/revdep/library.noindex/BEACH/rJava/libs/rJava.so, 6): Library not loaded: /Library/Java/JavaVirtualMachines/jdk-9.jdk/Contents/Home/lib/server/libjvm.dylib
  Referenced from: /Users/hadley/Documents/ingest/haven/revdep/library.noindex/BEACH/rJava/libs/rJava.so
  Reason: image not found
ERROR: lazy loading failed for package ‘BEACH’
* removing ‘/Users/hadley/Documents/ingest/haven/revdep/checks.noindex/BEACH/old/BEACH.Rcheck/BEACH’

```
# Blaunet

Version: 2.0.8

## In both

*   checking package dependencies ... ERROR
    ```
    Package required but not available: ‘gWidgetsRGtk2’
    
    See section ‘The DESCRIPTION file’ in the ‘Writing R Extensions’
    manual.
    ```

# bpnreg

Version: 1.0.0

## In both

*   checking whether package ‘bpnreg’ can be installed ... ERROR
    ```
    Installation failed.
    See ‘/Users/hadley/Documents/ingest/haven/revdep/checks.noindex/bpnreg/new/bpnreg.Rcheck/00install.out’ for details.
    ```

## Installation

### Devel

```
* installing *source* package ‘bpnreg’ ...
** package ‘bpnreg’ successfully unpacked and MD5 sums checked
** libs
clang++ -std=gnu++11 -I"/Library/Frameworks/R.framework/Resources/include" -DNDEBUG  -I"/Users/hadley/Documents/ingest/haven/revdep/library.noindex/haven/new/Rcpp/include" -I"/Users/hadley/Documents/ingest/haven/revdep/library.noindex/bpnreg/RcppArmadillo/include" -I"/Users/hadley/Documents/ingest/haven/revdep/library.noindex/haven/new/BH/include" -I/usr/local/include  -fopenmp -fPIC  -Wall -g -O2 -c RcppExports.cpp -o RcppExports.o
clang: error: unsupported option '-fopenmp'
make: *** [RcppExports.o] Error 1
ERROR: compilation failed for package ‘bpnreg’
* removing ‘/Users/hadley/Documents/ingest/haven/revdep/checks.noindex/bpnreg/new/bpnreg.Rcheck/bpnreg’

```
### CRAN

```
* installing *source* package ‘bpnreg’ ...
** package ‘bpnreg’ successfully unpacked and MD5 sums checked
** libs
clang++ -std=gnu++11 -I"/Library/Frameworks/R.framework/Resources/include" -DNDEBUG  -I"/Users/hadley/Documents/ingest/haven/revdep/library.noindex/haven/old/Rcpp/include" -I"/Users/hadley/Documents/ingest/haven/revdep/library.noindex/bpnreg/RcppArmadillo/include" -I"/Users/hadley/Documents/ingest/haven/revdep/library.noindex/haven/old/BH/include" -I/usr/local/include  -fopenmp -fPIC  -Wall -g -O2 -c RcppExports.cpp -o RcppExports.o
clang: error: unsupported option '-fopenmp'
make: *** [RcppExports.o] Error 1
ERROR: compilation failed for package ‘bpnreg’
* removing ‘/Users/hadley/Documents/ingest/haven/revdep/checks.noindex/bpnreg/old/bpnreg.Rcheck/bpnreg’

```
# codebook

Version: 0.6.3

## In both

*   checking examples ... ERROR
    ```
    Running examples in ‘codebook-Ex.R’ failed
    The error most likely occurred in:
    
    > ### Name: codebook
    > ### Title: Generate rmarkdown codebook
    > ### Aliases: codebook
    > 
    > ### ** Examples
    > 
    > # will generate figures in a temporary directory
    > old_base_dir <- knitr::opts_knit$get("base.dir")
    > knitr::opts_knit$set(base.dir = tempdir())
    > on.exit(knitr::opts_knit$set(base.dir = old_base_dir))
    > data("bfi")
    > bfi <- bfi[, c("BFIK_open_1", "BFIK_open_1")]
    > md <- codebook(bfi, survey_repetition = "single", metadata_table = FALSE)
    No missings.
    Error: Can't convert an environment to function
    [90mCall `rlang::last_error()` to see a backtrace[39m
    Execution halted
    ```

*   checking dependencies in R code ... NOTE
    ```
    Namespaces in Imports field not imported from:
      ‘graphics’ ‘pander’
      All declared Imports should be used.
    ```

*   checking data for non-ASCII characters ... NOTE
    ```
      Note: found 65 marked UTF-8 strings
    ```

# crosswalkr

Version: 0.1.1

## In both

*   checking dependencies in R code ... NOTE
    ```
    Namespace in Imports field not imported from: ‘dplyr’
      All declared Imports should be used.
    ```

# crunch

Version: 1.24.0

## In both

*   checking installed package size ... NOTE
    ```
      installed size is  6.9Mb
      sub-directories of 1Mb or more:
        R     4.1Mb
        doc   1.1Mb
    ```

# datadigest

Version: 1.0.2

## In both

*   checking dependencies in R code ... NOTE
    ```
    Namespace in Imports field not imported from: ‘miniUI’
      All declared Imports should be used.
    ```

# dataMaid

Version: 1.2.0

## In both

*   checking dependencies in R code ... NOTE
    ```
    Namespace in Imports field not imported from: ‘DT’
      All declared Imports should be used.
    ```

# duawranglr

Version: 0.6.3

## In both

*   checking dependencies in R code ... NOTE
    ```
    Namespaces in Imports field not imported from:
      ‘digest’ ‘dplyr’
      All declared Imports should be used.
    ```

# ess

Version: 0.1.1

## In both

*   checking tests ...
    ```
     ERROR
    Running the tests in ‘tests/testthat.R’ failed.
    Last 13 lines of output:
      
      [31m──[39m [31m5. Failure: show_country_rounds returns correct rounds for countries (@test-show_.R#101) [39m [31m────[39m
      show_country_rounds("Spain") not equal to 1:7.
      Lengths differ: 9 is not 7
      
      ══ testthat results  ════════════════════════════════════════════════════════════════════════════
      OK: 33 SKIPPED: 23 FAILED: 5
      1. Failure: show_country_rounds returns correct rounds for countries (@test-show_.R#97) 
      2. Failure: show_country_rounds returns correct rounds for countries (@test-show_.R#98) 
      3. Failure: show_country_rounds returns correct rounds for countries (@test-show_.R#99) 
      4. Failure: show_country_rounds returns correct rounds for countries (@test-show_.R#100) 
      5. Failure: show_country_rounds returns correct rounds for countries (@test-show_.R#101) 
      
      Error: testthat unit tests failed
      Execution halted
    ```

*   checking dependencies in R code ... NOTE
    ```
    Namespace in Imports field not imported from: ‘tibble’
      All declared Imports should be used.
    ```

# HMP16SData

Version: 1.0.1

## In both

*   checking re-building of vignette outputs ... WARNING
    ```
    ...
    Attaching package: 'dendextend'
    
    The following object is masked from 'package:stats':
    
        cutree
    
    ========================================
    circlize version 0.4.4
    CRAN page: https://cran.r-project.org/package=circlize
    Github page: https://github.com/jokergoo/circlize
    Documentation: http://jokergoo.github.io/circlize_book/book/
    
    If you use it in published research, please cite:
    Gu, Z. circlize implements and enhances circular visualization 
      in R. Bioinformatics 2014.
    ========================================
    
    Quitting from lines 58-71 (HMP16SData.Rmd) 
    Error: processing vignette 'HMP16SData.Rmd' failed with diagnostics:
    there is no package called 'curatedMetagenomicData'
    Execution halted
    ```

*   checking package dependencies ... NOTE
    ```
    Package suggested but not available for checking: ‘curatedMetagenomicData’
    ```

*   checking installed package size ... NOTE
    ```
      installed size is 19.1Mb
      sub-directories of 1Mb or more:
        doc       1.5Mb
        extdata  17.4Mb
    ```

# ImportExport

Version: 1.1

## In both

*   checking whether package ‘ImportExport’ can be installed ... ERROR
    ```
    Installation failed.
    See ‘/Users/hadley/Documents/ingest/haven/revdep/checks.noindex/ImportExport/new/ImportExport.Rcheck/00install.out’ for details.
    ```

## Installation

### Devel

```
* installing *source* package ‘ImportExport’ ...
** package ‘ImportExport’ successfully unpacked and MD5 sums checked
** R
** inst
** byte-compile and prepare package for lazy loading
Error: package or namespace load failed for ‘xlsx’:
 .onLoad failed in loadNamespace() for 'rJava', details:
  call: dyn.load(file, DLLpath = DLLpath, ...)
  error: unable to load shared object '/Users/hadley/Documents/ingest/haven/revdep/library.noindex/ImportExport/rJava/libs/rJava.so':
  dlopen(/Users/hadley/Documents/ingest/haven/revdep/library.noindex/ImportExport/rJava/libs/rJava.so, 6): Library not loaded: /Library/Java/JavaVirtualMachines/jdk-9.jdk/Contents/Home/lib/server/libjvm.dylib
  Referenced from: /Users/hadley/Documents/ingest/haven/revdep/library.noindex/ImportExport/rJava/libs/rJava.so
  Reason: image not found
Error : package ‘xlsx’ could not be loaded
ERROR: lazy loading failed for package ‘ImportExport’
* removing ‘/Users/hadley/Documents/ingest/haven/revdep/checks.noindex/ImportExport/new/ImportExport.Rcheck/ImportExport’

```
### CRAN

```
* installing *source* package ‘ImportExport’ ...
** package ‘ImportExport’ successfully unpacked and MD5 sums checked
** R
** inst
** byte-compile and prepare package for lazy loading
Error: package or namespace load failed for ‘xlsx’:
 .onLoad failed in loadNamespace() for 'rJava', details:
  call: dyn.load(file, DLLpath = DLLpath, ...)
  error: unable to load shared object '/Users/hadley/Documents/ingest/haven/revdep/library.noindex/ImportExport/rJava/libs/rJava.so':
  dlopen(/Users/hadley/Documents/ingest/haven/revdep/library.noindex/ImportExport/rJava/libs/rJava.so, 6): Library not loaded: /Library/Java/JavaVirtualMachines/jdk-9.jdk/Contents/Home/lib/server/libjvm.dylib
  Referenced from: /Users/hadley/Documents/ingest/haven/revdep/library.noindex/ImportExport/rJava/libs/rJava.so
  Reason: image not found
Error : package ‘xlsx’ could not be loaded
ERROR: lazy loading failed for package ‘ImportExport’
* removing ‘/Users/hadley/Documents/ingest/haven/revdep/checks.noindex/ImportExport/old/ImportExport.Rcheck/ImportExport’

```
# ipumsr

Version: 0.3.0

## In both

*   checking dependencies in R code ... NOTE
    ```
    Namespace in Imports field not imported from: ‘R6’
      All declared Imports should be used.
    ```

# labelled

Version: 1.1.0

## Newly broken

*   checking examples ... ERROR
    ```
    Running examples in ‘labelled-Ex.R’ failed
    The error most likely occurred in:
    
    > ### Name: labelled_spss
    > ### Title: Create a labelled vector with SPSS style of missing values.
    > ### Aliases: labelled_spss
    > 
    > ### ** Examples
    > 
    > x1 <- labelled_spss(1:10, c(Good = 1, Bad = 8), na_values = c(9, 10))
    Error in labelled(x, labels, label = label) : unused argument (labels)
    Calls: labelled_spss -> structure
    Execution halted
    ```

*   checking tests ...
    ```
     ERROR
    Running the tests in ‘tests/testthat.R’ failed.
    Last 13 lines of output:
      ══ testthat results  ════════════════════════════════════════════════════════════════════════════
      OK: 18 SKIPPED: 0 FAILED: 19
      1. Error: data.frame variables are named correctly (@test-dataframes.R#4) 
      2. Failure: labelled return an object of class labelled (@test-labelled.r#6) 
      3. Failure: labelled preserves variable label (@test-labelled.r#27) 
      4. Failure: val_labels preserves variable label (@test-labelled.r#36) 
      5. Failure: val_labels preserves variable label (@test-labelled.r#39) 
      6. Failure: val_label preserves variable label (@test-labelled.r#46) 
      7. Error: val_label preserves variable label (@test-labelled.r#48) 
      8. Error: val_labels and val_label preserves spss missing values (@test-labelled.r#53) 
      9. Error: value labels can't be removed if missing values are defined (@test-labelled.r#71) 
      1. ...
      
      Error: testthat unit tests failed
      Execution halted
    ```

*   checking for code/documentation mismatches ... WARNING
    ```
    Codoc mismatches from documentation object 'labelled':
    labelled
      Code: function(x, labels, label = NULL)
      Docs: function(x, labels)
      Argument names in code not in docs:
        label
    
    Codoc mismatches from documentation object 'labelled_spss':
    labelled_spss
      Code: function(x, labels, na_values = NULL, na_range = NULL, label =
                     NULL)
      Docs: function(x, labels, na_values = NULL, na_range = NULL)
      Argument names in code not in docs:
        label
    ```

*   checking re-building of vignette outputs ... WARNING
    ```
    Error in re-building vignettes:
      ...
    Quitting from lines 73-77 (intro_labelled.Rmd) 
    Error: processing vignette 'intro_labelled.Rmd' failed with diagnostics:
    no applicable method for 'val_label<-' applied to an object of class "haven_labelled"
    Execution halted
    ```

*   checking R code for possible problems ... NOTE
    ```
    labelled_spss: warning in labelled(x, labels, label = label): partial
      argument match of 'label' to 'labels'
      (/private/tmp/RtmpASh5Pj/R.INSTALL126c5372c1a47/haven/R/labelled_spss.R:39-44)
    labelled_spss: possible error in labelled(x, labels, label = label):
      unused argument (labels)
      (/private/tmp/RtmpASh5Pj/R.INSTALL126c5372c1a47/haven/R/labelled_spss.R:39-44)
    ```

## In both

*   checking package dependencies ... NOTE
    ```
    Package which this enhances but not available for checking: ‘memisc’
    ```

*   checking Rd cross-references ... NOTE
    ```
    Package unavailable to check Rd xrefs: ‘memisc’
    ```

# memapp

Version: 2.10

## In both

*   checking dependencies in R code ... NOTE
    ```
    Namespaces in Imports field not imported from:
      ‘DT’ ‘RColorBrewer’ ‘RODBC’ ‘dplyr’ ‘foreign’ ‘formattable’ ‘ggplot2’
      ‘haven’ ‘magrittr’ ‘mem’ ‘openxlsx’ ‘plotly’ ‘readxl’ ‘shinyBS’
      ‘shinydashboard’ ‘shinyjs’ ‘shinythemes’ ‘stringi’ ‘stringr’ ‘tidyr’
      All declared Imports should be used.
    ```

# netCoin

Version: 0.3.0

## In both

*   checking data for non-ASCII characters ... NOTE
    ```
      Note: found 4 marked UTF-8 strings
    ```

# opencpu

Version: 2.0.8

## In both

*   checking data for non-ASCII characters ... NOTE
    ```
      Note: found 4 marked UTF-8 strings
    ```

# pubh

Version: 0.4.2

## In both

*   checking dependencies in R code ... NOTE
    ```
    Namespaces in Imports field not imported from:
      ‘bookdown’ ‘desctable’ ‘effects’ ‘knitr’ ‘latex2exp’ ‘lme4’ ‘nlme’
      ‘nnet’ ‘ordinal’ ‘pander’ ‘papeR’
      All declared Imports should be used.
    ```

# simPop

Version: 1.1.1

## In both

*   checking Rd cross-references ... NOTE
    ```
    Package unavailable to check Rd xrefs: ‘sampling’
    ```

# tidyverse

Version: 1.2.1

## In both

*   checking dependencies in R code ... NOTE
    ```
    Namespaces in Imports field not imported from:
      ‘dbplyr’ ‘reprex’ ‘rlang’
      All declared Imports should be used.
    ```

# xSub

Version: 0.0.1

## In both

*   checking examples ... ERROR
    ```
    ...
    53       adm0, adm1, adm2, priogrid day*, month, week, year
    54       adm0, adm1, adm2, priogrid day*, month, week, year
    55       adm0, adm1, adm2, priogrid day*, month, week, year
    56       adm0, adm1, adm2, priogrid day*, month, week, year
    57       adm0, adm1, adm2, priogrid day*, month, week, year
    58 adm0, adm1, adm2, clea, priogrid day*, month, week, year
    59 adm0, adm1, adm2, clea, priogrid day*, month, week, year
    60       adm0, adm1, adm2, priogrid day*, month, week, year
    [1] "* daily-level data available only for adm0, adm1"
    [1] "For more info, subset by country"
    > 
    > # Download ACLED data for Egypt, at country-year level
    > my_file <- get_xSub(data_source = "ACLED",country_iso3 = "EGY",
    +            space_unit = "adm0",time_unit = "year")
    Loading required package: bitops
    Warning in download.file(file_url, temp, cacheOK = TRUE, quiet = (!verbose)) :
      cannot open URL 'http://cross-sub.org/download/file/xSub_ACLED_EGY_adm0_year.csv': HTTP status was '404 Not Found'
    Error in download.file(file_url, temp, cacheOK = TRUE, quiet = (!verbose)) : 
      cannot open URL 'http://cross-sub.org/download/file/xSub_ACLED_EGY_adm0_year.csv'
    Calls: get_xSub -> download.file
    Execution halted
    ```

*   checking data for non-ASCII characters ... NOTE
    ```
      Note: found 63 marked UTF-8 strings
    ```

