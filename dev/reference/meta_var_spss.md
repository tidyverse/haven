# SPSS metadata

`meta_var_spss` (column) and `meta_df_spss` (data frame) hold metadata
read from and written to SPSS files. See
[`meta()`](https://haven.tidyverse.org/dev/reference/meta.md) for how to
access them.

## Usage

``` r
meta_var_spss(
  format = NULL,
  display_width = NULL,
  width = NULL,
  measure = NULL,
  alignment = NULL
)

meta_df_spss(created = NULL, modified = NULL, notes = NULL, mr_sets = NULL)
```

## Arguments

- format:

  The display format.

- display_width:

  The display width.

- width:

  The variable width.

- measure:

  The measure level: `"nominal"`, `"ordinal"`, or `"scale"`.

- alignment:

  The display alignment: `"left"`, `"right"`, or `"center"`.

- created, modified:

  File creation and modification timestamps.

- notes:

  File notes.

- mr_sets:

  Multiple response sets.

## Examples

``` r
df <- read_sav(system.file("examples", "iris.sav", package = "haven"))
meta(df)
#> <haven::meta_df_spss>
#> @created : 2016-06-10 11:25:39 UTC
#> @modified: 2016-06-10 11:25:39 UTC
meta(df)@created
#> [1] "2016-06-10 11:25:39 UTC"
meta(df$Species)
#> <haven::meta_var_spss>
#> @format       : "F8.0"
#> @display_width: 8
#> @measure      : "nominal"
#> @alignment    : "right"
meta(df$Species)@measure
#> [1] "nominal"
```
