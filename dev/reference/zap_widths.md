# Remove display width attributes

To provide some mild support for round-tripping variables between SPSS
and R, haven stores display widths in an attribute: `display_width`. If
this causes problems for your code, you can get rid of them with
`zap_widths`.

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
[`zap_labels()`](https://haven.tidyverse.org/dev/reference/zap_labels.md)
