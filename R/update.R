update_readstat <- function() {
  tmp <- tempfile()
  utils::download.file("https://github.com/WizardMac/ReadStat/archive/master.zip", tmp,
    method = "wget")
  utils::unzip(tmp, exdir = tempdir())

  zip_dir <- file.path(tempdir(), "ReadStat-master", "src")
  src <- dir(zip_dir, "\\.[ch]$", recursive = TRUE)

  # Drop test & bin
  ignore <- dirname(src) %in% c("test", "bin", "bin/modules", "bin/util")
  src <- src[!ignore]


  dirs <- file.path("src", "readstat", c("sas", "stata", "spss"))
  lapply(dirs, dir.create, showWarnings = FALSE, recursive = TRUE)

  ok <- file.copy(file.path(zip_dir, src), file.path("src", "readstat", src), overwrite = TRUE)

  if (any(!ok)) {
    stop("Failed to copy: ", paste(src[!ok], collapse = ", "), call. = FALSE)
  }

  invisible()
}
