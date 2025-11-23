# Convert labelled vectors to factors

The base function [`as.factor()`](https://rdrr.io/r/base/factor.html) is
not a generic, but
[`forcats::as_factor()`](https://forcats.tidyverse.org/reference/as_factor.html)
is. haven provides `as_factor()` methods for
[`labelled()`](https://haven.tidyverse.org/dev/reference/labelled.md)
and
[`labelled_spss()`](https://haven.tidyverse.org/dev/reference/labelled_spss.md)
vectors, and data frames. By default, when applied to a data frame, it
only affects labelled columns.

## Usage

``` r
# S3 method for class 'data.frame'
as_factor(x, ..., only_labelled = TRUE)

# S3 method for class 'haven_labelled'
as_factor(
  x,
  levels = c("default", "labels", "values", "both"),
  ordered = FALSE,
  ...
)

# S3 method for class 'labelled'
as_factor(
  x,
  levels = c("default", "labels", "values", "both"),
  ordered = FALSE,
  ...
)
```

## Arguments

- x:

  Object to coerce to a factor.

- ...:

  Other arguments passed down to method.

- only_labelled:

  Only apply to labelled columns?

- levels:

  How to create the levels of the generated factor:

  - "default": uses labels where available, otherwise the values. Labels
    are sorted by value.

  - "both": like "default", but pastes together the level and value

  - "label": use only the labels; unlabelled values become `NA`

  - "values": use only the values

- ordered:

  If `TRUE` create an ordered (ordinal) factor, if `FALSE` (the default)
  create a regular (nominal) factor.

## Details

Includes methods for both class `haven_labelled` and `labelled` for
backward compatibility.

## Examples

``` r
x <- labelled(sample(5, 10, replace = TRUE), c(Bad = 1, Good = 5))

# Default method uses values where available
as_factor(x)
#>  [1] 4    Good 4    Bad  Good Good Good 2    3    3   
#> Levels: Bad 2 3 4 Good
# You can also extract just the labels
as_factor(x, levels = "labels")
#>  [1] <NA> Good <NA> Bad  Good Good Good <NA> <NA> <NA>
#> Levels: Bad Good
# Or just the values
as_factor(x, levels = "values")
#>  [1] 4 5 4 1 5 5 5 2 3 3
#> Levels: 1 2 3 4 5
# Or combine value and label
as_factor(x, levels = "both")
#>  [1] 4        [5] Good 4        [1] Bad  [5] Good [5] Good [5] Good
#>  [8] 2        3        3       
#> Levels: [1] Bad 2 3 4 [5] Good

# as_factor() will preserve SPSS missing values from values and ranges
y <- labelled_spss(1:10, na_values = c(2, 4), na_range = c(8, 10))
as_factor(y)
#>  [1] 1  2  3  4  5  6  7  8  9  10
#> Levels: 1 2 3 4 5 6 7 8 9 10
# use zap_missing() first to convert to NAs
zap_missing(y)
#>  [1]  1 NA  3 NA  5  6  7 NA NA NA
#> attr(,"class")
#> [1] "haven_labelled"
as_factor(zap_missing(y))
#>  [1] 1    <NA> 3    <NA> 5    6    7    <NA> <NA> <NA>
#> Levels: 1 3 5 6 7
```
