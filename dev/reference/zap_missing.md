# Zap special missings to regular R missings

This is useful if you want to convert tagged missing values from SAS or
Stata, or user-defined missings from SPSS, to regular R `NA`.

## Usage

``` r
zap_missing(x)
```

## Arguments

- x:

  A vector or data frame

## Examples

``` r
x1 <- labelled(
  c(1, 5, tagged_na("a", "b")),
  c(Unknown = tagged_na("a"), Refused = tagged_na("b"))
)
x1
#> <labelled<double>[4]>
#> [1]     1     5 NA(a) NA(b)
#> 
#> Labels:
#>  value   label
#>  NA(a) Unknown
#>  NA(b) Refused
zap_missing(x1)
#> <labelled<double>[4]>
#> [1]  1  5 NA NA

x2 <- labelled_spss(
  c(1, 2, 1, 99),
  c(missing = 99),
  na_value = 99
)
x2
#> <labelled_spss<double>[4]>
#> [1]  1  2  1 99
#> Missing values: 99
#> 
#> Labels:
#>  value   label
#>     99 missing
zap_missing(x2)
#> [1]  1  2  1 NA
#> attr(,"labels")
#> missing 
#>      99 
#> attr(,"class")
#> [1] "haven_labelled"

# You can also apply to data frames
df <- tibble::tibble(x1, x2, y = 4:1)
df
#> # A tibble: 4 × 3
#>   x1              x2                    y
#>   <dbl+lbl>       <dbl+lbl>         <int>
#> 1     1            1                    4
#> 2     5            2                    3
#> 3 NA(a) [Unknown]  1                    2
#> 4 NA(b) [Refused] 99 (NA) [missing]     1
zap_missing(df)
#> # A tibble: 4 × 3
#>   x1        x2            y
#>   <dbl+lbl> <dbl+lbl> <int>
#> 1  1         1            4
#> 2  5         2            3
#> 3 NA         1            2
#> 4 NA        NA            1
```
