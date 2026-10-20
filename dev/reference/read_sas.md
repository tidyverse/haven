# Read and write SAS files

`read_sas()` supports both sas7bdat files and the accompanying sas7bcat
files that SAS uses to record value labels. `write_sas()` creates
sas7bdat files; it does not write value labels.

The sas7bdat format is closed and undocumented, so if you need an open
format, instead consider
[`write_xpt()`](https://haven.tidyverse.org/dev/reference/read_xpt.md),
the SAS transport format.

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
  progress = NULL,
  .name_repair = "unique"
)

write_sas(data, path)
```

## Arguments

- data_file, catalog_file:

  Data and catalog files. Each can be a path, a URL, a connection, or a
  raw vector.

- encoding, catalog_encoding:

  The character encoding used for the `data_file` and `catalog_file`
  respectively. A value of `NULL` uses the encoding specified in the
  file; use this argument to override it if it is incorrect. Catalog
  files often don't specify an encoding, in which case the encoding of
  `data_file` is used.

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

  Set to 0 to return just metadata, i.e. data frame containing correctly
  typed (but empty) columns along with all data frame and variable
  metadata.

- progress:

  Display a progress bar? The default, `NULL`, only displays it in an
  interactive session. The automatic progress bar can be disabled with
  `options(haven.show_progress = FALSE)`.

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

  Data frame to write. File and variable metadata stored with
  [`meta()`](https://haven.tidyverse.org/dev/reference/meta.md) is used
  where possible; see
  [`meta_df_sas()`](https://haven.tidyverse.org/dev/reference/meta_var_sas.md)
  and
  [`meta_var_sas()`](https://haven.tidyverse.org/dev/reference/meta_var_sas.md)
  for the fields that affect the output.

- path:

  Path to file where the data will be written.

## Value

A tibble, data frame variant with nice defaults.

Variable labels are stored in the "label" attribute of each variable. It
is not printed on the console, but the RStudio viewer will show it.

Value labels are preserved with the
[`labelled()`](https://haven.tidyverse.org/dev/reference/labelled.md)
class. Labelled vectors are an intermediate representation that
preserves the original value labels, not a regular R factor. Convert
labelled categorical variables with
[`as_factor()`](https://haven.tidyverse.org/dev/reference/as_factor.md),
or remove value labels with
[`zap_labels()`](https://haven.tidyverse.org/dev/reference/zap_labels.md)
if you need plain R vectors for analysis. See
[`vignette("semantics")`](https://haven.tidyverse.org/dev/articles/semantics.md)
for more details.

The data frame and each variable also carry metadata read from the file;
access it with
[`meta()`](https://haven.tidyverse.org/dev/reference/meta.md). See
[`meta_var_sas()`](https://haven.tidyverse.org/dev/reference/meta_var_sas.md),
[`meta_var_spss()`](https://haven.tidyverse.org/dev/reference/meta_var_spss.md),
and
[`meta_var_stata()`](https://haven.tidyverse.org/dev/reference/meta_var_stata.md)
for the format-specific fields.

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

tmp <- tempfile(fileext = ".sas7bdat")
write_sas(mtcars, tmp)
read_sas(tmp)
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
