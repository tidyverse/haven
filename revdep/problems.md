# codebook (0.10.1)

* GitHub: <https://github.com/rubenarslan/codebook>
* Email: <mailto:ruben.arslan@gmail.com>
* GitHub mirror: <https://github.com/cran/codebook>

Run `revdepcheck::cloud_details(, "codebook")` for more info

## Newly broken

*   checking re-building of vignette outputs ... ERROR
     ```
     ...
       7.       └─base::eval(expr, p)
       8.         └─base::eval(...)
       9.           └─base::eval(...)
      10.             ├─future::value(future)
      11.             └─future:::value.Future(future)
      12.               └─future:::signalConditions(...)
     ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
     
     Error: processing vignette 'codebook_sav.Rmd' failed with diagnostics:
     Could not summarise item relStat. Error in `map()`:
     ℹ In index: 1.
     ℹ With name: relStat.
     Caused by error in `purrr::flatten()`:
     ! `.x[[2]]` must be a vector, not a <haven::meta_var_spss> object.
     
     --- failed re-building ‘codebook_sav.Rmd’
     
     --- re-building ‘codebook_tutorial.Rmd’ using rmarkdown
     --- finished re-building ‘codebook_tutorial.Rmd’
     
     SUMMARY: processing the following file failed:
       ‘codebook_sav.Rmd’
     
     Error: Vignette re-building failed.
     Execution halted
     ```

# coreval (0.3.0)

* GitHub: <https://github.com/hrach-gevorgyan/coreval>
* Email: <mailto:hrach.gevorgyan@yandex.com>
* GitHub mirror: <https://github.com/cran/coreval>

Run `revdepcheck::cloud_details(, "coreval")` for more info

## Newly broken

*   checking tests ... ERROR
     ```
     ...
     Running the tests in ‘tests/testthat.R’ failed.
     Complete output:
       > library(testthat)
       > library(coreval)
       > 
       > test_check("coreval")
       Saving _problems/test-read-194.R
       4 problems across 2 records  (160 checks ran, 51 could not)
         wrong value       2
         missing required  1
         missing optional  1
       [ FAIL 1 | WARN 0 | SKIP 0 | PASS 1303 ]
       
       ══ Failed tests ════════════════════════════════════════════════════════════════
       ── Failure ('test-read.R:194:3'): read_study reads a directory of XPT files with the same semantics ──
       Expected `dm$data$AGE` to equal `c(45, NA, 7)`.
       Differences:
       `attr(actual, 'haven_meta')` is an S7 object of class <haven::meta_var_sas>
       `attr(expected, 'haven_meta')` is absent
       
       
       [ FAIL 1 | WARN 0 | SKIP 0 | PASS 1303 ]
       Error:
       ! Test failures.
       Execution halted
     ```

# DIDHAD (2.0.0)

* Email: <mailto:diego.ciccia@kellogg.northwestern.edu>
* GitHub mirror: <https://github.com/cran/DIDHAD>

Run `revdepcheck::cloud_details(, "DIDHAD")` for more info

## Newly broken

*   checking examples ... ERROR
     ```
     Running examples in ‘DIDHAD-Ex.R’ failed
     The error most likely occurred in:
     
     > ### Name: did_had
     > ### Title: Main function of the DIDHAD package
     > ### Aliases: did_had
     > 
     > ### ** Examples
     > 
     > # The sample data for this example can be downloaded by running:
     > repo <-"https://raw.githubusercontent.com/chaisemartinPackages/did_had/" 
     > data <- haven::read_dta(paste0(repo,"main/tutorial_data.dta"))
     Error: Failed to parse /tmp/RtmpKG7AHD/filed3d14ded87f.dta: Unable to open file.
     Execution halted
     ```

# eatGADS (1.2.0)

* GitHub: <https://github.com/beckerbenj/eatGADS>
* Email: <mailto:b.becker@iqb.hu-berlin.de>
* GitHub mirror: <https://github.com/cran/eatGADS>

Run `revdepcheck::cloud_details(, "eatGADS")` for more info

## Newly broken

*   checking examples ... ERROR
     ```
     Running examples in ‘eatGADS-Ex.R’ failed
     The error most likely occurred in:
     
     > ### Name: import_spss
     > ### Title: Import SPSS data
     > ### Aliases: import_spss
     > 
     > ### ** Examples
     > 
     > # Use spss data from within package
     > spss_path <- system.file("extdata", "pisa.zsav", package = "eatGADS")
     > pisa_gads <- import_spss(spss_path)
     Error in extract_variable_level.savDat(rawDat = rawDat) : 
       Unknown attributes exported from haven:haven_metahaven_metahaven_metahaven_metahaven_metahaven_metahaven_metahaven_metahaven_metahaven_metahaven_metahaven_metahaven_metahaven_metahaven_metahaven_metahaven_metahaven_metahaven_metahaven_metahaven_metahaven_metahaven_metahaven_metahaven_metahaven_metahaven_metahaven_metahaven_metahaven_metahaven_metahaven_metahaven_metahaven_metahaven_metahaven_metahaven_metahaven_metahaven_metahaven_metahaven_metahaven_metahaven_metahaven_metahaven_metahaven_metahaven_metahaven_metahaven_metahaven_metahaven_metahaven_metahaven_metahaven_metahaven_metahaven_metahaven_metahaven_metahaven_metahaven_metahaven_metahaven_metahaven_metahaven_metahaven_metahaven_metahaven_metahaven_metahaven_metahaven_metahaven_metahaven_metahaven_metahaven_metahaven_metahaven_metahaven_metahaven_metahaven_metahaven_metahaven_metahaven_metahaven_metahaven_metahaven_metahaven_metahaven_metahaven_metahaven_metahaven_metahaven_metahaven_metahaven_metahaven_metahaven_metaha
     Calls: import_spss ... extract_variable_level -> extract_variable_level.savDat
     Execution halted
     ```

*   checking tests ... ERROR
     ```
     ...
       Error in `extract_variable_level.savDat(rawDat = rawDat)`: Unknown attributes exported from haven:haven_metahaven_metahaven_meta. Please contact package author.
       Backtrace:
           ▆
        1. └─eatGADS::import_stata(sav_path) at test_write_stata.R:31:3
        2.   └─eatGADS:::prepare_labels(...)
        3.     └─eatGADS:::extract_labels(rawDat = rawDat)
        4.       ├─eatGADS:::extract_variable_level(rawDat = rawDat)
        5.       └─eatGADS:::extract_variable_level.savDat(rawDat = rawDat)
       ── Error ('test_write_stata.R:44:3'): Write strings longer than 255 ────────────
       Error in `extract_variable_level.savDat(rawDat = rawDat)`: Unknown attributes exported from haven:haven_metahaven_meta. Please contact package author.
       Backtrace:
           ▆
        1. ├─base::suppressWarnings(g <- import_spss("helper_longstring.sav")) at test_write_stata.R:44:3
        2. │ └─base::withCallingHandlers(...)
        3. └─eatGADS::import_spss("helper_longstring.sav")
        4.   └─eatGADS:::prepare_labels(...)
        5.     └─eatGADS:::extract_labels(rawDat = rawDat)
        6.       ├─eatGADS:::extract_variable_level(rawDat = rawDat)
        7.       └─eatGADS:::extract_variable_level.savDat(rawDat = rawDat)
       
       [ FAIL 64 | WARN 0 | SKIP 0 | PASS 568 ]
       Error:
       ! Test failures.
       Execution halted
       File /tmp/RtmpwYHxvW/helper_dataBase.db was removed.
     ```

*   checking re-building of vignette outputs ... ERROR
     ```
     ...
     Quitting from recoding_multipleChoice.rmd:36-43 [setup]
     ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
     <error/rlang_error>
     Error in `extract_variable_level.savDat()`:
     ! Unknown attributes exported from haven:haven_metahaven_metahaven_metahaven_meta. Please contact package author.
     ---
     Backtrace:
         ▆
      1. └─eatGADS::import_spss(data_path)
      2.   └─eatGADS:::prepare_labels(...)
      3.     └─eatGADS:::extract_labels(rawDat = rawDat)
      4.       ├─eatGADS:::extract_variable_level(rawDat = rawDat)
      5.       └─eatGADS:::extract_variable_level.savDat(rawDat = rawDat)
     ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
     
     Error: processing vignette 'recoding_multipleChoice.rmd' failed with diagnostics:
     Unknown attributes exported from haven:haven_metahaven_metahaven_metahaven_meta. Please contact package author.
     --- failed re-building ‘recoding_multipleChoice.rmd’
     
     SUMMARY: processing the following files failed:
       ‘createGADS.Rmd’ ‘import_spss.Rmd’ ‘recoding_forcedChoice.rmd’
       ‘recoding_multipleChoice.rmd’
     
     Error: Vignette re-building failed.
     Execution halted
     ```

# fastreg (0.14.1)

* GitHub: <https://github.com/dp-next/fastreg>
* Email: <mailto:signekb@clin.au.dk>
* GitHub mirror: <https://github.com/cran/fastreg>

Run `revdepcheck::cloud_details(, "fastreg")` for more info

## Newly broken

*   checking tests ... ERROR
     ```
     ...
       
       `attr(actual$pnr, 'haven_meta')` is absent
       `attr(expected$pnr, 'haven_meta')` is an S7 object of class <haven::meta_var_sas>
       
       `attr(actual$foed_dato, 'haven_meta')` is absent
       `attr(expected$foed_dato, 'haven_meta')` is an S7 object of class <haven::meta_var_sas>
       
       ── Error ('test-convert.R:196:3'): read_sas_chunk() returns first chunk when skip is NA and empty tibble when skip exceeds the 32-bit limit ──
       Error in `haven::read_sas(path, skip = skip, n_max = chunk_size)`: `skip` must be a whole number, not `NA`.
       Backtrace:
           ▆
        1. └─fastreg:::read_sas_chunk(path, skip = NA, chunk_size = 100L) at test-convert.R:196:3
        2.   ├─dplyr::mutate(...)
        3.   ├─fastreg:::column_names_to_lower(...)
        4.   │ └─dplyr::rename_with(data, tolower)
        5.   └─haven::read_sas(skip = skip)
        6.     └─rlang::check_number_whole(skip, min = 0)
        7.       └─rlang:::.stop_not_number(...)
        8.         └─rlang::stop_input_type(...)
        9.           └─rlang::abort(message, ..., call = call, arg = arg)
       
       [ FAIL 2 | WARN 0 | SKIP 1 | PASS 76 ]
       Error:
       ! Test failures.
       Execution halted
     ```

# jmvReadWrite (0.4.14)

* GitHub: <https://github.com/sjentsch/jmvReadWrite>
* Email: <mailto:sebastian.jentschke@uib.no>
* GitHub mirror: <https://github.com/cran/jmvReadWrite>

Run `revdepcheck::cloud_details(, "jmvReadWrite")` for more info

## Newly broken

*   checking tests ... ERROR
     ```
     ...
       `expected$ID` is NULL
       
       `actual$supp` is a list
       `expected$supp` is NULL
       
       `actual$supp2` is a list
       `expected$supp2` is NULL
       
       `actual$dose` is a list
       `expected$dose` is NULL
       
       `actual$dose2` is a list
       `expected$dose2` is NULL
       
       `actual$len` is a list
       `expected$len` is NULL
       
       `actual$logLen` is a list
       `expected$logLen` is NULL
       
       
       [ FAIL 6 | WARN 0 | SKIP 0 | PASS 1689 ]
       Error:
       ! Test failures.
       Execution halted
     ```

# libr (1.4.2)

* GitHub: <https://github.com/dbosak01/libr>
* Email: <mailto:dbosak01@gmail.com>
* GitHub mirror: <https://github.com/cran/libr>

Run `revdepcheck::cloud_details(, "libr")` for more info

## Newly broken

*   checking tests ... ERROR
     ```
     ...
       Loading required package: common
       > 
       > test_check("libr")
       # library 'dat': 2 items
       - attributes: csv not loaded
       - path: ./data
       - items:
                Name Extension Rows Cols    Size        LastModified
       1 demo_studya       csv   10    9 11.7 Kb 2026-10-20 21:47:17
       2 demo_studyb       csv    2    9 10.7 Kb 2026-10-20 21:47:17
       Saving _problems/test-specs-370.R
       [ FAIL 1 | WARN 0 | SKIP 0 | PASS 573 ]
       
       ══ Failed tests ════════════════════════════════════════════════════════════════
       ── Failure ('test-specs.R:370:3'): specs17: dm and pe multiple import_spec col_types work for sas7bdat. ──
       Expected `class(dat$dm$BIRTHDD)` to equal "numeric".
       Differences:
       1/1 mismatches
       x[1]: "Date"
       y[1]: "numeric"
       
       [ FAIL 1 | WARN 0 | SKIP 0 | PASS 573 ]
       Error:
       ! Test failures.
       Execution halted
     ```

# mariposa (0.7.3)

* GitHub: <https://github.com/YannickDiehl/mariposa>
* Email: <mailto:yannick.diehl@gmail.com>
* GitHub mirror: <https://github.com/cran/mariposa>

Run `revdepcheck::cloud_details(, "mariposa")` for more info

## Newly broken

*   checking tests ... ERROR
     ```
     ...
       `expected`: TRUE 
       
       ── Failure ('test-read-sas.R:166:3'): frequency shows SAS tagged NA codes ──────
       Expected `".B" %in% na_display_vals` to be TRUE.
       Differences:
       `actual`:   FALSE
       `expected`: TRUE 
       
       ── Failure ('test-read-sas.R:185:3'): codebook works with SAS tagged NAs ───────
       Expected `".A" %in% na_vals` to be TRUE.
       Differences:
       `actual`:   FALSE
       `expected`: TRUE 
       
       ── Failure ('test-read-sas.R:186:3'): codebook works with SAS tagged NAs ───────
       Expected `".B" %in% na_vals` to be TRUE.
       Differences:
       `actual`:   FALSE
       `expected`: TRUE 
       
       
       [ FAIL 12 | WARN 10 | SKIP 178 | PASS 3476 ]
       Error:
       ! Test failures.
       Execution halted
     ```

# OneSampleMR (0.1.8)

* GitHub: <https://github.com/remlapmot/OneSampleMR>
* Email: <mailto:remlapmot@hotmail.com>
* GitHub mirror: <https://github.com/cran/OneSampleMR>

Run `revdepcheck::cloud_details(, "OneSampleMR")` for more info

## Newly broken

*   checking re-building of vignette outputs ... ERROR
     ```
     ...
     --- finished re-building ‘compare-smm-fits.Rmd’
     
     --- re-building ‘f-statistic-comparison.Rmd’ using rmarkdown
     
     Quitting from f-statistic-comparison.Rmd:34-40 [unnamed-chunk-2]
     ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
     <error/rlang_error>
     Error:
     ! Failed to parse /tmp/RtmpBnvpFv/file13a8279f411f.dta: Unable to open file.
     ---
     Backtrace:
         ▆
      1. └─haven::read_dta(url)
      2.   └─haven:::read(spec, df_parse_options("dta", encoding))
     ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
     
     Error: processing vignette 'f-statistic-comparison.Rmd' failed with diagnostics:
     Failed to parse /tmp/RtmpBnvpFv/file13a8279f411f.dta: Unable to open file.
     --- failed re-building ‘f-statistic-comparison.Rmd’
     
     SUMMARY: processing the following file failed:
       ‘f-statistic-comparison.Rmd’
     
     Error: Vignette re-building failed.
     Execution halted
     ```

# opendataformat (2.2.2)

* GitHub: <https://github.com/opendataformat/r-package-opendataformat>
* Email: <mailto:tom.hartl96@gmail.com>
* GitHub mirror: <https://github.com/cran/opendataformat>

Run `revdepcheck::cloud_details(, "opendataformat")` for more info

## Newly broken

*   checking tests ... ERROR
     ```
     ...
       [1] "Dataset successfully written to '/tmp/RtmpE0bsc4/MY_XML2.odf.zip'"
       [1] "Dataset successfully written to '/tmp/RtmpE0bsc4/MY_XML.odf.zip'"
       [1] "Dataset successfully written to '/tmp/RtmpE0bsc4/MY_XML.zip'"
       [1] "Dataset successfully written to '/tmp/RtmpE0bsc4/MY_XML.odf.zip'"
       [1] "Dataset successfully written to '/tmp/RtmpE0bsc4/MY_XML.odf.zip'"
       [1] "Dataset successfully written to '/tmp/RtmpE0bsc4/MY_XML.odf.zip'"
       [1] "Dataset successfully written to '/tmp/RtmpE0bsc4/MY_XML.odf.zip'"
       [ FAIL 2 | WARN 2 | SKIP 0 | PASS 695 ]
       
       ══ Failed tests ════════════════════════════════════════════════════════════════
       ── Failure ('test-read-and-write-odf.R:121:3'): read_write_odf_with_extreme values ──
       Expected `sum(df1$large_vals)` to equal `-216868392221803`.
       Differences:
       1/1 mismatches
       [1] NaN - -2.17e+14 == NaN
       ── Failure ('test-read-and-write-odf.R:122:3'): read_write_odf_with_extreme values ──
       Expected `sum(df2$large_vals)` to equal `-216868392221803`.
       Differences:
       1/1 mismatches
       [1] NaN - -2.17e+14 == NaN
       
       [ FAIL 2 | WARN 2 | SKIP 0 | PASS 695 ]
       Error:
       ! Test failures.
       Execution halted
     ```

# parquetize (0.5.8)

* GitHub: <https://github.com/ddotta/parquetize>
* Email: <mailto:damien.dotta@live.fr>
* GitHub mirror: <https://github.com/cran/parquetize>

Run `revdepcheck::cloud_details(, "parquetize")` for more info

## Newly broken

*   checking tests ... ERROR
     ```
     ...
       Writing file159c235cb965-44-86.parquet...
       Reading data...
       Writing file159c235cb965-87-129.parquet...
       Reading data...
       Writing file159c235cb965-130-150.parquet...
       v Data are available in parquet dataset under /tmp/RtmpWHdMHr/file159c235cb965/
       Writing file159c235cb965-130-150.parquet...
       [ FAIL 1 | WARN 1 | SKIP 3 | PASS 203 ]
       
       ══ Skipped tests (3) ═══════════════════════════════════════════════════════════
       • On CRAN (3): 'test-csv_to_parquet.R:54:3', 'test-download_extract.R:16:3',
         'test-download_extract.R:43:3'
       
       ══ Failed tests ════════════════════════════════════════════════════════════════
       ── Failure ('test-utilities.R:33:3'): test get_lines_for_memory return the good number of lines ──
       Expected `get_lines_for_memory(data, max_memory = 1/1024)` to equal 16.
       Differences:
         `actual`:  1.0
       `expected`: 16.0
       
       
       [ FAIL 1 | WARN 1 | SKIP 3 | PASS 203 ]
       Error:
       ! Test failures.
       Execution halted
     ```

# rio (1.3.0)

* GitHub: <https://github.com/gesistsa/rio>
* Email: <mailto:chainsawtiney@gmail.com>
* GitHub mirror: <https://github.com/cran/rio>

Run `revdepcheck::cloud_details(, "rio")` for more info

## Newly broken

*   checking tests ... ERROR
     ```
     ...
       Expected `"format.stata" %in% names(attributes(g))` to be TRUE.
       Differences:
       `actual`:   FALSE
       `expected`: TRUE 
       
       
       ── 4. Failure ('test_gather_attrs.R:23:9'): Spread attrs from Stata ────────────
       Expected `"notes" %in% names(attributes(s))` to be TRUE.
       Differences:
       `actual`:   FALSE
       `expected`: TRUE 
       
       
       ── 5. Failure ('test_gather_attrs.R:25:9'): Spread attrs from Stata ────────────
       Expected `"format.stata" %in% names(attributes(s[[1]]))` to be TRUE.
       Differences:
       `actual`:   FALSE
       `expected`: TRUE 
       
       
       ══ DONE ════════════════════════════════════════════════════════════════════════
       Keep trying!
       Error:
       ! Test failures.
       Execution halted
     ```

# sdtmval (0.4.1)

* GitHub: <https://github.com/skgithub14/sdtmval>
* Email: <mailto:stephen@knappconsultingllc.com>
* GitHub mirror: <https://github.com/cran/sdtmval>

Run `revdepcheck::cloud_details(, "sdtmval")` for more info

## Newly broken

*   checking tests ... ERROR
     ```
     ...
       `attr(expected$dm$RFSTDTC, 'haven_meta')` is absent
       
       `attr(actual$dm$RFXSTDTC, 'haven_meta')` is an S7 object of class <haven::meta_var_sas>
       `attr(expected$dm$RFXSTDTC, 'haven_meta')` is absent
       
       `attr(actual$dm$RFXENDTC, 'haven_meta')` is an S7 object of class <haven::meta_var_sas>
       `attr(expected$dm$RFXENDTC, 'haven_meta')` is absent
       
       ── Failure ('test-write.R:7:3'): write_tbl_to_xpt ──────────────────────────────
       Expected `tbl` to be identical to `check`.
       Differences:
       `attr(actual, 'haven_meta')` is absent
       `attr(expected, 'haven_meta')` is an S7 object of class <haven::meta_df_sas>
       
       `attr(actual$one, 'haven_meta')` is absent
       `attr(expected$one, 'haven_meta')` is an S7 object of class <haven::meta_var_sas>
       
       `attr(actual$two, 'haven_meta')` is absent
       `attr(expected$two, 'haven_meta')` is an S7 object of class <haven::meta_var_sas>
       
       
       [ FAIL 2 | WARN 1 | SKIP 0 | PASS 24 ]
       Error:
       ! Test failures.
       Execution halted
     ```

## In both

*   checking DESCRIPTION meta-information ... NOTE
     ```
       Missing dependency on R >= 4.1.0 because package code uses the pipe
       |> or function shorthand \(...) syntax added in R 4.1.0.
       File(s) using such syntax:
         ‘assign_meta_data.Rd’
     ```

# SynthesizerPlus (0.2.9)

* GitHub: <https://github.com/bogdanoancea/SynthesizerPlus>
* Email: <mailto:bogdan.oancea@gmail.com>
* GitHub mirror: <https://github.com/cran/SynthesizerPlus>

Run `revdepcheck::cloud_details(, "SynthesizerPlus")` for more info

## Newly broken

*   checking tests ... ERROR
     ```
     ...
       `attr(expected, 'haven_meta')` is absent
       
       `attr(actual$date, 'haven_meta')` is an S7 object of class <haven::meta_var_stata>
       `attr(expected$date, 'haven_meta')` is absent
       
       `attr(actual$time, 'haven_meta')` is an S7 object of class <haven::meta_var_stata>
       `attr(expected$time, 'haven_meta')` is absent
       
       ── Failure ('test-io.R:90:5'): SPSS, Stata and SAS transport files round-trip ──
       Expected `roundtrip(ext)` to equal `io_data()`.
       Differences:
       `attr(actual, 'haven_meta')` is an S7 object of class <haven::meta_df_sas>
       `attr(expected, 'haven_meta')` is absent
       
       `attr(actual$date, 'haven_meta')` is an S7 object of class <haven::meta_var_sas>
       `attr(expected$date, 'haven_meta')` is absent
       
       `attr(actual$time, 'haven_meta')` is an S7 object of class <haven::meta_var_sas>
       `attr(expected$time, 'haven_meta')` is absent
       
       
       [ FAIL 3 | WARN 24 | SKIP 7 | PASS 1159 ]
       Error:
       ! Test failures.
       Execution halted
     ```

# tablet (0.9.4)

* GitHub: <https://github.com/bergsmat/tablet>
* Email: <mailto:bergsmat@gmail.com>
* GitHub mirror: <https://github.com/cran/tablet>

Run `revdepcheck::cloud_details(, "tablet")` for more info

## Newly broken

*   checking examples ... ERROR
     ```
     ...
     <tr>
        <td style="text-align:left;padding-left: 2em;" indentlevel="1"> 0 </td>
        <td style="text-align:left;"> 16 (28.1%) </td>
        <td style="text-align:left;"> 92 (68.7%) </td>
        <td style="text-align:left;"> 7 (50%) </td>
        <td style="text-align:left;"> 115 (56.1%) </td>
       </tr>
       <tr>
        <td style="text-align:left;padding-left: 2em;" indentlevel="1"> 1 </td>
        <td style="text-align:left;"> 41 (71.9%) </td>
        <td style="text-align:left;"> 42 (31.3%) </td>
        <td style="text-align:left;"> 7 (50%) </td>
        <td style="text-align:left;"> 90 (43.9%) </td>
       </tr>
     </tbody>
     </table>> 
     > x <- system.file(package = 'tablet', 'shiny-examples/mesa/data/adsl.sas7bdat')
     > x %<>% read_sas %>% data.frame
     > decorations(x) 
     - anyaefl
      - label: Any TEAEs
     Error in as.vector(x, "list") : 
       cannot coerce type 'object' to vector of type 'list'
     Calls: <Anonymous> ... render.default -> sapply -> lapply -> as.list -> as.list.default
     Execution halted
     ```

## In both

*   checking re-building of vignette outputs ... WARNING
     ```
     Error(s) in re-building vignettes:
       ...
     --- re-building ‘tablet-introduction-html.Rmd’ using rmarkdown
     --- finished re-building ‘tablet-introduction-html.Rmd’
     
     --- re-building ‘tablet-introduction-pdf.Rmd’ using rmarkdown
     ! LaTeX Error: File `multirow.sty' not found.
     
     ! Emergency stop.
     <read *> 
     
     Error: processing vignette 'tablet-introduction-pdf.Rmd' failed with diagnostics:
     LaTeX failed to compile tablet-introduction-pdf.tex. See https://yihui.org/tinytex/r/#debugging for debugging tips. See tablet-introduction-pdf.log for more info.
     --- failed re-building ‘tablet-introduction-pdf.Rmd’
     
     SUMMARY: processing the following file failed:
       ‘tablet-introduction-pdf.Rmd’
     
     Error: Vignette re-building failed.
     Execution halted
     ```

# TwoWayFEWeights (2.1.0)

* GitHub: <https://github.com/Credible-Answers/twowayfeweights>
* Email: <mailto:anzony.quispe@gmail.com>
* GitHub mirror: <https://github.com/cran/TwoWayFEWeights>

Run `revdepcheck::cloud_details(, "TwoWayFEWeights")` for more info

## Newly broken

*   checking examples ... ERROR
     ```
     Running examples in ‘TwoWayFEWeights-Ex.R’ failed
     The error most likely occurred in:
     
     > ### Name: print.twowayfeweights
     > ### Title: A print method for twowayfeweights objects
     > ### Aliases: print.twowayfeweights
     > 
     > ### ** Examples
     > 
     > # The following example is based on data from F. Vella and M. Verbeek (1998), 
     > # "Whose Wages Do Unions Raise? A Dynamic Model of Unionism and Wage Rate 
     > # Determination for Young Men". 
     > # Results and further details about the estimation below can be found in 
     > # Section V.C of de Chaisemartin & D'Haultfoeuille (2020a).
     > # Run the following lines to download the dataset in your local working 
     > # directory and load it to your R environment:
     > 
     > repo = "chaisemartinPackages/twowayfeweights/main"
     > file = "wagepan_twfeweights.dta"
     > url = paste("https://raw.githubusercontent.com", repo, file, sep = "/")
     > wagepan =  haven::read_dta(url)
     Error: Failed to parse /tmp/RtmpMqXfyk/file12c616a664dd.dta: Unable to open file.
     Execution halted
     ```

# webuse (0.1.3)

* GitHub: <https://github.com/leeper/webuse>
* Email: <mailto:jodi@alum.mit.edu>
* GitHub mirror: <https://github.com/cran/webuse>

Run `revdepcheck::cloud_details(, "webuse")` for more info

## Newly broken

*   checking examples ... ERROR
     ```
     Running examples in ‘webuse-Ex.R’ failed
     The error most likely occurred in:
     
     > ### Name: webuse
     > ### Title: webuse
     > ### Aliases: webuse webuselist webuse-package
     > 
     > ### ** Examples
     > 
     > webuse("auto")
     Error: Failed to parse /tmp/RtmppvOqok/filecbb335f4568.dta: Unable to open file.
     Execution halted
     ```

*   checking tests ... ERROR
     ```
     ...
       > 
       > test_check("webuse")
       Saving _problems/tests-5.R
       [ FAIL 1 | WARN 0 | SKIP 2 | PASS 0 ]
       
       ══ Skipped tests (2) ═══════════════════════════════════════════════════════════
       • On CRAN (2): 'tests.R:10:3', 'tests.R:18:3'
       
       ══ Failed tests ════════════════════════════════════════════════════════════════
       ── Error ('tests.R:5:3'): Basic functionality ──────────────────────────────────
       Error: Failed to parse /tmp/Rtmp5UkKZO/filecdd7998e986.dta: Unable to open file.
       Backtrace:
           ▆
        1. ├─testthat::expect_true(...) at tests.R:5:3
        2. │ └─testthat::quasi_label(enquo(object), label)
        3. │   └─rlang::eval_bare(expr, quo_get_env(quo))
        4. ├─base::inherits(webuse("auto"), "data.frame")
        5. └─webuse::webuse("auto")
        6.   └─haven::read_dta(...)
        7.     └─haven:::read(spec, df_parse_options("dta", encoding))
       
       [ FAIL 1 | WARN 0 | SKIP 2 | PASS 0 ]
       Error:
       ! Test failures.
       Execution halted
     ```

# xportr (0.6.0)

* GitHub: <https://github.com/atorus-research/xportr>
* Email: <mailto:Eli.Miller@AtorusResearch.com>
* GitHub mirror: <https://github.com/cran/xportr>

Run `revdepcheck::cloud_details(, "xportr")` for more info

## Newly broken

*   checking tests ... ERROR
     ```
     ...
       Expected `read_xpt(tmp)` to equal `local_data`.
       Differences:
       `attr(actual, 'haven_meta')` is an S7 object of class <haven::meta_df_sas>
       `attr(expected, 'haven_meta')` is absent
       
       `attr(actual$E, 'haven_meta')` is an S7 object of class <haven::meta_var_sas>
       `attr(expected$E, 'haven_meta')` is absent
       
       `attr(actual$B, 'haven_meta')` is an S7 object of class <haven::meta_var_sas>
       `attr(expected$B, 'haven_meta')` is absent
       
       `attr(actual$X, 'haven_meta')` is an S7 object of class <haven::meta_var_sas>
       `attr(expected$X, 'haven_meta')` is absent
       
       ── Failure ('test-xportr.R:42:3'): xportr Test 1: pipeline results match `xportr()` results ──
       Expected `haven::read_xpt(pipeline_path)` to be identical to `haven::read_xpt(xportr_path)`.
       Differences:
       `attr(actual, 'haven_meta')@table_name`:   "adslpipe"
       `attr(expected, 'haven_meta')@table_name`: "adslxptr"
       
       
       [ FAIL 3 | WARN 0 | SKIP 5 | PASS 346 ]
       Error:
       ! Test failures.
       Execution halted
     ```

# yamlet (1.3.4)

* GitHub: <https://github.com/bergsmat/yamlet>
* Email: <mailto:bergsmat@gmail.com>
* GitHub mirror: <https://github.com/cran/yamlet>

Run `revdepcheck::cloud_details(, "yamlet")` for more info

## Newly broken

*   checking examples ... ERROR
     ```
     ...
     > file <- system.file(package = 'yamlet', 'extdata','quinidine.csv')
     > x <- decorate(as.csv(file))
     > x %>% select(Subject) %>% decorations
     - Subject: 
     > x %<>% redecorate('Subject: Patient Identifier')
     > x %>% select(Subject) %>% decorations
     - Subject
      - label: Patient Identifier
     > 
     > # xpt may already have labels:
     > 
     > dm <- 'extdata/dm.xpt.gz' %>% 
     +   system.file(package = 'yamlet') %>% 
     +   gzfile %>% 
     +   read_xpt
     > 
     > dm %>% class  
     [1] "tbl_df"     "tbl"        "data.frame"
     > dm %>% decorations(AGE, SEX, RACE)
     - AGE
      - label: Age
     Error in as.vector(x, "list") : 
       cannot coerce type 'object' to vector of type 'list'
     Calls: <Anonymous> ... render.default -> sapply -> lapply -> as.list -> as.list.default
     Execution halted
     ```

## In both

*   checking re-building of vignette outputs ... WARNING
     ```
     ...
       ...
     --- re-building ‘aesthetics.Rmd’ using rmarkdown
     --- finished re-building ‘aesthetics.Rmd’
     
     --- re-building ‘scripted-html.Rmd’ using rmarkdown
     --- finished re-building ‘scripted-html.Rmd’
     
     --- re-building ‘scripted-pdf.Rmd’ using rmarkdown
     ! LaTeX Error: File `multirow.sty' not found.
     
     ! Emergency stop.
     <read *> 
     
     Error: processing vignette 'scripted-pdf.Rmd' failed with diagnostics:
     LaTeX failed to compile scripted-pdf.tex. See https://yihui.org/tinytex/r/#debugging for debugging tips. See scripted-pdf.log for more info.
     --- failed re-building ‘scripted-pdf.Rmd’
     
     --- re-building ‘yamlet-introduction.Rmd’ using rmarkdown
     --- finished re-building ‘yamlet-introduction.Rmd’
     
     SUMMARY: processing the following file failed:
       ‘scripted-pdf.Rmd’
     
     Error: Vignette re-building failed.
     Execution halted
     ```

