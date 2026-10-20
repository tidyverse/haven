# Remove format attributes

**\[deprecated\]**

This function is deprecated because haven now stores formats in the
`haven_meta` attribute (see
[`meta()`](https://haven.tidyverse.org/dev/reference/meta.md)). Please
use
[`zap_meta()`](https://haven.tidyverse.org/dev/reference/zap_meta.md)
instead.

`zap_formats()` only removes the legacy `format.stata`, `format.spss`,
and `format.sas` attributes.

## Usage

``` r
zap_formats(x)
```

## Arguments

- x:

  A vector or data frame.

## See also

Other zappers:
[`zap_empty()`](https://haven.tidyverse.org/dev/reference/zap_empty.md),
[`zap_label()`](https://haven.tidyverse.org/dev/reference/zap_label.md),
[`zap_labels()`](https://haven.tidyverse.org/dev/reference/zap_labels.md),
[`zap_meta()`](https://haven.tidyverse.org/dev/reference/zap_meta.md),
[`zap_widths()`](https://haven.tidyverse.org/dev/reference/zap_widths.md)

## Examples

``` r
x <- structure(1:3, format.sas = "BEST12.")

# Old:
zap_formats(x)
#> Warning: `zap_formats()` was deprecated in haven 2.6.0.
#> ℹ Please use `zap_meta()` instead.
#> [1] 1 2 3
# New:
zap_meta(x)
#> [1] 1 2 3
```
