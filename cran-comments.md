## Changes

This update fixes a number of smaller bugs. It includes some improved UTF-8 handling, so may fix the problem on r-prerel-solaris-sparc. 

## Test environments
* local OS X install, R 3.1.3
* ubuntu 12.04 (on travis-ci), R 3.1.3
* win-builder (devel and release)

## R CMD check results
There were no ERRORs or WARNINGs. 

There was 1 NOTE:

* Possibly mis-spelled words in DESCRIPTION:
  ReadStat, SAS, SPSS and Stata - this are not misspelled.

## Reverse dependencies
I have run R CMD check on the 2 reverse dependencies of haven. There were no signficant problems.

Full results are available at https://github.com/hadley/haven/tree/master/revdep/summary.md.
