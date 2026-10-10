# nocov start
# Update the vendored copy of ReadStat in src/readstat.
#
# We track the dev branch because ReadStat releases are slow, and then apply
# the small set of local patches in patches/readstat/. Each patch is a git
# diff with a header explaining what it does and why; patches that backport an
# upstream fix are named after the ReadStat PR and link to the PR and the
# haven issue they fix. Remove a patch once it's no longer needed (e.g. its
# PR is merged into the branch we track).
update_readstat <- function(branch = "dev") {
  tmp <- tempfile()
  utils::download.file(
    paste0("https://github.com/WizardMac/ReadStat/archive/", branch, ".zip"),
    tmp,
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

  apply_readstat_patches()

  invisible()
}

apply_readstat_patches <- function() {
  patch_dir <- fs::path("patches", "readstat")
  if (!fs::dir_exists(patch_dir)) {
    return(invisible())
  }

  for (patch in sort(fs::dir_ls(patch_dir, glob = "*.patch"))) {
    cli::cli_inform("Applying {.file {patch}}")
    # Patch paths are a/src/... relative to the ReadStat repo root; strip a/
    # and src/ so they apply inside src/readstat
    status <- system2(
      "git",
      c(
        "apply",
        "-p2",
        "--directory",
        shQuote(fs::path("src", "readstat")),
        shQuote(patch)
      )
    )
    if (!identical(status, 0L)) {
      cli::cli_abort(c(
        "Failed to apply {.file {patch}}.",
        i = "Is it no longer needed (e.g. merged upstream)? If so, delete it."
      ))
    }
  }

  invisible()
}
# nocov end
