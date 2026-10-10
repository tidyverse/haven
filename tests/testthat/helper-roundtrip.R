# Removes file metadata attributes (creation/modified timestamps)
# that vary from run to run
zap_file_metadata <- function(x) {
  attr(x, "creation_timestamp") <- NULL
  attr(x, "modified_timestamp") <- NULL
  x
}

roundtrip_sav <- function(x, ...) {
  tmp <- tempfile()
  on.exit(unlink(tmp))

  write_sav(x, tmp, ...)
  zap_file_metadata(zap_formats(read_sav(tmp)))
}

roundtrip_dta <- function(x, ...) {
  tmp <- tempfile()
  on.exit(unlink(tmp))

  write_dta(x, tmp, ...)
  zap_file_metadata(zap_formats(read_dta(tmp)))
}

roundtrip_sas <- function(x, ...) {
  tmp <- tempfile()
  on.exit(unlink(tmp))

  write_sas(x, tmp, ...)
  zap_file_metadata(zap_formats(read_sas(tmp)))
}

roundtrip_xpt <- function(x, ...) {
  tmp <- tempfile()
  on.exit(unlink(tmp))

  write_xpt(x, tmp, ...)
  zap_file_metadata(zap_formats(read_xpt(tmp)))
}


roundtrip_var <- function(x, type = "sav", ...) {
  df <- tibble::tibble(x = x)

  # Forces xpt files to be correct length even when ending with
  # empty character strings
  if (type == "xpt") {
    df$y <- seq_along(x)
  }

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

  write_dta(tibble::tibble(x = x), path1)
  write_dta(tibble::tibble(x = rep(x, 2)), path2)
  (file.size(path2) - file.size(path1)) / length(x)
}
