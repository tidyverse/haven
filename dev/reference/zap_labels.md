# Zap value labels

Removes value labels, leaving unlabelled vectors as is. Use this if you
want to simply drop all `labels` from a data frame.

Zapping labels from
[`labelled_spss()`](https://haven.tidyverse.org/dev/reference/labelled_spss.md)
also removes user-defined missing values by default, replacing with
standard `NA`s. Use the `user_na` argument to override this behaviour.

## Usage

``` r
zap_labels(x, ...)

# S3 method for class 'haven_labelled_spss'
zap_labels(x, ..., user_na = FALSE)
```

## Arguments

- x:

  A vector or data frame

- ...:

  Other arguments passed down to method.

- user_na:

  If `FALSE`, the default, `zap_labels()` will convert
  [`labelled_spss()`](https://haven.tidyverse.org/dev/reference/labelled_spss.md)
  user-defined missing values to `NA`. If `TRUE` they will be treated
  like normal values.

## See also

[`zap_label()`](https://haven.tidyverse.org/dev/reference/zap_label.md)
to remove variable labels.

Other zappers:
[`zap_empty()`](https://haven.tidyverse.org/dev/reference/zap_empty.md),
[`zap_formats()`](https://haven.tidyverse.org/dev/reference/zap_formats.md),
[`zap_label()`](https://haven.tidyverse.org/dev/reference/zap_label.md),
[`zap_widths()`](https://haven.tidyverse.org/dev/reference/zap_widths.md)

## Examples

``` r
x1 <- labelled(1:5, c(good = 1, bad = 5))
x1
#> <labelled<integer>[5]>
#> [1] 1 2 3 4 5
#> 
#> Labels:
#>  value label
#>      1  good
#>      5   bad
zap_labels(x1)
#> [1] 1 2 3 4 5

x2 <- labelled_spss(c(1:4, 9), c(good = 1, bad = 5), na_values = 9)
x2
#> <labelled_spss<double>[5]>
#> [1] 1 2 3 4 9
#> Missing values: 9
#> 
#> Labels:
#>  value label
#>      1  good
#>      5   bad
zap_labels(x2)
#> [1]  1  2  3  4 NA

# Keep the user defined missing values
zap_labels(x2, user_na = TRUE)
#> [1] 1 2 3 4 9

# zap_labels also works with data frames
df <- tibble::tibble(x1, x2)
df
#> # A tibble: 5 × 2
#>   x1        x2       
#>   <int+lbl> <dbl+lbl>
#> 1 1 [good]  1 [good] 
#> 2 2         2        
#> 3 3         3        
#> 4 4         4        
#> 5 5 [bad]   9 (NA)   
zap_labels(df)
#> # A tibble: 5 × 2
#>      x1    x2
#>   <int> <dbl>
#> 1     1     1
#> 2     2     2
#> 3     3     3
#> 4     4     4
#> 5     5    NA
```
