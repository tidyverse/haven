# Zap variable labels

Removes variable label, leaving unlabelled vectors as is.

## Usage

``` r
zap_label(x)
```

## Arguments

- x:

  A vector or data frame

## See also

[`zap_labels()`](https://haven.tidyverse.org/dev/reference/zap_labels.md)
to remove value labels.

Other zappers:
[`zap_empty()`](https://haven.tidyverse.org/dev/reference/zap_empty.md),
[`zap_formats()`](https://haven.tidyverse.org/dev/reference/zap_formats.md),
[`zap_labels()`](https://haven.tidyverse.org/dev/reference/zap_labels.md),
[`zap_widths()`](https://haven.tidyverse.org/dev/reference/zap_widths.md)

## Examples

``` r
x1 <- labelled(1:5, c(good = 1, bad = 5), label = "rating")
x1
#> <labelled<integer>[5]>: rating
#> [1] 1 2 3 4 5
#> 
#> Labels:
#>  value label
#>      1  good
#>      5   bad
zap_label(x1)
#> <labelled<integer>[5]>
#> [1] 1 2 3 4 5
#> 
#> Labels:
#>  value label
#>      1  good
#>      5   bad

x2 <- labelled_spss(c(1:4, 9), label = "score", na_values = 9)
x2
#> <labelled_spss<double>[5]>: score
#> [1] 1 2 3 4 9
#> Missing values: 9
zap_label(x2)
#> <labelled_spss<double>[5]>
#> [1] 1 2 3 4 9
#> Missing values: 9

# zap_label also works with data frames
df <- tibble::tibble(x1, x2)
str(df)
#> tibble [5 × 2] (S3: tbl_df/tbl/data.frame)
#>  $ x1: int+lbl [1:5] 1, 2, 3, 4, 5
#>    ..@ labels: Named int [1:2] 1 5
#>    .. ..- attr(*, "names")= chr [1:2] "good" "bad"
#>    ..@ label : chr "rating"
#>  $ x2: dbl+lbl [1:5] 1, 2, 3, 4, 9
#>    ..@ label    : chr "score"
#>    ..@ na_values: num 9
str(zap_label(df))
#> tibble [5 × 2] (S3: tbl_df/tbl/data.frame)
#>  $ x1: int+lbl [1:5] 1, 2, 3, 4, 5
#>    ..@ labels: Named int [1:2] 1 5
#>    .. ..- attr(*, "names")= chr [1:2] "good" "bad"
#>  $ x2: dbl+lbl [1:5] 1, 2, 3, 4, 9
#>    ..@ na_values: num 9
```
