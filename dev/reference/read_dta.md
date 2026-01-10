# Read and write Stata DTA files

Currently haven can read and write logical, integer, numeric, character
and factors. See
[`labelled()`](https://haven.tidyverse.org/dev/reference/labelled.md)
for how labelled variables in Stata are handled in R.

Character vectors will be stored as `strL` if any components are
`strl_threshold` bytes or longer (and `version` \>= 13); otherwise they
will be stored as the appropriate `str#`.

## Usage

``` r
read_dta(
  file,
  encoding = NULL,
  col_select = NULL,
  skip = 0,
  n_max = Inf,
  .name_repair = "unique"
)

read_stata(
  file,
  encoding = NULL,
  col_select = NULL,
  skip = 0,
  n_max = Inf,
  .name_repair = "unique"
)

write_dta(
  data,
  path,
  version = 14,
  label = attr(data, "label"),
  strl_threshold = 2045,
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

- encoding:

  The character encoding used for the file. Generally, only needed for
  Stata 13 files and earlier. See Encoding section for details.

- col_select:

  Columns to include in the results. You can use the same mini-language
  as `dplyr::select()` to refer to the columns by name. Use
  [`c()`](https://rdrr.io/r/base/c.html) to use more than one selection
  expression. Although this usage is less common, `col_select` also
  accepts a numeric column index. See
  [`?tidyselect::language`](https://tidyselect.r-lib.org/reference/language.html)
  for full details on the selection language.

  Predicates using
  [`where()`](https://tidyselect.r-lib.org/reference/where.html) are not
  supported.

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

  File version to use. Supports versions 8-15. See the "Stata versions"
  section below for details on compatibility.

- label:

  Dataset label to use, or `NULL`. Defaults to the value stored in the
  "label" attribute of `data`. Must be \<= 80 characters.

- strl_threshold:

  Any character vectors with a maximum length greater than
  `strl_threshold` bytes will be stored as a long string (strL) instead
  of a standard string (str#) variable if `version` \>= 13. This
  defaults to 2045, the maximum length of str# variables. See the Stata
  [long string](https://www.stata.com/features/overview/long-strings/)
  documentation for more details.

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

If a dataset label is defined in Stata, it will stored in the "label"
attribute of the tibble.

`write_dta()` returns the input `data` invisibly.

## Character encoding

Prior to Stata 14, files did not declare a text encoding, and the
default encoding differed across platforms. If `encoding = NULL`, haven
assumes the encoding is windows-1252, the text encoding used by Stata on
Windows. Unfortunately Stata on Mac and Linux use a different default
encoding, "latin1". If you encounter an error such as "Unable to convert
string to the requested encoding", try `encoding = "latin1"`

For Stata 14 and later, you should not need to manually specify
`encoding` value unless the value was incorrectly recorded in the source
file.

## Stata versions

The `version` argument controls which Stata file format is used, which
determines compatibility with different versions and flavours of Stata.
The following table shows the supported versions:

|             |                 |                   |                         |
|-------------|-----------------|-------------------|-------------------------|
| **version** | **File format** | **Stata version** | **Compatible flavours** |
| 8-9         | 113             | Stata 8-9         | BE, SE, MP              |
| 10-11       | 114             | Stata 10-11       | BE, SE, MP              |
| 12          | 115             | Stata 12          | BE, SE, MP              |
| 13          | 117             | Stata 13          | BE, SE, MP              |
| 14          | 118             | Stata 14+         | BE, SE, MP              |
| 15          | 119             | Stata 15+         | MP only                 |

In most cases you should use the default `version = 14`:

- Versions 8-14 create files readable by all flavours of Stata
  (Stata/BE, Stata/SE, and Stata/MP).

- Version 15 creates format 119 files, designed for datasets exceeding
  32,767 variables. These files can only be opened by Stata/MP, even if
  your dataset has fewer variables. Stata/BE and Stata/SE will refuse to
  open these files with an error like "dataset too large".

Only use `version = 15` if you need to write a dataset with more than
32,767 variables and your recipients have access to Stata/MP.

## Examples

``` r
path <- system.file("examples", "iris.dta", package = "haven")
read_dta(path)
#> # A tibble: 150 × 5
#>    sepallength sepalwidth petallength petalwidth species
#>          <dbl>      <dbl>       <dbl>      <dbl> <chr>  
#>  1        5.10       3.5         1.40      0.200 setosa 
#>  2        4.90       3           1.40      0.200 setosa 
#>  3        4.70       3.20        1.30      0.200 setosa 
#>  4        4.60       3.10        1.5       0.200 setosa 
#>  5        5          3.60        1.40      0.200 setosa 
#>  6        5.40       3.90        1.70      0.400 setosa 
#>  7        4.60       3.40        1.40      0.300 setosa 
#>  8        5          3.40        1.5       0.200 setosa 
#>  9        4.40       2.90        1.40      0.200 setosa 
#> 10        4.90       3.10        1.5       0.100 setosa 
#> # ℹ 140 more rows

tmp <- tempfile(fileext = ".dta")
write_dta(mtcars, tmp)
read_dta(tmp)
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
read_stata(tmp)
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
