#' @useDynLib haven
#' @importFrom Rcpp sourceCpp
NULL

#' Read SAS files.
#'
#' This supports both b7dat files and the accompanying b7cat files
#' that are SAS's equivalent of factor labels.
#'
#' @param b7dat,b7cat Path to data and catalog files. If the path is a URL, the
#'   file will be first download to a temporary location before reading.
#' @return A data frame with additional "tbl_df" and "tbl" classes, which
#'   improve printing if dplyr is loaded.
#'
#'   Variable labels are stored in the "label" attribute of each variable.
#'   It is not printed on the console, but the RStudio viewer will show it.
#' @export
read_sas <- function(b7dat, b7cat = NULL) {
  df_parse_sas(clean_path(b7dat), clean_path(b7cat))
}

#' Read SPSS (SAV) files. Write SAV files.
#'
#' Currently haven can read and write logical, integer, numeric, character
#' and factors. See \code{\link{labelled}} for how labelled variables in
#' Stata are handled in R. \code{read_spss} is an alias for \code{read_sav}.
#'
#' @param path Path to data. When reading data, if the path is a URL, the file
#'   will be first downloaded to a temporary location before reading.
#' @param data Data frame to write.
#' @return A data frame with additional "tbl_df" and "tbl" classes, which
#'   improve printing if dplyr is loaded.
#'
#'   Variable labels are stored in the "label" attribute of each variable.
#'   It is not printed on the console, but the RStudio viewer will show it.
#' @name read_spss
#' @examples
#' tmp <- tempfile(fileext = ".sav")
#' write_sav(mtcars, tmp)
#' read_sav(tmp)
NULL

#' @export
#' @rdname read_spss
read_sav <- function(path) {
  df_parse_sav(clean_path(path))
}

#' @export
#' @rdname read_spss
write_sav <- function(data, path) {
  write_sav_(data, normalizePath(path, mustWork = FALSE))
}


#' @export
#' @rdname read_spss
read_spss <- function(path) {
  ext <- tolower(tools::file_ext(path))

  switch(ext,
    sav = read_sav(path),
    stop("Unknown extension '.",  ext, "'", call. = FALSE)
  )
}

#' Read and write Stata DTA files.
#'
#' Currently haven can read and write logical, integer, numeric, character
#' and factors. See \code{\link{labelled}} for how labelled variables in
#' Stata are handled in R.
#'
#' @inheritParams read_spss
#' @param encoding The character encoding used for the file. This defaults to
#'   the encoding specified in the file, or UTF-8. But older versions of Stata
#'   (13 and earlier) did not store the encoding used, and you'll need to
#'   specify manually. A commonly used value is "Win 1252".
#' @return A data frame with additional "tbl_df" and "tbl" classes, which
#'   improve printing if dplyr is loaded.
#'
#'   Variable labels are stored in the "label" attribute of each variable.
#'   It is not printed on the console, but the RStudio viewer will show it.
#' @export
#' @examples
#' tmp <- tempfile(fileext = ".dta")
#' write_dta(mtcars, tmp)
#' read_dta(tmp)
#' read_stata(tmp)
read_dta <- function(path, encoding = NULL) {
  if (is.null(encoding)) {
    encoding <- ""
  }
  df_parse_dta(clean_path(path), encoding)
}

#' @export
#' @rdname read_dta
read_stata <- function(path, encoding = NULL) {
  read_dta(path, encoding)
}

#' @export
#' @rdname read_dta
write_dta <- function(data, path) {
  validate_dta(data)
  write_dta_(data, normalizePath(path, mustWork = FALSE))
}

validate_dta <- function(data) {
  # Check variable names
  bad_names <- !grepl("^[a-zA-Z0-9_]+$", names(data))
  if (any(bad_names)) {
    stop(
      "The following variable names are not valid Stata variables: ",
      var_names(data, bad_names),
      call. = FALSE
    )
  }

  # Check for labelled double vectors
  is_labelled <- vapply(data, is.labelled, logical(1))
  is_integer <- vapply(data, typeof, character(1)) == "integer"
  bad_labels <- is_labelled && !is_integer
  if (any(bad_labels)) {
    stop(
      "Stata only supports labelled integers.\nProblems: ",
      var_names(data, bad_labels),
      call. = FALSE
    )
  }

  # Check lengths of labels
  lengths <- vapply(data, label_length, integer(1))
  bad_lengths <- lengths > 32
    if (any(bad_lengths)) {
    stop(
      "Stata only supports value labels up to 32 characters in length. \nProblems: ",
      var_names(data, bad_lengths),
      call. = FALSE
    )
  }

}

var_names <- function(data, i) {
  x <- names(data)[i]
  paste(encodeString(x, quote = "`"), collapse = ", ")
}
