
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

roundtrip_xpt <- function(x) {
  tmp <- tempfile()
  on.exit(unlink(tmp))

  write_xpt(x, tmp)
  zap_formats(read_xpt(tmp))
}


roundtrip_var <- function(x, type = "sav") {
  df <- tibble::tibble(x = x)

  # Forces xpt files to be correct length even when ending with
  # empty character strings
  if (type == "xpt") {
    df$y <- seq_along(x)
  }

  switch(type,
    sav = roundtrip_sav(df)$x,
    dta = roundtrip_dta(df)$x,
    sas = roundtrip_sas(df)$x,
    xpt = roundtrip_xpt(df)$x,
    stop("Unsupported type")
  )
}
