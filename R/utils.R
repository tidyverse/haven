
clean_path <- function(path) {
  if (is.null(path)) {
    ""
  } else if (grepl("^(http|ftp|https)://", path)) {
    tmp <- tempfile()
    download.file(path, tmp, quiet = TRUE, mode = "wb")
    tmp
  } else {
    normalizePath(path, mustWork = TRUE)
  }
}

`%||%` <- function(x, y) {
  if (is.null(x)) y else x
}

autoname <- function(x) {
  stats::setNames(x, x)
}
