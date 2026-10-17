roundtrip_sav <- function(x, ...) {
  tmp <- tempfile()
  on.exit(unlink(tmp))

  write_sav(x, tmp, ...)
  zap_meta(read_sav(tmp))
}

roundtrip_dta <- function(x, ...) {
  tmp <- tempfile()
  on.exit(unlink(tmp))

  write_dta(x, tmp, ...)
  zap_meta(read_dta(tmp))
}

roundtrip_sas <- function(x, ...) {
  tmp <- tempfile()
  on.exit(unlink(tmp))

  write_sas(x, tmp, ...)
  zap_meta(read_sas(tmp))
}

roundtrip_xpt <- function(x, ...) {
  tmp <- tempfile()
  on.exit(unlink(tmp))

  write_xpt(x, tmp, ...)
  zap_meta(read_xpt(tmp))
}


roundtrip_var <- function(x, type = "sav", ...) {
  df <- tibble::tibble(x = x)

  switch(
    type,
    sav = roundtrip_sav(df, ...)$x,
    dta = roundtrip_dta(df, ...)$x,
    sas = roundtrip_sas(df, ...)$x,
    xpt = roundtrip_xpt(df, ...)$x,
    stop("Unsupported type")
  )
}

# Bytes used to store each value of x in a dta file
dta_width <- function(x) {
  path1 <- tempfile()
  path2 <- tempfile()
  on.exit(unlink(c(path1, path2)))

  write_dta(data.frame(x = x), path1)
  write_dta(data.frame(x = rep(x, 2)), path2)
  (file.size(path2) - file.size(path1)) / length(x)
}
