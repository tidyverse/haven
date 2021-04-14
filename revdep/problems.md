# duawranglr

<details>

* Version: 0.6.5
* GitHub: https://github.com/btskinner/duawranglr
* Source code: https://github.com/cran/duawranglr
* Date/Publication: 2019-11-19 22:10:03 UTC
* Number of recursive dependencies: 60

Run `cloud_details(, "duawranglr")` for more info

</details>

## Newly broken

*   checking tests ... ERROR
    ```
      Running ‘testthat.R’
    Running the tests in ‘tests/testthat.R’ failed.
    Last 13 lines of output:
      > 
      > test_check("duawranglr")
      ReadStat: Error parsing page 0, bytes 1024-5119
      ══ Failed tests ════════════════════════════════════════════════════════════════
      ── Error (test-sreader__.R:69:5): Failed to read crosswalk file of type: SAS (sas7bdat) ──
      Error: Failed to parse /tmp/workdir/duawranglr/new/duawranglr.Rcheck/tests/testthat/testdata/dua_one.sas7bdat: Invalid file, or file has unsupported features.
      Backtrace:
          █
       1. └─duawranglr:::sreader__("./testdata/dua_one.sas7bdat") test-sreader__.R:69:4
       2.   └─haven::read_sas(file, ...)
       3.     └─haven:::df_parse_sas_file(...)
      
      [ FAIL 1 | WARN 0 | SKIP 0 | PASS 52 ]
      Error: Test failures
      Execution halted
    ```

## In both

*   checking dependencies in R code ... NOTE
    ```
    Namespaces in Imports field not imported from:
      ‘digest’ ‘dplyr’
      All declared Imports should be used.
    ```

