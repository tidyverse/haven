#' @useDynLib haven
#' @importFrom Rcpp sourceCpp
#' @importFrom tibble tibble
NULL

#' Read and write SAS files.
#'
#' Reading supports both sas7bdat files and the accompanying sas7bdat files
#' that SAS uses to record value labels. Writing value labels is not currently
#' supported.
#'
#' @param data_file,catalog_file Path to data and catalog files. The files are
#'   processed with \code{\link[readr]{datasource}()}.
#' @param data Data frame to write.
#' @param path Path to file where the data will be written.
#' @param encoding The character encoding used for the file. This defaults to
#'   the encoding specified in the file, or UTF-8. You can use this argument
#'   to override the value stored in the file if it is correct
#' @param cols_only A character vector giving an experimental way to read in
#'   only specified columns.
#' @return A tibble, data frame variant with nice defaults.
#'
#'   Variable labels are stored in the "label" attribute of each variable.
#'   It is not printed on the console, but the RStudio viewer will show it.
#' @export
#' @examples
#' path <- system.file("examples", "iris.sas7bdat", package = "haven")
#' read_sas(path)
read_sas <- function(data_file, catalog_file = NULL, encoding = NULL,
                     cols_only = NULL) {
  if (is.null(encoding)) {
    encoding <- ""
  }
  if (is.null(cols_only)) {
    cols_only <- character()
  }

  spec_data <- readr::datasource(data_file)
  if (is.null(catalog_file)) {
    spec_cat <- list()
  } else {
    spec_cat <- readr::datasource(catalog_file)
  }

  switch(class(spec_data)[1],
    source_file = df_parse_sas_file(spec_data, spec_cat, encoding = encoding, cols_only = cols_only),
    source_raw = df_parse_sas_raw(spec_data, spec_cat, encoding = encoding, cols_only = cols_only),
    stop("This kind of input is not handled", call. = FALSE)
  )
}

#' @export
#' @rdname read_sas
write_sas <- function(data, path) {
  validate_sas(data)
  write_sas_(data, normalizePath(path, mustWork = FALSE))
}

#' Read and write SAS transport files
#'
#' The SAS transport format is a open format, as is required for submission
#' of the data to the FDA.
#'
#' @inherit read_spss
#' @export
#' @examples
#' tmp <- tempfile(fileext = ".xpt")
#' write_xpt(mtcars, tmp)
#' read_xpt(tmp)
read_xpt <- function(file) {
  spec <- readr::datasource(file)
  switch(class(spec)[1],
    source_file = df_parse_xpt_file(spec),
    source_raw = df_parse_xpt_raw(spec),
    stop("This kind of input is not handled", call. = FALSE)
  )
}

#' @export
#' @rdname read_xpt
#' @param version Version of transport file specification to use: either 5 or 8.
write_xpt <- function(data, path, version = 8) {
  stopifnot(version %in% c(5, 8))

  write_xpt_(data, normalizePath(path, mustWork = FALSE), version)
}


#' Read SPSS (SAV & POR) files. Write SAV files.
#'
#' Currently haven can read and write logical, integer, numeric, character
#' and factors. See \code{\link{labelled_spss}} for how labelled variables in
#' SPSS are handled in R. \code{read_spss} is an alias for \code{read_sav}.
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
read_por <- function(file, user_na = FALSE) {
  spec <- readr::datasource(file)
  switch(class(spec)[1],
    source_file = df_parse_por_file(spec, user_na),
    source_raw = df_parse_por_raw(spec, user_na),
    stop("This kind of input is not handled", call. = FALSE)
  )
}

#' @export
#' @rdname read_spss
write_sav <- function(data, path) {
  validate_sav(data)
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
    por = read_por(file, user_na = user_na),
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
#' @param version File version to use. Supports versions 8-14.
write_dta <- function(data, path, version = 14) {
  validate_dta(data)
  write_dta_(data,
    normalizePath(path, mustWork = FALSE),
    version = stata_file_format(version)
  )
}

stata_file_format <- function(version) {
  stopifnot(is.numeric(version), length(version) == 1)
  version <- as.integer(version)

  if (version == 14L) {
    118
  } else if (version == 13L) {
    117
  } else if (version == 12L) {
    115
  } else if (version %in% c(10L, 11L)) {
    114
  } else if (version %in% c(8L, 9L)) {
    113
  } else {
    stop("Version ", version, " not currently supported", call. = FALSE)
  }
}

validate_dta <- function(data) {
  stopifnot(is.data.frame(data))

  # Check variable names
  bad_names <- !grepl("^[A-Za-z_]{1}[A-Za-z0-9_]{0,31}$", names(data))
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
}

validate_sav <- function(data) {
  stopifnot(is.data.frame(data))

  # Check factor lengths
  level_lengths <- vapply(data, max_level_length, integer(1))

  bad_lengths <- level_lengths > 120
  if (any(bad_lengths)) {
    stop(
      "SPSS only supports levels with <= 120 characters\n",
      "Problems: ", var_names(data, bad_lengths),
      call. = FALSE
    )
  }
}

validate_sas <- function(data) {
  stopifnot(is.data.frame(data))

}

var_names <- function(data, i) {
  x <- names(data)[i]
  paste(encodeString(x, quote = "`"), collapse = ", ")
}


max_level_length <- function(x) {
  if (!is.factor(x))
    return(0L)

  max(nchar(levels(x)))
}
