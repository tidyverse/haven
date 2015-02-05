
clean_path <- function(path) {
  if (grepl("^(http|ftp|https)://", path)) {
    tmp <- tempfile()
    download.file(path, tmp, quiet = TRUE, mode = "wb")
    tmp
  } else {
    normalizePath(path, mustWork = TRUE)
  }
}
