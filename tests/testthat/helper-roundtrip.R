
roundtrip_sav <- function(x) {
  tmp <- tempfile()
  on.exit(unlink(tmp))

  write_sav(x, tmp)
  zap_formats(read_sav(tmp))
}

roundtrip_dta <- function(x) {
  tmp <- tempfile()
  on.exit(unlink(tmp))

  write_dta(x, tmp)
  zap_formats(read_dta(tmp))
}

roundtrip_sas <- function(x) {
  tmp <- tempfile()
  on.exit(unlink(tmp))

  write_sas(x, tmp)
  zap_formats(read_sas(tmp))
}

roundtrip_var <- function(x, type = "sav") {
  df <- list(x = x)
  class(df) <- "data.frame"
  attr(df, "row.names") <- .set_row_names(length(x))

  switch(type,
    sav = roundtrip_sav(df)$x,
    dta = roundtrip_dta(df)$x,
    sas = roundtrip_sas(df)$x,
    stop("Unsupported type")
  )
}
