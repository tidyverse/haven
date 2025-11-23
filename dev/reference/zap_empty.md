# Convert empty strings into missing values

Convert empty strings into missing values

## Usage

``` r
zap_empty(x)
```

## Arguments

- x:

  A character vector

## Value

A character vector with empty strings replaced by missing values.

## See also

Other zappers:
[`zap_formats()`](https://haven.tidyverse.org/dev/reference/zap_formats.md),
[`zap_label()`](https://haven.tidyverse.org/dev/reference/zap_label.md),
[`zap_labels()`](https://haven.tidyverse.org/dev/reference/zap_labels.md),
[`zap_widths()`](https://haven.tidyverse.org/dev/reference/zap_widths.md)

## Examples

``` r
x <- c("a", "", "c")
zap_empty(x)
#> [1] "a" NA  "c"
```
