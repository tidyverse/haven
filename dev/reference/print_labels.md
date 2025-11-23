# Print the labels of a labelled vector

This is a convenience function, useful to explore the variables of a
newly imported dataset.

## Usage

``` r
print_labels(x, name = NULL)
```

## Arguments

- x:

  A labelled vector

- name:

  The name of the vector (optional)

## Examples

``` r
s1 <- labelled(c("M", "M", "F"), c(Male = "M", Female = "F"))
s2 <- labelled(c(1, 1, 2), c(Male = 1, Female = 2))
labelled_df <- tibble::tibble(s1, s2)

for (var in names(labelled_df)) {
  print_labels(labelled_df[[var]], var)
}
#> 
#> Labels:s1
#>  value  label
#>      M   Male
#>      F Female
#> 
#> Labels:s2
#>  value  label
#>      1   Male
#>      2 Female
```
