# Read SAS files

`read_sas()` supports both sas7bdat files and the accompanying sas7bcat
files that SAS uses to record value labels.

## Usage

``` r
read_sas(
  data_file,
  catalog_file = NULL,
  encoding = NULL,
  catalog_encoding = encoding,
  col_select = NULL,
  skip = 0L,
  n_max = Inf,
  cols_only = deprecated(),
  .name_repair = "unique"
)
```

## Arguments

- data_file, catalog_file:

  Path to data and catalog files. The files are processed with
  [`readr::datasource()`](https://readr.tidyverse.org/reference/datasource.html).

- encoding, catalog_encoding:

  The character encoding used for the `data_file` and `catalog_encoding`
  respectively. A value of `NULL` uses the encoding specified in the
  file; use this argument to override it if it is incorrect.

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

- cols_only:

  **\[deprecated\]** `cols_only` is no longer supported; use
  `col_select` instead.

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

## Value

A tibble, data frame variant with nice defaults.

Variable labels are stored in the "label" attribute of each variable. It
is not printed on the console, but the RStudio viewer will show it.

[`write_sas()`](https://haven.tidyverse.org/dev/reference/write_sas.md)
returns the input `data` invisibly.

## Examples

``` r
path <- system.file("examples", "iris.sas7bdat", package = "haven")
read_sas(path)
#> # A tibble: 150 × 5
#>    Sepal_Length Sepal_Width Petal_Length Petal_Width Species
#>           <dbl>       <dbl>        <dbl>       <dbl> <chr>  
#>  1          5.1         3.5          1.4         0.2 setosa 
#>  2          4.9         3            1.4         0.2 setosa 
#>  3          4.7         3.2          1.3         0.2 setosa 
#>  4          4.6         3.1          1.5         0.2 setosa 
#>  5          5           3.6          1.4         0.2 setosa 
#>  6          5.4         3.9          1.7         0.4 setosa 
#>  7          4.6         3.4          1.4         0.3 setosa 
#>  8          5           3.4          1.5         0.2 setosa 
#>  9          4.4         2.9          1.4         0.2 setosa 
#> 10          4.9         3.1          1.5         0.1 setosa 
#> # ℹ 140 more rows
```
