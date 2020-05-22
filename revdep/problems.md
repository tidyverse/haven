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

*   checking tests ... ERROR
    ```
      Running ‘testthat.R’
    Running the tests in ‘tests/testthat.R’ failed.
    Last 13 lines of output:
      class(update_labelled(v1)) not equal to c("haven_labelled_spss", "haven_labelled").
      Lengths differ: 4 is not 2
      
      ══ testthat results  ═══════════════════════════════════════════════════════════
      [ OK: 76 | SKIPPED: 0 | WARNINGS: 0 | FAILED: 7 ]
      1. Failure: labelled return an object of class haven_labelled (@test-labelled.r#53) 
      2. Failure: dplyr::recode could be applied to numeric labelled vector (@test-labelled.r#268) 
      3. Failure: dplyr::recode could be applied to character labelled vector (@test-labelled.r#273) 
      4. Failure: update_labelled update previous haven's labelled objects but not Hmisc's labelled objects (@test-labelled.r#282) 
      5. Failure: update_labelled update previous haven's labelled objects but not Hmisc's labelled objects (@test-labelled.r#286) 
      6. Failure: update_labelled update to haven_labelled_spss if there are na values (@test-labelled.r#296) 
      7. Failure: update_labelled update to haven_labelled_spss if there are na values (@test-labelled.r#297) 
      
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

