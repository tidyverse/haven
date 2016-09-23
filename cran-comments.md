## Test environments
* local OS X install, R 3.3.2
* ubuntu 12.04 (on travis-ci), oldrel, release, devel
* win-builder (devel)

## R CMD check results
0 ERRORs | 0 WARNINGs | 2 NOTEs

* checking for GNU extensions in Makefiles ... NOTE
  GNU make is a SystemRequirements.

* License components with restrictions and base license permitting such:
  MIT + file LICENSE

## Reverse dependencies
I have run R CMD check on the 12 reverse dependencies of haven. 

There was one problem:

* rio: uses older parameter names. It will be resubmitted once haven is
  accepted.

Full results are available at https://github.com/hadley/haven/tree/master/revdep/.

