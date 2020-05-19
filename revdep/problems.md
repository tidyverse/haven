# crosswalkr

<details>

* Version: 0.2.6
* Source code: https://github.com/cran/crosswalkr
* URL: https://github.com/btskinner/crosswalkr
* BugReports: https://github.com/btskinner/crosswalkr/issues
* Date/Publication: 2020-01-08 14:30:02 UTC
* Number of recursive dependencies: 59

Run `cloud_details(, "crosswalkr")` for more info

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

# dataMaid

<details>

* Version: 1.4.0
* Source code: https://github.com/cran/dataMaid
* URL: https://github.com/ekstroem/dataMaid, https://doi.org/10.18637/jss.v090.i06
* BugReports: https://github.com/ekstroem/dataMaid/issues
* Date/Publication: 2019-12-10 19:20:09 UTC
* Number of recursive dependencies: 77

Run `cloud_details(, "dataMaid")` for more info

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

Run `cloud_details(, "ipumsr")` for more info

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
      [ OK: 243 | SKIPPED: 11 | WARNINGS: 0 | FAILED: 1 ]
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

Run `cloud_details(, "labelled")` for more info

</details>

## Newly broken

*   checking examples ... ERROR
    ```
    ...
    > ### ** Examples
    > 
    > v <- labelled(c(1,2,2,2,3,9,1,3,2,NA), c(yes = 1, no = 3, "don't know" = 9))
    > v
    <labelled<double>[10]>
     [1]  1  2  2  2  3  9  1  3  2 NA
    
    Labels:
     value      label
         1        yes
         3         no
         9 don't know
    > na_values(v) <- 9
    Error: `na_values` must be same type as `x`.
    Backtrace:
        █
     1. ├─labelled::`na_values<-`(`*tmp*`, value = 9)
     2. └─labelled:::`na_values<-.haven_labelled`(`*tmp*`, value = 9)
     3.   └─haven::labelled_spss(...)
     4.     └─haven:::new_labelled_spss(...)
    Execution halted
    ```

*   checking tests ... ERROR
    ```
      Running ‘testthat.R’
    Running the tests in ‘tests/testthat.R’ failed.
    Last 13 lines of output:
      ══ testthat results  ═══════════════════════════════════════════════════════════
      [ OK: 47 | SKIPPED: 0 | WARNINGS: 0 | FAILED: 15 ]
      1. Failure: labelled return an object of class haven_labelled (@test-labelled.r#53) 
      2. Error: val_labels preserves variable label (@test-labelled.r#76) 
      3. Error: val_label preserves variable label (@test-labelled.r#86) 
      4. Error: val_labels and val_label preserves spss missing values (@test-labelled.r#95) 
      5. Error: value labels can be removed if missing values are defined (@test-labelled.r#113) 
      6. Error: strict option of to_factor works correctly (@test-labelled.r#144) 
      7. Failure: set_value_labels replaces all value labels (@test-labelled.r#222) 
      8. Error: add_value_labels and remove_value_labels updates the list of value labels (@test-labelled.r#228) 
      9. Error: na_values and na_range keep variable label (@test-labelled.r#261) 
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

Run `cloud_details(, "rdhs")` for more info

</details>

## Newly broken

*   checking examples ... ERROR
    ```
    ...
    + labels=list(area = "concatenate", climate = c("cold"=0, "warm/hot"=1)))
    Error: Can't convert <double> to <labelled<integer>>.
    Backtrace:
         █
      1. ├─rdhs::rbind_labelled(...)
      2. │ └─base::Map(haven::labelled, df[names(labels)], labels, labl)
      3. │   └─base::mapply(FUN = f, ..., SIMPLIFY = FALSE)
      4. │     └─(function (x = double(), labels = NULL, label = NULL) ...
      5. │       └─haven:::vec_cast_named(labels, x, x_arg = "labels", to_arg = "x")
      6. │         ├─stats::setNames(vec_cast(x, to, ...), names(x))
      7. │         └─vctrs::vec_cast(x, to, ...)
      8. ├─vctrs::vec_default_cast(...)
      9. │ └─vctrs:::vctr_cast(x, to, x_arg = x_arg, to_arg = to_arg)
     10. │   └─vctrs::vec_restore(x, to)
     11. ├─vctrs:::vec_restore_dispatch(x = x, to = to, n = n)
     12. └─vctrs:::vec_restore.vctrs_vctr(x = x, to = to, n = n)
     13.   └─vctrs::stop_incompatible_cast(x, to, x_arg = "", to_arg = "")
     14.     └─vctrs::stop_incompatible_type(...)
     15.       └─vctrs:::stop_incompatible(...)
     16.     
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

Run `cloud_details(, "sjlabelled")` for more info

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

