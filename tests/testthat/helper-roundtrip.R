# Writes and reads back, keeping metadata
roundtrip_file <- function(x, type, ...) {
  tmp <- tempfile()
  on.exit(unlink(tmp))

  switch(
    type,
    sav = {
      write_sav(x, tmp, ...)
      read_sav(tmp)
    },
    dta = {
      write_dta(x, tmp, ...)
      read_dta(tmp)
    },
    sas = {
      write_sas(x, tmp, ...)
      read_sas(tmp)
    },
    xpt = {
      write_xpt(x, tmp, ...)
      read_xpt(tmp)
    },
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

roundtrip_sav <- function(x, ...) zap_meta(roundtrip_file(x, "sav", ...))
roundtrip_dta <- function(x, ...) zap_meta(roundtrip_file(x, "dta", ...))
roundtrip_sas <- function(x, ...) zap_meta(roundtrip_file(x, "sas", ...))
roundtrip_xpt <- function(x, ...) zap_meta(roundtrip_file(x, "xpt", ...))

roundtrip_var <- function(x, type = "sav", ...) {
  df <- tibble::tibble(x = x)
  zap_meta(roundtrip_file(df, type, ...))$x
}
