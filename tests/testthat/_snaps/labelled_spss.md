# constructor checks na_value

    Code
      labelled_spss(1:10, na_values = "a")
    Condition
      Error in `vec_cast_named()`:
      ! Can't convert `na_values` <character> to match type of `x` <integer>.
    Code
      labelled_spss(1:10, na_values = NA_integer_)
    Condition
      Error in `new_labelled_spss()`:
      ! `na_values` can not contain missing values.

# constructor checks na_range

    Code
      labelled_spss(1:10, na_range = "a")
    Condition
      Error in `new_labelled_spss()`:
      ! `na_range` must be a vector of length two the same type as `x`.
    Code
      labelled_spss(1:10, na_range = 1:3)
    Condition
      Error in `new_labelled_spss()`:
      ! `na_range` must be a vector of length two the same type as `x`.
    Code
      labelled_spss(1:10, na_range = c(2, NA))
    Condition
      Error in `new_labelled_spss()`:
      ! `na_range` can not contain missing values.
    Code
      labelled_spss(1:10, na_range = c(2, 1))
    Condition
      Error in `new_labelled_spss()`:
      ! `na_range` must be in ascending order.

# printed output is stable

    Code
      x
    Output
      <labelled_spss<integer>[5]>
      [1] 1 2 3 4 5
      Missing values: 1, 2
      Missing range:  [3, Inf]
      
      Labels:
       value label
           1  Good
           5   Bad

# take labels from LHS

    `..1` and `..2` have conflicting value labels.
    i Labels for these values will be taken from `..1`.
    x Values: 1 and 5

---

    `..1` and `..2` have conflicting value labels.
    i Labels for these values will be taken from `..1`.
    x Values: 1

# warn only for conflicting labels

    `..1` and `..2` have conflicting value labels.
    i Labels for these values will be taken from `..1`.
    x Values: 1 and 2

---

    `..1` and `..2` have conflicting value labels.
    i Labels for these values will be taken from `..1`.
    x Values: 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, ...

