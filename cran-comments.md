## Test environments

* local: darwin15.6.0-3.5.1
* travis: 3.1, 3.2, 3.3, oldrel, release, devel
* r-hub: windows-x86_64-devel, ubuntu-gcc-release, fedora-clang-devel
* win-builder: windows-x86_64-devel

## R CMD check results
0 ERRORs | 0 WARNINGs | 1 NOTEs

* checking for GNU extensions in Makefiles ... NOTE
  GNU make is a SystemRequirements.

## revdepcheck results

We checked 66 reverse dependencies, comparing R CMD check results across CRAN and dev versions of this package.

 * We saw 4 new problems
 * We failed to check 0 packages

Issues with CRAN packages are summarised below.

### New problems
(This reports the first line of each new failure)

* crosswalkr
  checking examples ... ERROR
  checking tests ... ERROR
  Problem caused by labelled package (below)

* ipumsr
  checking tests ... ERROR
  Fix merged in development version:
  https://github.com/mnpopcenter/ipumsr/pull/60

* labelled
  checking tests ... ERROR
  Fix merged in development version:
  https://github.com/larmarange/labelled/pull/59

* sjlabelled
  checking examples ... ERROR
  Author alerted to problem and fix proposed:
  https://github.com/strengejacke/sjlabelled/pull/35
