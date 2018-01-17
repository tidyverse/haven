## Test environments
* local OS X install: R-release
* travis-ci: R-3.1, R-3.2, R-oldrel, R-release, R-devel
* win-builder: R-devel

## R CMD check results
0 ERRORs | 0 WARNINGs | 1 NOTEs

* checking for GNU extensions in Makefiles ... NOTE
  GNU make is a SystemRequirements.

## revdepcheck results

We checked 30 reverse dependencies (29 from CRAN + 1 from BioConductor), comparing R CMD check results across CRAN and dev versions of this package.

 * We saw 0 new problems
 * We failed to check 1 packages

Issues with CRAN packages are summarised below.

### Failed to check

* BEACH (failed to install)
  Fails with "‘label<-.data.frame’ is not exported by 'namespace:Hmisc'" - 
  this seems unlikely to be related to haven
