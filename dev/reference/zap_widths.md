# Remove display width attributes

**\[deprecated\]**

This function is deprecated because haven now stores display widths in
the `haven_meta` attribute (see
[`meta()`](https://haven.tidyverse.org/dev/reference/meta.md)). Please
use
[`zap_meta()`](https://haven.tidyverse.org/dev/reference/zap_meta.md)
instead.

`zap_widths()` only removes the legacy `display_width` attribute.

## Usage

``` r
zap_widths(x)
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
[`zap_meta()`](https://haven.tidyverse.org/dev/reference/zap_meta.md)

## Examples

``` r
x <- structure(1:3, display_width = 10)

# Old:
zap_widths(x)
#> Warning: `zap_widths()` was deprecated in haven 2.6.0.
#> ℹ Please use `zap_meta()` instead.
#> [1] 1 2 3
# New:
zap_meta(x)
#> [1] 1 2 3
```
