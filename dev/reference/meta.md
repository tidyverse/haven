# File metadata

`meta()` gets and sets the metadata attached to a vector or data frame
by
[`read_sas()`](https://haven.tidyverse.org/dev/reference/read_sas.md),
[`read_spss()`](https://haven.tidyverse.org/dev/reference/read_spss.md),
or
[`read_stata()`](https://haven.tidyverse.org/dev/reference/read_dta.md).
The metadata is stored in a single `haven_meta` attribute holding an S7
object whose class depends on the vendor and on whether `x` is a data
frame:

- Column metadata:
  [meta_var_sas](https://haven.tidyverse.org/dev/reference/meta_var_sas.md),
  [meta_var_spss](https://haven.tidyverse.org/dev/reference/meta_var_spss.md),
  [meta_var_stata](https://haven.tidyverse.org/dev/reference/meta_var_stata.md).

- Data frame metadata:
  [meta_df_sas](https://haven.tidyverse.org/dev/reference/meta_var_sas.md),
  [meta_df_spss](https://haven.tidyverse.org/dev/reference/meta_var_spss.md),
  [meta_df_stata](https://haven.tidyverse.org/dev/reference/meta_var_stata.md).

If no metadata has been set, `meta()` falls back to the legacy
`format.*`, `display_width`, `width`, `notes`, and `*_timestamp`
attributes, so objects saved by older versions of haven keep working.

## Usage

``` r
meta(x, vendor = NULL)

meta(x, vendor = NULL) <- value
```

## Arguments

- x:

  A vector or data frame.

- vendor:

  If supplied, one of `"sas"`, `"spss"`, or `"stata"`. Returns an empty
  object of the appropriate class if no metadata is present, and errors
  if the existing metadata belongs to another vendor.

- value:

  `NULL` or a metadata object of the appropriate class. Setting metadata
  (including `NULL`) also removes the legacy attributes, so they can't
  resurface later.

## Value

`meta()` returns an S7 object inheriting from `meta_var` (for vectors)
or `meta_df` (for data frames), or `NULL`.

## Examples

``` r
df <- read_sav(system.file("examples", "iris.sav", package = "haven"))

# Data frame metadata
meta(df)
#> <haven::meta_df_spss>
#> @created : 2016-06-10 11:25:39 UTC
#> @modified: 2016-06-10 11:25:39 UTC
meta(df)@created
#> [1] "2016-06-10 11:25:39 UTC"

# Column metadata
meta(df$Species)
#> <haven::meta_var_spss>
#> @format       : "F8.0"
#> @display_width: 8
#> @measure      : "nominal"
#> @alignment    : "right"
meta(df$Species)@format
#> [1] "F8.0"

# Modify metadata
meta(df$Species)@measure <- "ordinal"
meta(df$Species)
#> <haven::meta_var_spss>
#> @format       : "F8.0"
#> @display_width: 8
#> @measure      : "ordinal"
#> @alignment    : "right"
```
