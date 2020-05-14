# crosswalkr

<details>

* Version: 0.2.6
* Source code: https://github.com/cran/crosswalkr
* URL: https://github.com/btskinner/crosswalkr
* BugReports: https://github.com/btskinner/crosswalkr/issues
* Date/Publication: 2020-01-08 14:30:02 UTC
* Number of recursive dependencies: 59

Run `revdep_details(,"crosswalkr")` for more info

</details>

## Newly broken

*   checking examples ... ERROR
    ```
    ...
    The error most likely occurred in:
    
    > ### Name: encodefrom
    > ### Title: Encode data frame column using external crosswalk file.
    > ### Aliases: encodefrom encodefrom_
    > 
    > ### ** Examples
    > 
    > df <- data.frame(state = c('Kentucky','Tennessee','Virginia'),
    +                  stfips = c(21,47,51),
    +                  cenregnm = c('South','South','South'))
    > 
    > df_tbl <- tibble::as_tibble(df)
    > 
    > cw <- get(data(stcrosswalk))
    > 
    > df$state2 <- encodefrom(df, state, cw, stname, stfips, stabbr)
    > df_tbl$state2 <- encodefrom(df_tbl, state, cw, stname, stfips, stabbr)
    Error in is_coercible(x, labels) : could not find function "is_coercible"
    Calls: encodefrom -> encodefrom_ -> <Anonymous>
    Execution halted
    ```

*   checking tests ... ERROR
    ```
      Running ‘testthat.R’
    Running the tests in ‘tests/testthat.R’ failed.
    Last 13 lines of output:
      > 
      > test_check("crosswalkr")
      ── 1. Error: (unknown) (@test-encodefrom.R#32)  ────────────────────────────────
      could not find function "is_coercible"
      Backtrace:
       1. crosswalkr::encodefrom(...)
       2. crosswalkr::encodefrom_(...)
       3. labelled::labelled(val_vec, labels = val_labels)
      
      ══ testthat results  ═══════════════════════════════════════════════════════════
      [ OK: 36 | SKIPPED: 0 | WARNINGS: 0 | FAILED: 1 ]
      1. Error: (unknown) (@test-encodefrom.R#32) 
      
      Error: testthat unit tests failed
      Execution halted
    ```

# crunch

<details>

* Version: 1.26.3
* Source code: https://github.com/cran/crunch
* URL: https://crunch.io/r/crunch/, https://github.com/Crunch-io/rcrunch
* BugReports: https://github.com/Crunch-io/rcrunch/issues
* Date/Publication: 2020-03-12 18:00:02 UTC
* Number of recursive dependencies: 103

Run `revdep_details(,"crunch")` for more info

</details>

## Newly broken

*   checking tests ... ERROR
    ```
      Running ‘spelling.R’
      Running ‘testthat.R’
    Running the tests in ‘tests/testthat.R’ failed.
    Last 13 lines of output:
      [90m 12. [39mvctrs::vec_default_cast(...)
      [90m 13. [39mvctrs::stop_incompatible_cast(...)
      [90m 14. [39mvctrs::stop_incompatible_type(...)
      [90m 15. [39mvctrs:::stop_incompatible(...)
      [90m 16. [39mvctrs:::stop_vctrs(...)
      
      
      ══ testthat results  ═══════════════════════════════════════════════════════════
      [ OK: 3135 | SKIPPED: 16 | WARNINGS: 0 | FAILED: 2 ]
      1. Error: toVariable parses haven::labelled (@test-add-variable.R#114) 
      2. Error: toVariable parses haven::labelled_spss (@test-add-variable.R#132) 
      
      Error: testthat unit tests failed
      Total teardown: Time difference of 2.098083e-05 secs
      Execution halted
    ```

# dataMaid

<details>

* Version: 1.4.0
* Source code: https://github.com/cran/dataMaid
* URL: https://github.com/ekstroem/dataMaid, https://doi.org/10.18637/jss.v090.i06
* BugReports: https://github.com/ekstroem/dataMaid/issues
* Date/Publication: 2019-12-10 19:20:09 UTC
* Number of recursive dependencies: 77

Run `revdep_details(,"dataMaid")` for more info

</details>

## Newly broken

*   checking tests ... ERROR
    ```
      Running ‘testthat.R’
    Running the tests in ‘tests/testthat.R’ failed.
    Last 13 lines of output:
      If any elements are named, all elements must be named.
      Backtrace:
        1. testthat::expect_equal(length(summarize(typelab)), 4)
       21. vctrs:::set_names_fallback(x = x, names = names)
       23. vctrs:::`names<-.vctrs_vctr`(`*tmp*`, value = names)
      
      ══ testthat results  ═══════════════════════════════════════════════════════════
      [ OK: 62 | SKIPPED: 0 | WARNINGS: 0 | FAILED: 4 ]
      1. Error: check returns a list for (most) atomic vectors (@testcheck.R#33) 
      2. Error: check return the right number of tests for (most) atomic vectors (@testcheck.R#47) 
      3. Error: summarize returns a list for (most) atomic vectors (@testsummarize.R#30) 
      4. Error: summarize return the right number of tests for (most) atomic vectors (@testsummarize.R#44) 
      
      Error: testthat unit tests failed
      Execution halted
    ```

# ipumsr

<details>

* Version: 0.4.3
* Source code: https://github.com/cran/ipumsr
* URL: https://www.ipums.org, https://github.com/mnpopcenter/ipumsr
* BugReports: https://github.com/mnpopcenter/ipumsr/issues
* Date/Publication: 2020-04-30 23:40:05 UTC
* Number of recursive dependencies: 107

Run `revdep_details(,"ipumsr")` for more info

</details>

## Newly broken

*   checking tests ... ERROR
    ```
      Running ‘testthat.R’
    Running the tests in ‘tests/testthat.R’ failed.
    Last 13 lines of output:
      
          matches
      
      > 
      > test_check("ipumsr")
      ── 1. Failure: lbl_define: basic (@test_lbls.r#193)  ───────────────────────────
      lbl_define(...) not equal to `college_age`.
      Attributes: < Component "class": Lengths (1, 3) differ (string compare on first 1) >
      
      ══ testthat results  ═══════════════════════════════════════════════════════════
      [ OK: 195 | SKIPPED: 23 | WARNINGS: 0 | FAILED: 1 ]
      1. Failure: lbl_define: basic (@test_lbls.r#193) 
      
      Error: testthat unit tests failed
      Execution halted
    ```

# labelled

<details>

* Version: 2.3.1
* Source code: https://github.com/cran/labelled
* URL: http://larmarange.github.io/labelled/
* BugReports: https://github.com/larmarange/labelled/issues
* Date/Publication: 2020-04-29 10:30:07 UTC
* Number of recursive dependencies: 69

Run `revdep_details(,"labelled")` for more info

</details>

## Newly broken

*   checking examples ... ERROR
    ```
    ...
    [1] Male   Male   Female
    Levels: Male Female
    > to_factor(s1, levels = "prefixed")
    [1] [M] Male   [M] Male   [F] Female
    Levels: [M] Male [F] Female
    > to_factor(s2)
    Error: Can't convert <labelled<double>> to <character>.
    Backtrace:
         █
      1. ├─labelled::to_factor(s2)
      2. ├─labelled:::to_factor.haven_labelled(s2)
      3. │ └─base::factor(...)
      4. │   ├─base::as.character(x)
      5. │   └─vctrs:::as.character.vctrs_vctr(x)
      6. │     └─vctrs::vec_cast(x, character())
      7. └─haven:::vec_cast.character.haven_labelled(...)
      8.   └─vctrs::stop_incompatible_cast(x, to, ...)
      9.     └─vctrs::stop_incompatible_type(...)
     10.       └─vctrs:::stop_incompatible(...)
     11.         └─vctrs:::stop_vctrs(...)
    Execution halted
    ```

*   checking tests ... ERROR
    ```
      Running ‘testthat.R’
    Running the tests in ‘tests/testthat.R’ failed.
    Last 13 lines of output:
      ══ testthat results  ═══════════════════════════════════════════════════════════
      [ OK: 34 | SKIPPED: 0 | WARNINGS: 0 | FAILED: 21 ]
      1. Failure: labelled return an object of class haven_labelled (@test-labelled.r#53) 
      2. Error: val_labels preserves variable label (@test-labelled.r#76) 
      3. Error: val_label preserves variable label (@test-labelled.r#86) 
      4. Error: val_labels and val_label preserves spss missing values (@test-labelled.r#95) 
      5. Error: value labels can be removed if missing values are defined (@test-labelled.r#113) 
      6. Error: to_factor preserves variable label (@test-labelled.r#139) 
      7. Error: strict option of to_factor works correctly (@test-labelled.r#144) 
      8. Error: to_factor works on data.frame (@test-labelled.r#157) 
      9. Error: to_character produce an appropriate character vector (@test-labelled.r#169) 
      1. ...
      
      Error: testthat unit tests failed
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

# rdhs

<details>

* Version: 0.6.3
* Source code: https://github.com/cran/rdhs
* URL: https://ropensci.github.io/rdhs/
* BugReports: https://github.com/ropensci/rdhs/issues
* Date/Publication: 2019-03-19 14:20:03 UTC
* Number of recursive dependencies: 89

Run `revdep_details(,"rdhs")` for more info

</details>

## Newly broken

*   checking examples ... ERROR
    ```
    ...
    + climate = haven::labelled(c(1L, 0L), c("cold"=0, "warm"=1))
    + )
    > 
    > # Default: all data frames inherit labels from first df. Incorrect if
    > # "reg 1" and "reg A" are from different countries, for example.
    > dfA <- rbind_labelled(df1, df2)
    Warning in rbind_labelled(df1, df2) :
      Some variables have non-matching value labels: area, climate.
    Inheriting labels from first data frame with labels.
    Error: `levels.haven_labelled()` not supported.
    Backtrace:
        █
     1. └─rdhs::rbind_labelled(df1, df2)
     2.   ├─base::do.call(rbind, dfs)
     3.   └─(function (..., deparse.level = 1) ...
     4.     └─base::rbind(deparse.level, ...)
     5.       ├─base::levels(xj)
     6.       └─vctrs:::levels.vctrs_vctr(xj)
     7.         └─vctrs:::stop_unsupported(x, "levels")
     8.           └─vctrs:::stop_vctrs(...)
    Execution halted
    ```

*   checking tests ... ERROR
    ```
      Running ‘testthat.R’
    Running the tests in ‘tests/testthat.R’ failed.
    Last 13 lines of output:
       2. base::Map(...)
       3. base::mapply(FUN = f, ..., SIMPLIFY = FALSE)
       6. haven:::new_labelled(x, labels = labels, label = label)
      
      trying URL 'https://dhsprogram.com/customcf/legacy/data/sample_download_dataset.cfm?Filename=ZZAR61FL.ZIP&Tp=4&Ctry_Code=zz&survey_id=0&doctype=hiv'
      Content type 'application/x-zip-compressed' length 81939 bytes (80 KB)
      ==================================================
      downloaded 80 KB
      
      ══ testthat results  ═══════════════════════════════════════════════════════════
      [ OK: 18 | SKIPPED: 44 | WARNINGS: 0 | FAILED: 1 ]
      1. Error: datasets parse (@test_read_dhs_flat.R#13) 
      
      Error: testthat unit tests failed
      Execution halted
    ```

# sjlabelled

<details>

* Version: 1.1.4
* Source code: https://github.com/cran/sjlabelled
* URL: https://strengejacke.github.io/sjlabelled
* BugReports: https://github.com/strengejacke/sjlabelled/issues
* Date/Publication: 2020-04-30 22:00:07 UTC
* Number of recursive dependencies: 96

Run `revdep_details(,"sjlabelled")` for more info

</details>

## Newly broken

*   checking examples ... ERROR
    ```
    ...
        as_factor, read_sas, read_spss, read_stata, write_sas, zap_labels
    
    Error: Can't convert <character> to <labelled<double>>.
    Backtrace:
         █
      1. ├─sjlabelled::as_label(x, drop.na = FALSE)
      2. ├─sjlabelled:::as_label.default(x, drop.na = FALSE)
      3. │ └─sjlabelled:::as_label_helper(...)
      4. │   ├─base::`[<-`(...)
      5. │   └─vctrs:::`[<-.vctrs_vctr`(...)
      6. │     └─vctrs::vec_cast(value, x)
      7. ├─vctrs::vec_default_cast(...)
      8. │ └─vctrs:::vctr_cast(x, to, x_arg = x_arg, to_arg = to_arg)
      9. │   └─vctrs::vec_restore(x, to)
     10. ├─vctrs:::vec_restore_dispatch(x = x, to = to, n = n)
     11. └─vctrs:::vec_restore.vctrs_vctr(x = x, to = to, n = n)
     12.   └─vctrs::stop_incompatible_cast(x, to, x_arg = "", to_arg = "")
     13.     └─vctrs::stop_incompatible_type(...)
     14.       └─vctrs:::stop_incompatible(...)
     15.         └─vctrs:::stop_vctrs(...)
    Execution halted
    ```

# sjmisc

<details>

* Version: 2.8.4
* Source code: https://github.com/cran/sjmisc
* URL: https://strengejacke.github.io/sjmisc
* BugReports: https://github.com/strengejacke/sjmisc/issues
* Date/Publication: 2020-04-03 13:20:02 UTC
* Number of recursive dependencies: 97

Run `revdep_details(,"sjmisc")` for more info

</details>

## Newly broken

*   checking examples ... ERROR
    ```
    ...
    +   # recode 2 into 5; Values of tagged NAs are preserved
    +   rec(x, rec = "2=5;else=copy")
    +   na_tag(rec(x, rec = "2=5;else=copy"))
    + }
    Loading required package: haven
    Error: <labelled<double>> %% <double> is not permitted
    Backtrace:
         █
      1. ├─sjmisc::rec(x, rec = "2=5;else=copy")
      2. └─sjmisc:::rec.default(x, rec = "2=5;else=copy")
      3.   └─sjmisc:::rec_core_fun(...)
      4.     └─sjmisc:::rec_helper(...)
      5.       ├─sjmisc::is_float(x)
      6.       └─sjmisc:::is_float.default(x)
      7.         └─vctrs:::`%%.vctrs_vctr`(x, 1)
      8.           ├─vctrs::vec_arith("%%", e1, e2)
      9.           └─vctrs:::vec_arith.default("%%", e1, e2)
     10.             └─vctrs::stop_incompatible_op(op, x, y)
     11.               └─vctrs:::stop_incompatible(...)
     12.                 └─vctrs:::stop_vctrs(...)
    Execution halted
    ```

