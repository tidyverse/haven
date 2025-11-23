# Read and write SAS transport files

The SAS transport format is a open format, as is required for submission
of the data to the FDA.

## Usage

``` r
read_xpt(
  file,
  col_select = NULL,
  skip = 0,
  n_max = Inf,
  .name_repair = "unique"
)

write_xpt(
  data,
  path,
  version = 8,
  name = NULL,
  label = attr(data, "label"),
  adjust_tz = TRUE
)
```

## Arguments

- file:

  Either a path to a file, a connection, or literal data (either a
  single string or a raw vector).

  Files ending in `.gz`, `.bz2`, `.xz`, or `.zip` will be automatically
  uncompressed. Files starting with `http://`, `https://`, `ftp://`, or
  `ftps://` will be automatically downloaded. Remote gz files can also
  be automatically downloaded and decompressed.

  Literal data is most useful for examples and tests. To be recognised
  as literal data, the input must be either wrapped with
  [`I()`](https://rdrr.io/r/base/AsIs.html), be a string containing at
  least one new line, or be a vector containing at least one string with
  a new line.

  Using a value of
  [`clipboard()`](https://readr.tidyverse.org/reference/clipboard.html)
  will read from the system clipboard.

- col_select:

  One or more selection expressions, like in `dplyr::select()`. Use
  [`c()`](https://rdrr.io/r/base/c.html) or
  [`list()`](https://rdrr.io/r/base/list.html) to use more than one
  expression. See `?dplyr::select` for details on available selection
  options. Only the specified columns will be read from `data_file`.

- skip:

  Number of lines to skip before reading data.

- n_max:

  Maximum number of lines to read.

- .name_repair:

  Treatment of problematic column names:

  - `"minimal"`: No name repair or checks, beyond basic existence,

  - `"unique"`: Make sure names are unique and not empty,

  - `"check_unique"`: (default value), no name repair, but check they
    are `unique`,

  - `"universal"`: Make the names `unique` and syntactic

  - `"unique_quiet"`: Same as `"unique"`, but "quiet"

  - `"universal_quiet"`: Same as `"universal"`, but "quiet"

  - a function: apply custom name repair (e.g.,
    `.name_repair = make.names` for names in the style of base R).

  - A purrr-style anonymous function, see
    [`rlang::as_function()`](https://rlang.r-lib.org/reference/as_function.html)

  This argument is passed on as `repair` to
  [`vctrs::vec_as_names()`](https://vctrs.r-lib.org/reference/vec_as_names.html).
  See there for more details on these terms and the strategies used to
  enforce them.

- data:

  Data frame to write.

- path:

  Path to a file where the data will be written.

- version:

  Version of transport file specification to use: either 5 or 8.

- name:

  Member name to record in file. Defaults to file name sans extension.
  Must be \<= 8 characters for version 5, and \<= 32 characters for
  version 8.

- label:

  Dataset label to use, or `NULL`. Defaults to the value stored in the
  "label" attribute of `data`.

  Note that although SAS itself supports dataset labels up to 256
  characters long, dataset labels in SAS transport files must be \<= 40
  characters.

- adjust_tz:

  Stata, SPSS and SAS do not have a concept of time zone, and all
  [date-time](https://rdrr.io/r/base/DateTimeClasses.html) variables are
  treated as UTC. `adjust_tz` controls how the timezone of date-time
  values is treated when writing.

  - If `TRUE` (the default) the timezone of date-time values is ignored,
    and they will display the same in R and Stata/SPSS/SAS, e.g.
    `"2010-01-01 09:00:00 NZDT"` will be written as
    `"2010-01-01 09:00:00"`. Note that this changes the underlying
    numeric data, so use caution if preserving between-time-point
    differences is critical.

  - If `FALSE`, date-time values are written as the corresponding UTC
    value, e.g. `"2010-01-01 09:00:00 NZDT"` will be written as
    `"2009-12-31 20:00:00"`.

## Value

A tibble, data frame variant with nice defaults.

Variable labels are stored in the "label" attribute of each variable. It
is not printed on the console, but the RStudio viewer will show it.

If a dataset label is defined, it will be stored in the "label"
attribute of the tibble.

`write_xpt()` returns the input `data` invisibly.

## Details

Note that character limits are expressed in bytes. The number of bytes
will often be the same as the number of characters , but strings with
multibyte character sequences will count some symbols as more than one
character. For example, the string "café" is 5 bytes long in UTF-8.

## Examples

``` r
tmp <- tempfile(fileext = ".xpt")
write_xpt(mtcars, tmp)
read_xpt(tmp)
#> # A tibble: 32 × 11
#>      mpg   cyl  disp    hp  drat    wt  qsec    vs    am  gear  carb
#>    <dbl> <dbl> <dbl> <dbl> <dbl> <dbl> <dbl> <dbl> <dbl> <dbl> <dbl>
#>  1  21       6  160    110  3.9   2.62  16.5     0     1     4     4
#>  2  21       6  160    110  3.9   2.88  17.0     0     1     4     4
#>  3  22.8     4  108     93  3.85  2.32  18.6     1     1     4     1
#>  4  21.4     6  258    110  3.08  3.22  19.4     1     0     3     1
#>  5  18.7     8  360    175  3.15  3.44  17.0     0     0     3     2
#>  6  18.1     6  225    105  2.76  3.46  20.2     1     0     3     1
#>  7  14.3     8  360    245  3.21  3.57  15.8     0     0     3     4
#>  8  24.4     4  147.    62  3.69  3.19  20       1     0     4     2
#>  9  22.8     4  141.    95  3.92  3.15  22.9     1     0     4     2
#> 10  19.2     6  168.   123  3.92  3.44  18.3     1     0     4     4
#> # ℹ 22 more rows
```
