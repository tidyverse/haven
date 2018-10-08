## Test environments
* local OS X install: R-release
* travis-ci: R-3.1, R-3.2, R-oldrel, R-release, R-devel
* win-builder: R-devel

## R CMD check results
0 ERRORs | 0 WARNINGs | 1 NOTEs

* checking for GNU extensions in Makefiles ... NOTE
  GNU make is a SystemRequirements.

## revdepcheck results

We checked 44 reverse dependencies (43 from CRAN + 1 from BioConductor), comparing R CMD check results across CRAN and dev versions of this package.

 * We saw 1 new problems
 * We failed to check 3 packages

Issues with CRAN packages are summarised below: package authors were notified one month ago, and again today.

### New problems
(This reports the first line of each new failure)

* labelled
  checking examples ... ERROR
  checking tests ...
  checking for code/documentation mismatches ... WARNING
  checking re-building of vignette outputs ... WARNING
  checking R code for possible problems ... NOTE

### Failed to check

* BEACH        (failed to install)
* bpnreg       (failed to install)
* ImportExport (failed to install)
