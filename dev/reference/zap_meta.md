# Remove metadata

Removes the metadata that haven uses to round-trip variables and data
frames between R and SAS/SPSS/Stata (see
[`meta()`](https://haven.tidyverse.org/dev/reference/meta.md)). This
removes the `haven_meta` attribute as well as the legacy `format.sas`,
`format.spss`, `format.stata`, `display_width`, and `width` attributes
on columns and `notes`, `creation_timestamp`, and `modified_timestamp`
attributes on data frames.

Variable labels, value labels, and user-defined missing values are not
affected; use
[`zap_label()`](https://haven.tidyverse.org/dev/reference/zap_label.md),
[`zap_labels()`](https://haven.tidyverse.org/dev/reference/zap_labels.md),
and
[`zap_missing()`](https://haven.tidyverse.org/dev/reference/zap_missing.md)
to remove those.

## Usage

``` r
zap_meta(x)
```

## Arguments

- x:

  A vector or data frame.

## See also

Other zappers:
[`zap_empty()`](https://haven.tidyverse.org/dev/reference/zap_empty.md),
[`zap_formats()`](https://haven.tidyverse.org/dev/reference/zap_formats.md),
[`zap_label()`](https://haven.tidyverse.org/dev/reference/zap_label.md),
[`zap_labels()`](https://haven.tidyverse.org/dev/reference/zap_labels.md),
[`zap_widths()`](https://haven.tidyverse.org/dev/reference/zap_widths.md)

## Examples

``` r
x <- 1:3
meta(x) <- meta_var_sas(format = "BEST12.")
meta(x)
#> <haven::meta_var_sas>
#> @format: "BEST12."
meta(zap_meta(x))
#> NULL

# zap_meta() also works with data frames, removing metadata from the
# data frame and every column
df <- tibble::tibble(x)
meta(df) <- meta_df_sas(table_name = "DATA")
df <- zap_meta(df)
meta(df)
#> NULL
meta(df$x)
#> NULL
```
