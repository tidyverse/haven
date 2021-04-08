# nice display in tibbles

    Code
      int <- labelled(1:5, labels = c(good = 1L, bad = 5L))
      dbl <- labelled(1:5 / 10, labels = c(good = 0.1, bad = 0.5))
      chr <- labelled(letters[1:5], labels = c(good = "a", bad = "e"))
      tibble(int, dbl, chr)
    Output
      # A tibble: 5 x 3
              int        dbl chr      
        <int+lbl>  <dbl+lbl> <chr+lbl>
      1  1 [good] 0.1 [good] a [good] 
      2  2        0.2        b        
      3  3        0.3        c        
      4  4        0.4        d        
      5  5 [bad]  0.5 [bad]  e [bad]  

