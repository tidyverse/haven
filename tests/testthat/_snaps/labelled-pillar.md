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

# pillar

    Code
      x <- labelled(1:11, c(Good = 1, Bad = 8))
      tibble::tibble(x)
    Output
      # A tibble: 11 x 1
                 x
         <int+lbl>
       1  1 [Good]
       2  2       
       3  3       
       4  4       
       5  5       
       6  6       
       7  7       
       8  8 [Bad] 
       9  9       
      10 10       
      11 11       

---

    Code
      x <- labelled(c(rep(c(1.22352, 1000, -345), each = 3), 35, 35), c(One = 1.22352,
        Two = 35, Threeeee = 1000))
      tibble::tibble(x)
    Output
      # A tibble: 11 x 1
                       x
               <dbl+lbl>
       1    1.22 [One]  
       2    1.22 [One]  
       3    1.22 [One]  
       4 1000 [Threeeee]
       5 1000 [Threeeee]
       6 1000 [Threeeee]
       7 -345           
       8 -345           
       9 -345           
      10   35 [Two]     
      11   35 [Two]     

---

    Code
      x <- labelled(c(rep("A", 3), rep("B", 3), rep("XXXXXX", 4), NA), c(Apple = "A",
        Banana = "B", Mystery = "XXXXXX"))
      tibble::tibble(x)
    Output
      # A tibble: 11 x 1
         x               
         <chr+lbl>       
       1 A [Apple]       
       2 A [Apple]       
       3 A [Apple]       
       4 B [Banana]      
       5 B [Banana]      
       6 B [Banana]      
       7 XXXXXX [Mystery]
       8 XXXXXX [Mystery]
       9 XXXXXX [Mystery]
      10 XXXXXX [Mystery]
      11 <NA>            

---

    Code
      x <- labelled(c(1:8, tagged_na("a"), tagged_na("b"), NA), c(Good = 1, Bad = 8,
        Refused = tagged_na("b")))
      tibble::tibble(x)
    Output
      # A tibble: 11 x 1
                       x
               <dbl+lbl>
       1     1 [Good]   
       2     2          
       3     3          
       4     4          
       5     5          
       6     6          
       7     7          
       8     8 [Bad]    
       9 NA(a)          
      10 NA(b) [Refused]
      11    NA          

---

    Code
      x <- labelled_spss(c(1:10, NA), c(Good = 1, Bad = 8, Refused = 10), c(9, 10))
      tibble::tibble(x)
    Output
      # A tibble: 11 x 1
                         x
                 <int+lbl>
       1  1 [Good]        
       2  2               
       3  3               
       4  4               
       5  5               
       6  6               
       7  7               
       8  8 [Bad]         
       9  9 (NA)          
      10 10 (NA) [Refused]
      11 NA               

