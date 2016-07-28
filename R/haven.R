#' @useDynLib haven
#' @importFrom Rcpp sourceCpp
#' @importFrom tibble tibble
NULL

#' Read SAS files.
#'
#' This supports both b7dat files and the accompanying b7cat files
#' that are SAS's equivalent of factor labels.
#'
#' @param b7dat,b7cat Path to data and catalog files. The files are
#'   processed with \code{\link[readr]{datasource}()}.
#' @param encoding The character encoding used for the file. This defaults to
#'   the encoding specified in the file, or UTF-8. You can use this argument
#'   to override the value stored in the file if it is correct
#' @return A tibble, data frame variant with nice defaults.
#'
#'   Variable labels are stored in the "label" attribute of each variable.
#'   It is not printed on the console, but the RStudio viewer will show it.
#' @export
#' @examples
#' path <- system.file("examples", "iris.sas7bdat", package = "haven")
#' read_sas(path)
read_sas <- function(b7dat, b7cat = NULL, encoding = NULL) {
  if (is.null(encoding)) {
    encoding <- ""
  }

  spec_b7dat <- readr::datasource(b7dat)
  if (is.null(b7cat)) {
    spec_b7cat <- list()
  } else {
    spec_b7cat <- readr::datasource(b7cat)
  }
  switch(class(spec_b7dat)[1],
    source_file = df_parse_sas_file(spec_b7dat, spec_b7cat, encoding = encoding),
    source_raw = df_parse_sas_raw(spec_b7dat, spec_b7cat, encoding = encoding),
    stop("This kind of input is not handled", call. = FALSE)
  )
}

#' Read SPSS (SAV) files. Write SAV files.
#'
#' Currently haven can read and write logical, integer, numeric, character
#' and factors. See \code{\link{labelled}} for how labelled variables in
#' Stata are handled in R. \code{read_spss} is an alias for \code{read_sav}.
#'
#' @inheritParams readr::datasource
#' @param path Path to a file where the data will be written.
#' @param data Data frame to write.
#' @return A tibble, data frame variant with nice defaults.
#'
#'   Variable labels are stored in the "label" attribute of each variable.
#'   It is not printed on the console, but the RStudio viewer will show it.
#' @name read_spss
#' @examples
#' path <- system.file("examples", "iris.sav", package = "haven")
#' read_sav(path)
#'
#' tmp <- tempfile(fileext = ".sav")
#' write_sav(mtcars, tmp)
#' read_sav(tmp)
NULL

#' @export
#' @rdname read_spss
read_sav <- function(file, user_na = FALSE) {
  spec <- readr::datasource(file)
  switch(class(spec)[1],
    source_file = df_parse_sav_file(spec, user_na),
    source_raw = df_parse_sav_raw(spec, user_na),
    stop("This kind of input is not handled", call. = FALSE)
  )
}

#' @export
#' @rdname read_spss
write_sav <- function(data, path) {
  write_sav_(data, normalizePath(path, mustWork = FALSE))
}


#' @export
#' @rdname read_spss
#' @param user_na If \code{TRUE} variables with user defined missing will
#'   be read into \code{\link{labelled_spss}} objects. If \code{FALSE}, the
#'   default, user-defined missings will be converted to \code{NA}.
read_spss <- function(file, user_na = FALSE) {
  ext <- tolower(tools::file_ext(file))

  switch(ext,
    sav = read_sav(file, user_na = user_na),
    stop("Unknown extension '.",  ext, "'", call. = FALSE)
  )
}

#' Read and write Stata DTA files.
#'
#' Currently haven can read and write logical, integer, numeric, character
#' and factors. See \code{\link{labelled}} for how labelled variables in
#' Stata are handled in R.
#'
#' @inheritParams readr::datasource
#' @inheritParams read_spss
#' @param encoding The character encoding used for the file. This defaults to
#'   the encoding specified in the file, or UTF-8. But older versions of Stata
#'   (13 and earlier) did not store the encoding used, and you'll need to
#'   specify manually. A commonly used value is "Win 1252".
#' @return A tibble, data frame variant with nice defaults.
#'
#'   Variable labels are stored in the "label" attribute of each variable.
#'   It is not printed on the console, but the RStudio viewer will show it.
#' @export
#' @examples
#' path <- system.file("examples", "iris.dta", package = "haven")
#' read_dta(path)
#'
#' tmp <- tempfile(fileext = ".dta")
#' write_dta(mtcars, tmp)
#' read_dta(tmp)
#' read_stata(tmp)
read_dta <- function(file, encoding = NULL) {
  if (is.null(encoding)) {
    encoding <- ""
  }

  spec <- readr::datasource(file)
  switch(class(spec)[1],
    source_file = df_parse_dta_file(spec, encoding),
    source_raw = df_parse_dta_raw(spec, encoding),
    stop("This kind of input is not handled", call. = FALSE)
  )
}

#' @export
#' @rdname read_dta
read_stata <- function(file, encoding = NULL) {
  read_dta(file, encoding)
}

#' @export
#' @rdname read_dta
write_dta <- function(data, path) {
  validate_dta(data)
  write_dta_(data, normalizePath(path, mustWork = FALSE))
}

validate_dta <- function(data) {
  # Check variable names
  bad_names <- !grepl("^[a-zA-Z0-9_]{1,32}$", names(data))
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
