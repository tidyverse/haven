# Conversion semantics

There are some differences between the way that R, SAS, SPSS, and Stata
represented labelled data and missing values. While SAS, SPSS, and Stata
share some obvious similarities, R is little different. This vignette
explores the differences, and shows you how haven bridges the gap.

## Value labels

Base R has one data type that effectively maintains a mapping between
integers and character labels: the factor. This however, is not the
primary use of factors: they are instead designed to automatically
generate useful contrasts for linear models. Factors differ from the
labelled values provided by the other tools in important ways:

- SPSS and SAS can label numeric and character values, not just integer
  values.

- The value do not need to be exhaustive. It is common to label the
  special missing values (e.g. `.D` = did not respond, `.N` = not
  applicable), while leaving other values as is.

Value labels in SAS are a little different again. In SAS, labels are
just special case of general formats. Formats include currencies and
dates, but user-defined just assigns labels to individual values
(including special missings value). Formats have names and existing
independently of the variables they are associated with. You create a
named format with `PROC FORMAT` and then associated with variables in a
`DATA` step (the names of character formats thealways start with `$`).

### `labelled()`

To allow you to import labelled vectors into R, haven provides the S3
labelled class, created with
[`labelled()`](https://haven.tidyverse.org/dev/reference/labelled.md).
This class allows you to associated arbitrary labels with numeric or
character vectors:

``` r
x1 <- labelled(
  sample(1:5), 
  c(Good = 1, Bad = 5)
)
x1
#> <labelled<integer>[5]>
#> [1] 5 3 4 1 2
#> 
#> Labels:
#>  value label
#>      1  Good
#>      5   Bad

x2 <- labelled(
  c("M", "F", "F", "F", "M"), 
  c(Male = "M", Female = "F")
)
x2
#> <labelled<character>[5]>
#> [1] M F F F M
#> 
#> Labels:
#>  value  label
#>      M   Male
#>      F Female
```

The goal of haven is not to provide a labelled vector that you can use
everywhere in your analysis. The goal is to provide an intermediate
datastructure that you can convert into a regular R data frame. You can
do this by either converting to a factor or stripping the labels:

``` r
as_factor(x1)
#> [1] Bad  3    4    Good 2   
#> Levels: Good 2 3 4 Bad
zap_labels(x1)
#> [1] 5 3 4 1 2

as_factor(x2)
#> [1] Male   Female Female Female Male  
#> Levels: Female Male
zap_labels(x2)
#> [1] "M" "F" "F" "F" "M"
```

See the documentation for
[`as_factor()`](https://haven.tidyverse.org/dev/reference/as_factor.md)
for more options to control exactly what the factor uses for levels.

Both
[`as_factor()`](https://haven.tidyverse.org/dev/reference/as_factor.md)
and
[`zap_labels()`](https://haven.tidyverse.org/dev/reference/zap_labels.md)
have data frame methods if you want to apply the same strategy to every
column in a data frame:

``` r
df <- tibble::data_frame(x1, x2, z = 1:5)
#> Warning: `data_frame()` was deprecated in tibble 1.1.0.
#> ℹ Please use `tibble()` instead.
#> This warning is displayed once per session.
#> Call `lifecycle::last_lifecycle_warnings()` to see where this warning
#> was generated.
df
#> # A tibble: 5 × 3
#>   x1        x2             z
#>   <int+lbl> <chr+lbl>  <int>
#> 1 5 [Bad]   M [Male]       1
#> 2 3         F [Female]     2
#> 3 4         F [Female]     3
#> 4 1 [Good]  F [Female]     4
#> 5 2         M [Male]       5

zap_labels(df)
#> # A tibble: 5 × 3
#>      x1 x2        z
#>   <int> <chr> <int>
#> 1     5 M         1
#> 2     3 F         2
#> 3     4 F         3
#> 4     1 F         4
#> 5     2 M         5
as_factor(df)
#> # A tibble: 5 × 3
#>   x1    x2         z
#>   <fct> <fct>  <int>
#> 1 Bad   Male       1
#> 2 3     Female     2
#> 3 4     Female     3
#> 4 Good  Female     4
#> 5 2     Male       5
```

## Missing values

All three tools provide a global “system missing value” which is
displayed as `.`. This is roughly equivalent to R’s `NA`, although
neither Stata nor SAS propagate missingness in numeric comparisons: SAS
treats the missing value as the smallest possible number (i.e. `-inf`),
and Stata treats it as the largest possible number (i.e. `inf`).

Each tool also provides a mechanism for recording multiple types of
missingness:

- Stata has “extended” missing values, `.A` through `.Z`.

- SAS has “special” missing values, `.A` through `.Z` plus `._`.

- SPSS has per-column “user” missing values. Each column can declare up
  to three distinct values or a range of values (plus one distinct
  value) that should be treated as missing.

Stata and SAS only support tagged missing values for numeric columns.
SPSS supports up to three distinct values for character columns.
Generally, operations involving a user-missing type return a system
missing value.

Haven models these missing values in two different ways:

- For SAS and Stata, haven provides “tagged” missing values which extend
  R’s regular `NA` to add a single character label.

- For SPSS, haven provides a subclass of `labelled` that also provides
  user defined values and ranges.

### Tagged missing values

To support Stata’s extended and SAS’s special missing value, haven
implements a tagged NA. It does this by taking advantage of the internal
structure of a floating point NA. That allows these values to behave
identical to NA in regular R operations, while still preserving the
value of the tag.

The R interface for creating with tagged NAs is a little clunky because
generally they’ll be created by haven for you. But you can create your
own with
[`tagged_na()`](https://haven.tidyverse.org/dev/reference/tagged_na.md):

``` r
x <- c(1:3, tagged_na("a", "z"), 3:1)
x
#> [1]  1  2  3 NA NA  3  2  1
```

Note these tagged NAs behave identically to regular NAs, even when
printing. To see their tags, use
[`print_tagged_na()`](https://haven.tidyverse.org/dev/reference/tagged_na.md):

``` r
print_tagged_na(x)
#> [1]     1     2     3 NA(a) NA(z)     3     2     1
```

To test if a value is a tagged NA, use
[`is_tagged_na()`](https://haven.tidyverse.org/dev/reference/tagged_na.md),
and to extract the value of the tag, use
[`na_tag()`](https://haven.tidyverse.org/dev/reference/tagged_na.md):

``` r
is_tagged_na(x)
#> [1] FALSE FALSE FALSE  TRUE  TRUE FALSE FALSE FALSE
is_tagged_na(x, "a")
#> [1] FALSE FALSE FALSE  TRUE FALSE FALSE FALSE FALSE

na_tag(x)
#> [1] NA  NA  NA  "a" "z" NA  NA  NA
```

My expectation is that tagged missings are most often used in conjuction
with labels (described below), so labelled vectors print the tags for
you, and
[`as_factor()`](https://haven.tidyverse.org/dev/reference/as_factor.md)
knows how to relabel:

``` r
y <- labelled(x, c("Not home" = tagged_na("a"), "Refused" = tagged_na("z")))
y
#> <labelled<double>[8]>
#> [1]     1     2     3 NA(a) NA(z)     3     2     1
#> 
#> Labels:
#>  value    label
#>  NA(a) Not home
#>  NA(z)  Refused

as_factor(y)
#> [1] 1        2        3        Not home Refused  3        2       
#> [8] 1       
#> Levels: 1 2 3 Not home Refused
```

### User defined missing values

SPSS’s user-defined values work differently to SAS and Stata. Each
column can have either up to three distinct values that are considered
as missing, or a range. Haven provides
[`labelled_spss()`](https://haven.tidyverse.org/dev/reference/labelled_spss.md)
as a subclass of
[`labelled()`](https://haven.tidyverse.org/dev/reference/labelled.md) to
model these additional user-defined missings.

``` r
x1 <- labelled_spss(c(1:10, 99), c(Missing = 99), na_value = 99)
x2 <- labelled_spss(c(1:10, 99), c(Missing = 99), na_range = c(90, Inf))

x1
#> <labelled_spss<double>[11]>
#>  [1]  1  2  3  4  5  6  7  8  9 10 99
#> Missing values: 99
#> 
#> Labels:
#>  value   label
#>     99 Missing
x2
#> <labelled_spss<double>[11]>
#>  [1]  1  2  3  4  5  6  7  8  9 10 99
#> Missing range:  [90, Inf]
#> 
#> Labels:
#>  value   label
#>     99 Missing
```

These objects are somewhat dangerous to work with in R because most R
functions don’t know those values are missing:

``` r
mean(x1)
#> [1] 14
```

Because of that danger, the default behaviour of
[`read_spss()`](https://haven.tidyverse.org/dev/reference/read_spss.md)
is to return regular labelled objects where user-defined missing values
have been converted to `NA`s. To get
[`read_spss()`](https://haven.tidyverse.org/dev/reference/read_spss.md)
to return
[`labelled_spss()`](https://haven.tidyverse.org/dev/reference/labelled_spss.md)
objects, you’ll need to set `user_na = TRUE`.

I’ve defined an [`is.na()`](https://rdrr.io/r/base/NA.html) method so
you can find them yourself:

``` r
is.na(x1)
#>  [1] FALSE FALSE FALSE FALSE FALSE FALSE FALSE FALSE FALSE FALSE  TRUE
```

And the presence of that method does mean many functions with an `na.rm`
argument will work correctly:

``` r
mean(x1, na.rm = TRUE)
#> [1] 14
```

But generally you should either convert to a factor, convert to regular
missing vaues, or strip the all the labels:

``` r
as_factor(x1)
#>  [1] 1       2       3       4       5       6       7       8      
#>  [9] 9       10      Missing
#> Levels: 1 2 3 4 5 6 7 8 9 10 Missing
zap_missing(x1)
#>  [1]  1  2  3  4  5  6  7  8  9 10 NA
#> attr(,"labels")
#> Missing 
#>      99 
#> attr(,"class")
#> [1] "haven_labelled"
zap_labels(x1)
#>  [1]  1  2  3  4  5  6  7  8  9 10 NA
```
