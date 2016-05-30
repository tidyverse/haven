
roundtrip <- function(x) {
  tmp <- tempfile()
  write_sav(x, tmp)
  read_sav(tmp)
}

roundtrip_var <- function(x) {
  df <- list(x = x)
  class(df) <- "data.frame"
  attr(df, "row.names") <- .set_row_names(length(x))
  roundtrip(df)$x
}
