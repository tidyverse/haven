This is a resubmission, changing AUTHOR to COPYRIGHT HOLDER in license. 

---

## Test environments
* local OS X install, R 3.4.0
* ubuntu 12.04 (on travis-ci), oldrel, release, devel
* win-builder (devel)

## R CMD check results
0 ERRORs | 0 WARNINGs | 2 NOTEs

* checking for GNU extensions in Makefiles ... NOTE
  GNU make is a SystemRequirements.

* License components with restrictions and base license permitting such:
  MIT + file LICENSE

## Reverse dependencies
I have run R CMD check on the 13 reverse dependencies of haven. 
I did not see any new issues.

I am using a new system for reverse dependency checks, co-developed with
Gabor Csardi. Now, I run R CMD check twice for each revdep: once with the
CRAN version of haven, and once with the development verison. I then
compare the two, to see if there are any new problems. This substantially
decreases the number of false positions and makes it much easier to
see exactly what's changed.

Full results are available at https://github.com/hadley/haven/tree/master/revdep/.

