# Remove format attributes

To provide some mild support for round-tripping variables between
Stata/SPSS and R, haven stores variable formats in an attribute:
`format.stata`, `format.spss`, or `format.sas`. If this causes problems
for your code, you can get rid of them with `zap_formats`.

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
[`zap_widths()`](https://haven.tidyverse.org/dev/reference/zap_widths.md)
