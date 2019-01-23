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

## Newly broken

*   checking examples ... ERROR
    ```
    Running examples in ‘codebook-Ex.R’ failed
    The error most likely occurred in:
    
    > ### Name: codebook_component_scale
    > ### Title: Codebook component for scales
    > ### Aliases: codebook_component_scale
    > 
    > ### ** Examples
    > 
    > # will generate figures in a temporary directory
    > old_base_dir <- knitr::opts_knit$get("base.dir")
    > knitr::opts_knit$set(base.dir = tempdir())
    > on.exit(knitr::opts_knit$set(base.dir = old_base_dir))
    > data("bfi")
    > bfi <- bfi[,c("BFIK_open", paste0("BFIK_open_", 1:4))]
    > codebook_component_scale(bfi[,1], "BFIK_open", bfi[,-1],
    +    reliabilities = list(BFIK_open = psych::alpha(bfi[,-1])))
    Error: C stack usage  7969220 is too close to the limit
    Execution halted
    ```

*   checking re-building of vignette outputs ... WARNING
    ```
    Error in re-building vignettes:
      ...
    Error: C stack usage  7970848 is too close to the limit
    Execution halted
    ```

## In both

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

## Newly broken

*   checking tests ...
    ```
     ERROR
    Running the tests in ‘tests/testthat.R’ failed.
    Last 13 lines of output:
      > test_check("crosswalkr")
      [31m──[39m [31m1. Failure: Failed to assign proper values (@test-encodefrom.R#67) [39m [31m─────────────────────────────────────────────[39m
      haven::zap_labels(vec) not equal to `act_vals`.
      Attributes: < Modes: list, NULL >
      Attributes: < Lengths: 2, 0 >
      Attributes: < names for target but not for current >
      Attributes: < current is not list-like >
      target is labelled, current is character
      
      ══ testthat results  ═══════════════════════════════════════════════════════════════════════════════════════════════
      OK: 36 SKIPPED: 0 FAILED: 1
      1. Failure: Failed to assign proper values (@test-encodefrom.R#67) 
      
      Error: testthat unit tests failed
      Execution halted
    ```

## In both

*   checking dependencies in R code ... NOTE
    ```
    Namespace in Imports field not imported from: ‘dplyr’
      All declared Imports should be used.
    ```

# crunch

Version: 1.24.0

## In both

*   checking package dependencies ... NOTE
    ```
    Package suggested but not available for checking: ‘devtools’
    ```

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

# EdSurvey

Version: 2.2.2

## In both

*   checking installed package size ... NOTE
    ```
      installed size is  5.8Mb
      sub-directories of 1Mb or more:
        R   4.1Mb
    ```

# ess

Version: 0.1.1

## In both

*   checking tests ...
    ```
     ERROR
    Running the tests in ‘tests/testthat.R’ failed.
    Last 13 lines of output:
      
      [31m──[39m [31m5. Failure: show_country_rounds returns correct rounds for countries (@test-show_.R#101) [39m [31m───────────────────────[39m
      show_country_rounds("Spain") not equal to 1:7.
      Lengths differ: 9 is not 7
      
      ══ testthat results  ═══════════════════════════════════════════════════════════════════════════════════════════════
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

Version: 1.2.0

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
    ...
    > ### Title: Get / Set SPSS missing values
    > ### Aliases: na_values na_values<- na_range na_range<- set_na_values
    > ###   set_na_range
    > 
    > ### ** Examples
    > 
    > v <- labelled(c(1,2,2,2,3,9,1,3,2,NA), c(yes = 1, no = 3, "don't know" = 9))
    > v
    <Labelled double>
     [1]  1  2  2  2  3  9  1  3  2 NA
    
    Labels:
     value      label
         1        yes
         3         no
         9 don't know
    > na_values(v) <- 9
    Error in `na_values<-.default`(`*tmp*`, value = 9) : 
      Value labels need to be defined first. Please use val_labels().
    Calls: na_values<- -> na_values<-.default
    Execution halted
    ```

*   checking tests ...
    ```
     ERROR
    Running the tests in ‘tests/testthat.R’ failed.
    Last 13 lines of output:
      ══ testthat results  ═══════════════════════════════════════════════════════════════════════════════════════════════
      OK: 20 SKIPPED: 0 FAILED: 18
      1. Failure: labelled return an object of class labelled (@test-labelled.r#6) 
      2. Failure: labelled preserves variable label (@test-labelled.r#27) 
      3. Failure: val_labels preserves variable label (@test-labelled.r#36) 
      4. Failure: val_labels preserves variable label (@test-labelled.r#39) 
      5. Failure: val_label preserves variable label (@test-labelled.r#46) 
      6. Error: val_label preserves variable label (@test-labelled.r#48) 
      7. Error: val_labels and val_label preserves spss missing values (@test-labelled.r#55) 
      8. Failure: value labels can't be removed if missing values are defined (@test-labelled.r#72) 
      9. Failure: remove_labels strips labelled attributes (@test-labelled.r#83) 
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
    Warning in engine$weave(file, quiet = quiet, encoding = enc) :
      The vignette engine knitr::rmarkdown is not available, because the rmarkdown package is not installed. Please install it.
    Quitting from lines 73-77 (intro_labelled.Rmd) 
    Error: processing vignette 'intro_labelled.Rmd' failed with diagnostics:
    no applicable method for 'val_label<-' applied to an object of class "haven_labelled"
    Execution halted
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
      (/Users/hadley/Documents/ingest/haven/revdep/checks.noindex/manifestoR/new/manifestoR.Rcheck/00_pkg_src/manifestoR/R/manifesto.R:456-457)
    print.ManifestoAvailability: no visible binding for global variable
      ‘annotations’
      (/Users/hadley/Documents/ingest/haven/revdep/checks.noindex/manifestoR/new/manifestoR.Rcheck/00_pkg_src/manifestoR/R/manifesto.R:371-374)
    Undefined global functions or variables:
      annotations
    ```

# memapp

Version: 2.11

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

Version: 0.3.0

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

# rio

Version: 0.5.10

## Newly broken

*   checking examples ... ERROR
    ```
    ...
    List of 5
     $ label    : chr "1978 Automobile Data"
     $ notes    : chr [1:2] "from Consumer Reports with permission" "1"
     $ names    : chr [1:12] "make" "price" "mpg" "rep78" ...
     $ row.names: int [1:74] 1 2 3 4 5 6 7 8 9 10 ...
     $ class    : chr "data.frame"
    > str(g)
    'data.frame':	74 obs. of  12 variables:
     $ make        : chr  "AMC Concord" "AMC Pacer" "AMC Spirit" "Buick Century" ...
     $ price       : num  4099 4749 3799 4816 7827 ...
     $ mpg         : num  22 17 22 20 15 18 26 20 16 19 ...
     $ rep78       : num  3 3 NA 3 4 3 NA 3 3 3 ...
     $ headroom    : num  2.5 3 3 4.5 4 4 3 2 3.5 3.5 ...
     $ trunk       : num  11 11 12 16 20 21 10 16 17 13 ...
     $ weight      : num  2930 3350 2640 3250 4080 3670 2230 3280 3880 3400 ...
     $ length      : num  186 173 168 196 222 218 170 200 207 200 ...
     $ turn        : num  40 40 35 40 43 43 34 42 43 42 ...
     $ displacement: num  121 258 121 196 350 231 304 196 231 231 ...
     $ gear_ratio  : num  3.58 2.53 3.08 2.93 2.41 ...
     $ foreign     :Error: `x` and `labels` must be same type
    Execution halted
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

Version: 2.0.1

## In both

*   checking data for non-ASCII characters ... NOTE
    ```
      Note: found 268 marked UTF-8 strings
    ```

