
lump_test <- function(x) {
  ifelse(in_smallest(x), "other", letters[seq_along(x)])
}
