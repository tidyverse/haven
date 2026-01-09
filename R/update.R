# nocov start
update_readstat <- function(branch = "master") {
  tmp <- tempfile()
  utils::download.file(
    paste0("https://github.com/WizardMac/ReadStat/archive/", branch, ".zip"), tmp,
    quiet = TRUE
  )
  base <- fs::path_common(utils::unzip(tmp, exdir = tempdir()))

  in_dir <- fs::path(base, "src")
  out_dir <- fs::path("src", "readstat")

  fs::dir_delete(out_dir)
  fs::dir_copy(in_dir, out_dir)
  fs::dir_delete(fs::path(out_dir, c("bin", "fuzz", "test")))

  fs::file_copy(fs::path(base, "LICENSE"), out_dir)
  fs::file_copy(fs::path(base, "NEWS"), out_dir)

  apply_iconv_hack()

  invisible()
}

apply_iconv_hack <- function() {
  path <- fs::path("src", "readstat", "readstat_iconv.h")
  lines <- readLines(path)

  # Replace the autotools ICONV_CONST fallback with platform-specific logic
  # Also update the comment to reflect that we're manually hacking this
  ifndef_line <- which(lines == "#ifndef ICONV_CONST")
  comment_line <- grep("^/\\* ICONV_CONST", lines)

  if (length(ifndef_line) == 1 && length(comment_line) == 1) {
    lines <- c(
      lines[1:(comment_line - 1)],
      "/* ICONV_CONST defined by autotools; so we hack this in manually */",
      "#if defined(_WIN32) || defined(__sun)",
      "  #define ICONV_CONST const",
      "#else",
      "  #define ICONV_CONST",
      "#endif",
      lines[(ifndef_line + 3):length(lines)]
    )
    writeLines(lines, path)
  }
}
# nocov end
