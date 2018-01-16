max_level_length <- function(x) {
  if (!is.factor(x))
    return(0L)

  max(0L, nchar(levels(x)), na.rm = TRUE)
}
