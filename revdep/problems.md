# BEACH

Version: 1.2.1

## In both

*   checking package dependencies ... ERROR
    ```
    Package required but not available: ‘devtools’
    
    See section ‘The DESCRIPTION file’ in the ‘Writing R Extensions’
    manual.
    ```

# Blaunet

Version: 2.0.8

## In both

*   checking package dependencies ... ERROR
    ```
    Packages required but not available: ‘gWidgetsRGtk2’ ‘RGtk2’
    
    See section ‘The DESCRIPTION file’ in the ‘Writing R Extensions’
    manual.
    ```

# bpnreg

Version: 1.0.0

## In both

*   checking whether package ‘bpnreg’ can be installed ... ERROR
    ```
    Installation failed.
    See ‘/Users/hadley/Documents/tidyverse/haven/revdep/checks.noindex/bpnreg/new/bpnreg.Rcheck/00install.out’ for details.
    ```

## Installation

### Devel

```
* installing *source* package ‘bpnreg’ ...
** package ‘bpnreg’ successfully unpacked and MD5 sums checked
** libs
clang++ -std=gnu++11 -I"/Library/Frameworks/R.framework/Resources/include" -DNDEBUG  -I"/Users/hadley/Documents/tidyverse/haven/revdep/library.noindex/bpnreg/Rcpp/include" -I"/Users/hadley/Documents/tidyverse/haven/revdep/library.noindex/bpnreg/RcppArmadillo/include" -I"/Users/hadley/Documents/tidyverse/haven/revdep/library.noindex/haven/new/BH/include" -I/usr/local/include  -fopenmp -fPIC  -Wall -g -O2 -c RcppExports.cpp -o RcppExports.o
clang: error: unsupported option '-fopenmp'
make: *** [RcppExports.o] Error 1
ERROR: compilation failed for package ‘bpnreg’
* removing ‘/Users/hadley/Documents/tidyverse/haven/revdep/checks.noindex/bpnreg/new/bpnreg.Rcheck/bpnreg’

```
### CRAN

```
* installing *source* package ‘bpnreg’ ...
** package ‘bpnreg’ successfully unpacked and MD5 sums checked
** libs
clang++ -std=gnu++11 -I"/Library/Frameworks/R.framework/Resources/include" -DNDEBUG  -I"/Users/hadley/Documents/tidyverse/haven/revdep/library.noindex/bpnreg/Rcpp/include" -I"/Users/hadley/Documents/tidyverse/haven/revdep/library.noindex/bpnreg/RcppArmadillo/include" -I"/Users/hadley/Documents/tidyverse/haven/revdep/library.noindex/haven/old/BH/include" -I/usr/local/include  -fopenmp -fPIC  -Wall -g -O2 -c RcppExports.cpp -o RcppExports.o
clang: error: unsupported option '-fopenmp'
make: *** [RcppExports.o] Error 1
ERROR: compilation failed for package ‘bpnreg’
* removing ‘/Users/hadley/Documents/tidyverse/haven/revdep/checks.noindex/bpnreg/old/bpnreg.Rcheck/bpnreg’

```
# codebook

Version: 0.7.6

## In both

*   checking dependencies in R code ... NOTE
    ```
    Namespaces in Imports field not imported from:
      ‘graphics’ ‘pander’
      All declared Imports should be used.
    ```

*   checking Rd cross-references ... NOTE
    ```
    Package unavailable to check Rd xrefs: ‘mice’
    ```

*   checking data for non-ASCII characters ... NOTE
    ```
      Note: found 65 marked UTF-8 strings
    ```

# crosswalkr

Version: 0.2.2

## In both

*   checking dependencies in R code ... NOTE
    ```
    Namespace in Imports field not imported from: ‘dplyr’
      All declared Imports should be used.
    ```

# crunch

Version: 1.25.0

## In both

*   checking package dependencies ... NOTE
    ```
    Package suggested but not available for checking: ‘devtools’
    ```

*   checking installed package size ... NOTE
    ```
      installed size is  6.9Mb
      sub-directories of 1Mb or more:
        R               4.1Mb
        app.crunch.io   1.0Mb
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

# EdSurvey

Version: 2.2.2

## In both

*   checking installed package size ... NOTE
    ```
      installed size is  5.7Mb
      sub-directories of 1Mb or more:
        R   4.0Mb
    ```

# ess

Version: 0.1.1

## In both

*   checking examples ... ERROR
    ```
    Running examples in ‘ess-Ex.R’ failed
    The error most likely occurred in:
    
    > ### Name: show_countries
    > ### Title: Return available countries in the European Social Survey
    > ### Aliases: show_countries
    > 
    > ### ** Examples
    > 
    > 
    > show_countries()
    Error in safe_GET(paste0(ess_website, module_index)) : 
      Not Found (HTTP 404).
    Calls: show_countries ... show_any -> table_to_list -> safe_GET -> <Anonymous>
    Execution halted
    ```

*   checking tests ...
    ```
     ERROR
    Running the tests in ‘tests/testthat.R’ failed.
    Last 13 lines of output:
      ══ testthat results  ═══════════════════════════════════════════════════════════════════════════════════════
      OK: 2 SKIPPED: 23 FAILED: 14
      1. Error: show_rounds returns correct output (@test-show_.R#4) 
      2. Failure: show_rounds_country returns error when arguments are wrong (@test-show_.R#18) 
      3. Failure: show_rounds_country returns error when arguments are wrong (@test-show_.R#21) 
      4. Error: show_rounds_country returns correct output (@test-show_.R#27) 
      5. Error: show_rounds_country returns non-duplicate rounds (@test-show_.R#36) 
      6. Error: show_rounds_country returns correct countries always (@test-show_.R#56) 
      7. Error: show_countries returns correct output (@test-show_.R#65) 
      8. Failure: show_country_rounds returns error when wrong country as argument (@test-show_.R#76) 
      9. Error: show_country_rounds returns correct output (@test-show_.R#82) 
      1. ...
      
      Error: testthat unit tests failed
      Execution halted
    ```

*   checking re-building of vignette outputs ... WARNING
    ```
    Error in re-building vignettes:
      ...
    Quitting from lines 53-54 (intro_ess.Rmd) 
    Error: processing vignette 'intro_ess.Rmd' failed with diagnostics:
    Not Found (HTTP 404).
    Execution halted
    ```

*   checking dependencies in R code ... NOTE
    ```
    Namespace in Imports field not imported from: ‘tibble’
      All declared Imports should be used.
    ```

# essurvey

Version: 1.0.2

## Newly broken

*   checking tests ...
    ```
     ERROR
    Running the tests in ‘tests/testthat.R’ failed.
    Last 13 lines of output:
      > library(testthat)
      > library(essurvey)
      > 
      > test_check("essurvey")
      [31m──[39m [31m1. Failure: show_country_rounds returns error when wrong country as argument (@test-show_.R#76) [39m [31m───[39m
      `show_country_rounds("whatever")` threw an error with unexpected message.
      Expected match: "Country not available in ESS. Check show_countries()"
      Actual message: "Gateway Timeout (HTTP 504)."
      
      ══ testthat results  ══════════════════════════════════════════════════════════════════════════════════
      OK: 30 SKIPPED: 25 FAILED: 1
      1. Failure: show_country_rounds returns error when wrong country as argument (@test-show_.R#76) 
      
      Error: testthat unit tests failed
      Execution halted
    ```

# ezpickr

Version: 1.0.0

## In both

*   checking Rd cross-references ... NOTE
    ```
    Package unavailable to check Rd xrefs: ‘BrailleR’
    ```

# HMP16SData

Version: 1.2.0

## In both

*   checking re-building of vignette outputs ... WARNING
    ```
    ...
    Attaching package: 'dendextend'
    
    The following object is masked from 'package:stats':
    
        cutree
    
    ========================================
    circlize version 0.4.5
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
    Packages suggested but not available for checking:
      ‘curatedMetagenomicData’ ‘devtools’
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
    See ‘/Users/hadley/Documents/tidyverse/haven/revdep/checks.noindex/ImportExport/new/ImportExport.Rcheck/00install.out’ for details.
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
  error: unable to load shared object '/Users/hadley/Documents/tidyverse/haven/revdep/library.noindex/ImportExport/rJava/libs/rJava.so':
  dlopen(/Users/hadley/Documents/tidyverse/haven/revdep/library.noindex/ImportExport/rJava/libs/rJava.so, 6): Library not loaded: /Library/Java/JavaVirtualMachines/jdk-9.jdk/Contents/Home/lib/server/libjvm.dylib
  Referenced from: /Users/hadley/Documents/tidyverse/haven/revdep/library.noindex/ImportExport/rJava/libs/rJava.so
  Reason: image not found
Error : package ‘xlsx’ could not be loaded
ERROR: lazy loading failed for package ‘ImportExport’
* removing ‘/Users/hadley/Documents/tidyverse/haven/revdep/checks.noindex/ImportExport/new/ImportExport.Rcheck/ImportExport’

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
  error: unable to load shared object '/Users/hadley/Documents/tidyverse/haven/revdep/library.noindex/ImportExport/rJava/libs/rJava.so':
  dlopen(/Users/hadley/Documents/tidyverse/haven/revdep/library.noindex/ImportExport/rJava/libs/rJava.so, 6): Library not loaded: /Library/Java/JavaVirtualMachines/jdk-9.jdk/Contents/Home/lib/server/libjvm.dylib
  Referenced from: /Users/hadley/Documents/tidyverse/haven/revdep/library.noindex/ImportExport/rJava/libs/rJava.so
  Reason: image not found
Error : package ‘xlsx’ could not be loaded
ERROR: lazy loading failed for package ‘ImportExport’
* removing ‘/Users/hadley/Documents/tidyverse/haven/revdep/checks.noindex/ImportExport/old/ImportExport.Rcheck/ImportExport’

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

Version: 2.0.2

## In both

*   checking package dependencies ... NOTE
    ```
    Package which this enhances but not available for checking: ‘memisc’
    ```

*   checking Rd cross-references ... NOTE
    ```
    Package unavailable to check Rd xrefs: ‘memisc’
    ```

# manifestoR

Version: 1.3.0

## In both

*   checking package dependencies ... NOTE
    ```
    Package suggested but not available for checking: ‘devtools’
    ```

*   checking R code for possible problems ... NOTE
    ```
    mp_corpus: no visible binding for global variable ‘annotations’
      (/Users/hadley/Documents/tidyverse/haven/revdep/checks.noindex/manifestoR/new/manifestoR.Rcheck/00_pkg_src/manifestoR/R/manifesto.R:456-457)
    print.ManifestoAvailability: no visible binding for global variable
      ‘annotations’
      (/Users/hadley/Documents/tidyverse/haven/revdep/checks.noindex/manifestoR/new/manifestoR.Rcheck/00_pkg_src/manifestoR/R/manifesto.R:371-374)
    Undefined global functions or variables:
      annotations
    ```

# memapp

Version: 2.12

## In both

*   checking dependencies in R code ... NOTE
    ```
    Namespaces in Imports field not imported from:
      ‘DT’ ‘RColorBrewer’ ‘RODBC’ ‘dplyr’ ‘foreign’ ‘formattable’ ‘ggplot2’
      ‘haven’ ‘mem’ ‘openxlsx’ ‘plotly’ ‘readxl’ ‘shinyBS’ ‘shinydashboard’
      ‘shinydashboardPlus’ ‘shinyjs’ ‘shinythemes’ ‘stringi’ ‘stringr’
      ‘tidyr’
      All declared Imports should be used.
    ```

# netCoin

Version: 0.3.2

## In both

*   checking data for non-ASCII characters ... NOTE
    ```
      Note: found 4 marked UTF-8 strings
    ```

# opencpu

Version: 2.1

## In both

*   checking data for non-ASCII characters ... NOTE
    ```
      Note: found 4 marked UTF-8 strings
    ```

# pubh

Version: 0.4.3

## In both

*   checking dependencies in R code ... NOTE
    ```
    Namespaces in Imports field not imported from:
      ‘bookdown’ ‘desctable’ ‘effects’ ‘knitr’ ‘latex2exp’ ‘lme4’ ‘nlme’
      ‘nnet’ ‘ordinal’ ‘pander’ ‘papeR’
      All declared Imports should be used.
    ```

# readit

Version: 1.0.0

## In both

*   checking package dependencies ... NOTE
    ```
    Package suggested but not available for checking: ‘devtools’
    ```

# Rilostat

Version: 0.2.1

## In both

*   checking package dependencies ... NOTE
    ```
    Package suggested but not available for checking: ‘devtools’
    ```

# sdcMicro

Version: 5.3.0

## In both

*   checking package dependencies ... ERROR
    ```
    Packages required but not available:
      ‘car’ ‘carData’ ‘rmarkdown’ ‘knitr’ ‘data.table’ ‘xtable’
      ‘robustbase’ ‘e1071’ ‘sets’ ‘ggplot2’ ‘shiny’ ‘rhandsontable’ ‘DT’
      ‘shinyBS’ ‘prettydoc’ ‘VIM’
    
    Packages suggested but not available for checking: ‘laeken’ ‘testthat’
    
    VignetteBuilder package required for checking but not installed: ‘knitr’
    
    Files named as vignettes but with no recognized vignette engine:
       ‘inst/doc/sdc_guidelines.rnw’
    (Is a VignetteBuilder field missing?)
    
    See section ‘The DESCRIPTION file’ in the ‘Writing R Extensions’
    manual.
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

Version: 2.0.2

## In both

*   checking data for non-ASCII characters ... NOTE
    ```
      Note: found 268 marked UTF-8 strings
    ```

