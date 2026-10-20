# SAS metadata

`meta_var_sas` (column) and `meta_df_sas` (data frame) hold metadata
read from and written to SAS files. See
[`meta()`](https://haven.tidyverse.org/dev/reference/meta.md) for how to
access them.

## Usage

``` r
meta_var_sas(format = NULL, informat = NULL, width = NULL)

meta_df_sas(
  created = NULL,
  modified = NULL,
  table_name = NULL,
  xpt_version = NULL
)
```

## Arguments

- format:

  The display format (e.g. `"DATE9."`).

- informat:

  The input format.

- width:

  The variable width.

- created, modified:

  File creation and modification timestamps.

- table_name:

  The SAS table name.

- xpt_version:

  The transport file version (5 or 8).

## Examples

``` r
df <- read_sas(system.file("examples", "iris.sas7bdat", package = "haven"))
meta(df)
#> <haven::meta_df_sas>
#> @created   : 2016-06-08 22:38:50 UTC
#> @modified  : 2016-06-08 22:38:50 UTC
#> @table_name: "IRIS"
meta(df)@table_name
#> [1] "IRIS"
meta(df$Species)
#> <haven::meta_var_sas>
#> @format  : "$6"
#> @informat: "$6"
#> @width   : 6
meta(df$Species)@format
#> [1] "$6"
```
