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

We checked 46 reverse dependencies (45 from CRAN + 1 from BioConductor), comparing R CMD check results across CRAN and dev versions of this package.

 * We saw 1 new problems
 * We failed to check 2 packages

Issues with CRAN packages are summarised below.

### New problems
(This reports the first line of each new failure)

* essurvey
  checking tests ...
  
  Fails with `Actual message: "Gateway Timeout (HTTP 504)."` suggesting
  that this is not related to haven.

### Failed to check

* bpnreg       (failed to install)
* ImportExport (failed to install)

### Failed to check

* bpnreg       (failed to install)
* ImportExport (failed to install)
