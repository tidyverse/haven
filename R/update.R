# nocov start
update_readstat <- function() {
  tmp <- tempfile()
  utils::download.file(
    "https://github.com/WizardMac/ReadStat/archive/master.zip", tmp,
    method = "wget"
  )
  base <- fs::path_common(utils::unzip(tmp, exdir = tempdir()))

  in_dir <- fs::path(base, "src")
  out_dir <- fs::path("src", "readstat")

  fs::dir_delete(out_dir)
  fs::dir_copy(in_dir, out_dir)
  fs::dir_delete(fs::path(out_dir, c("bin", "fuzz", "test")))

  invisible()
}
# nocov end
