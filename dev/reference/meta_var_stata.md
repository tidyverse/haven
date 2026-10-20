# Stata metadata

`meta_var_stata` (column) and `meta_df_stata` (data frame) hold metadata
read from and written to Stata files. See
[`meta()`](https://haven.tidyverse.org/dev/reference/meta.md) for how to
access them.

## Usage

``` r
meta_var_stata(format = NULL, type = NULL)

meta_df_stata(created = NULL, modified = NULL, notes = NULL, version = NULL)
```

## Arguments

- format:

  The display format.

- type:

  The storage type: `"byte"`, `"int"`, `"long"`, `"float"`, `"double"`,
  or `"strL"`.

- created, modified:

  File creation and modification timestamps.

- notes:

  File notes.

- version:

  The Stata file version.

## Examples

``` r
df <- read_dta(system.file("examples", "iris.dta", package = "haven"))
meta(df)
#> <haven::meta_df_stata>
#> @created : 2016-06-10 13:00:00 UTC
#> @modified: 2016-06-10 13:00:00 UTC
#> @version : 14
meta(df)@version
#> [1] 14
meta(df$species)
#> <haven::meta_var_stata>
#> @format: "%10s"
meta(df$species)@format
#> [1] "%10s"
```
