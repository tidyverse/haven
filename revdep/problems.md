# BEACH

Version: 1.2.1

## In both

*   checking dependencies in R code ... NOTE
    ```
    Namespace in Imports field not imported from: ‘readxl’
      All declared Imports should be used.
    ```

# Blaunet

Version: 2.0.7

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

*   checking package dependencies ... ERROR
    ```
    Package required but not available: ‘RcppArmadillo’
    
    See section ‘The DESCRIPTION file’ in the ‘Writing R Extensions’
    manual.
    ```

# codebook

Version: 0.5.9

## In both

*   checking dependencies in R code ... NOTE
    ```
    Namespaces in Imports field not imported from:
      ‘graphics’ ‘htmltools’ ‘pander’ ‘readr’
      All declared Imports should be used.
    ```

*   checking Rd cross-references ... NOTE
    ```
    Package unavailable to check Rd xrefs: ‘labelled’
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

# EdSurvey

Version: 2.0.3

## In both

*   checking whether package ‘EdSurvey’ can be installed ... WARNING
    ```
    Found the following significant warnings:
      Warning: package ‘lfactors’ was built under R version 3.4.4
    See ‘/Users/hadley/Documents/ingest/haven/revdep/checks.noindex/EdSurvey/new/EdSurvey.Rcheck/00install.out’ for details.
    ```

# ess

Version: 0.1.1

## In both

*   checking tests ...
    ```
     ERROR
    Running the tests in ‘tests/testthat.R’ failed.
    Last 13 lines of output:
      [31m──[39m [31m1. Failure: show_country_rounds returns correct rounds for countries (@test-show_.R#100) [39m [31m─────[39m
      show_country_rounds("Italy") not equal to c(1, 2, 6).
      Lengths differ: 4 is not 3
      
      [31m──[39m [31m2. Failure: show_country_rounds returns correct rounds for countries (@test-show_.R#101) [39m [31m─────[39m
      show_country_rounds("Spain") not equal to 1:7.
      Lengths differ: 8 is not 7
      
      ══ testthat results  ═════════════════════════════════════════════════════════════════════════════
      OK: 36 SKIPPED: 23 FAILED: 2
      1. Failure: show_country_rounds returns correct rounds for countries (@test-show_.R#100) 
      2. Failure: show_country_rounds returns correct rounds for countries (@test-show_.R#101) 
      
      Error: testthat unit tests failed
      Execution halted
    ```

*   checking dependencies in R code ... NOTE
    ```
    Namespace in Imports field not imported from: ‘tibble’
      All declared Imports should be used.
    ```

# essurvey

Version: 1.0.1

## In both

*   checking dependencies in R code ... NOTE
    ```
    Namespace in Imports field not imported from: ‘tibble’
      All declared Imports should be used.
    ```

# ImportExport

Version: 1.1

## In both

*   checking whether package ‘ImportExport’ can be installed ... WARNING
    ```
    Found the following significant warnings:
      Warning: package ‘survival’ was built under R version 3.4.4
    See ‘/Users/hadley/Documents/ingest/haven/revdep/checks.noindex/ImportExport/new/ImportExport.Rcheck/00install.out’ for details.
    ```

# ipumsr

Version: 0.2.0

## In both

*   checking package dependencies ... NOTE
    ```
    Package suggested but not available for checking: ‘rgdal’
    ```

# labelled

Version: 1.1.0

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

*   checking package dependencies ... ERROR
    ```
    Package required but not available: ‘tm’
    
    See section ‘The DESCRIPTION file’ in the ‘Writing R Extensions’
    manual.
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

# mitml

Version: 0.3-5

## In both

*   checking examples ... ERROR
    ```
    Running examples in ‘mitml-Ex.R’ failed
    The error most likely occurred in:
    
    > ### Name: mids2mitml.list
    > ### Title: Convert objects of class 'mids' to 'mitml.list'
    > ### Aliases: mids2mitml.list
    > 
    > ### ** Examples
    > 
    > data(studentratings)
    > 
    > # imputation using mice
    > require(mice)
    Loading required package: mice
    Warning in library(package, lib.loc = lib.loc, character.only = TRUE, logical.return = TRUE,  :
      there is no package called ‘mice’
    > imp <- mice(studentratings)
    Error in mice(studentratings) : could not find function "mice"
    Execution halted
    ```

*   checking package dependencies ... NOTE
    ```
    Packages suggested but not available for checking: ‘mice’ ‘miceadds’
    ```

# netCoin

Version: 0.2.8

## In both

*   checking data for non-ASCII characters ... NOTE
    ```
      Note: found 77 marked UTF-8 strings
    ```

# opencpu

Version: 2.0.5

## In both

*   checking data for non-ASCII characters ... NOTE
    ```
      Note: found 4 marked UTF-8 strings
    ```

# pubh

Version: 0.1.7

## In both

*   checking dependencies in R code ... NOTE
    ```
    Namespaces in Imports field not imported from:
      ‘bookdown’ ‘descr’ ‘gee’ ‘lme4’ ‘nlme’ ‘nnet’ ‘ordinal’ ‘pander’
      ‘papeR’ ‘rmdformats’ ‘visreg’
      All declared Imports should be used.
    ```

# RcmdrMisc

Version: 1.0-10

## In both

*   checking whether package ‘RcmdrMisc’ can be installed ... WARNING
    ```
    Found the following significant warnings:
      Warning: package ‘car’ was built under R version 3.4.4
      Warning: package ‘carData’ was built under R version 3.4.4
    See ‘/Users/hadley/Documents/ingest/haven/revdep/checks.noindex/RcmdrMisc/new/RcmdrMisc.Rcheck/00install.out’ for details.
    ```

# readit

Version: 1.0.0

## In both

*   checking package dependencies ... ERROR
    ```
    Package required and available but unsuitable version: ‘tools’
    
    See section ‘The DESCRIPTION file’ in the ‘Writing R Extensions’
    manual.
    ```

# rio

Version: 0.5.10

## In both

*   checking package dependencies ... NOTE
    ```
    Package suggested but not available for checking: ‘fst’
    ```

*   checking Rd cross-references ... NOTE
    ```
    Package unavailable to check Rd xrefs: ‘fst’
    ```

# tidyverse

Version: 1.2.1

## In both

*   checking whether package ‘tidyverse’ can be installed ... WARNING
    ```
    Found the following significant warnings:
      Warning: package ‘tibble’ was built under R version 3.4.3
      Warning: package ‘tidyr’ was built under R version 3.4.4
      Warning: package ‘purrr’ was built under R version 3.4.4
      Warning: package ‘dplyr’ was built under R version 3.4.4
      Warning: package ‘forcats’ was built under R version 3.4.3
    See ‘/Users/hadley/Documents/ingest/haven/revdep/checks.noindex/tidyverse/new/tidyverse.Rcheck/00install.out’ for details.
    ```

*   checking dependencies in R code ... NOTE
    ```
    Namespaces in Imports field not imported from:
      ‘dbplyr’ ‘reprex’ ‘rlang’
      All declared Imports should be used.
    ```

# xSub

Version: 0.0.1

## In both

*   checking data for non-ASCII characters ... NOTE
    ```
      Note: found 63 marked UTF-8 strings
    ```

