# Labelled vectors for SPSS

This class is only used when `user_na = TRUE` in
[`read_sav()`](https://haven.tidyverse.org/dev/reference/read_spss.md).
It is similar to the
[`labelled()`](https://haven.tidyverse.org/dev/reference/labelled.md)
class but it also models SPSS's user-defined missings, which can be up
to three distinct values, or for numeric vectors a range.

## Usage

``` r
labelled_spss(
  x = double(),
  labels = NULL,
  na_values = NULL,
  na_range = NULL,
  label = NULL
)
```

## Arguments

- x:

  A vector to label. Must be either numeric (integer or double) or
  character.

- labels:

  A named vector or `NULL`. The vector should be the same type as `x`.
  Unlike factors, labels don't need to be exhaustive: only a fraction of
  the values might be labelled.

- na_values:

  A vector of values that should also be considered as missing.

- na_range:

  A numeric vector of length two giving the (inclusive) extents of the
  range. Use `-Inf` and `Inf` if you want the range to be open ended.

- label:

  A short, human-readable description of the vector.

## Examples

``` r
x1 <- labelled_spss(1:10, c(Good = 1, Bad = 8), na_values = c(9, 10))
is.na(x1)
#>  [1] FALSE FALSE FALSE FALSE FALSE FALSE FALSE FALSE  TRUE  TRUE

x2 <- labelled_spss(
  1:10,
  c(Good = 1, Bad = 8),
  na_range = c(9, Inf),
  label = "Quality rating"
)
is.na(x2)
#>  [1] FALSE FALSE FALSE FALSE FALSE FALSE FALSE FALSE  TRUE  TRUE

# Print data and metadata
x2
#> <labelled_spss<integer>[10]>: Quality rating
#>  [1]  1  2  3  4  5  6  7  8  9 10
#> Missing range:  [9, Inf]
#> 
#> Labels:
#>  value label
#>      1  Good
#>      8   Bad
```
