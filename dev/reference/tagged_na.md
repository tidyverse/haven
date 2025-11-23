# "Tagged" missing values

"Tagged" missing values work exactly like regular R missing values
except that they store one additional byte of information a tag, which
is usually a letter ("a" to "z"). When by loading a SAS and Stata file,
the tagged missing values always use lower case values.

## Usage

``` r
tagged_na(...)

na_tag(x)

is_tagged_na(x, tag = NULL)

format_tagged_na(x, digits = getOption("digits"))

print_tagged_na(x, digits = getOption("digits"))
```

## Arguments

- ...:

  Vectors containing single character. The letter will be used to "tag"
  the missing value.

- x:

  A numeric vector

- tag:

  If `NULL`, will only return true if the tag has this value.

- digits:

  Number of digits to use in string representation

## Details

`format_tagged_na()` and `print_tagged_na()` format tagged NA's as
NA(a), NA(b), etc.

## Examples

``` r
x <- c(1:5, tagged_na("a"), tagged_na("z"), NA)

# Tagged NA's work identically to regular NAs
x
#> [1]  1  2  3  4  5 NA NA NA
is.na(x)
#> [1] FALSE FALSE FALSE FALSE FALSE  TRUE  TRUE  TRUE

# To see that they're special, you need to use na_tag(),
# is_tagged_na(), or print_tagged_na():
is_tagged_na(x)
#> [1] FALSE FALSE FALSE FALSE FALSE  TRUE  TRUE FALSE
na_tag(x)
#> [1] NA  NA  NA  NA  NA  "a" "z" NA 
print_tagged_na(x)
#> [1]     1     2     3     4     5 NA(a) NA(z)    NA

# You can test for specific tagged NAs with the second argument
is_tagged_na(x, "a")
#> [1] FALSE FALSE FALSE FALSE FALSE  TRUE FALSE FALSE

# Because the support for tagged's NAs is somewhat tagged on to R,
# the left-most NA will tend to be preserved in arithmetic operations.
na_tag(tagged_na("a") + tagged_na("z"))
#> [1] "a"
```
