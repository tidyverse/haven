# constructor checks na_range

    Code
      labelled_spss(1:10, na_range = "a")
    Error <rlang_error>
      `na_range` must be a vector of length two the same type as `x`.
    Code
      labelled_spss(1:10, na_range = 1:3)
    Error <rlang_error>
      `na_range` must be a vector of length two the same type as `x`.

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

