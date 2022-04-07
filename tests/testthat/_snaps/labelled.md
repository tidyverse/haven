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

# printed output is stable

    Code
      x
    Output
      <labelled<double>[9]>
      [1]     1     2     3     4     5    NA NA(x) NA(y) NA(z)
      
      Labels:
       value             label
           1              Good
           5               Bad
       NA(x)    Not Applicable
       NA(y) Refused to answer

