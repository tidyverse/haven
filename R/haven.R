#' @useDynLib haven, .registration = TRUE
#' @import rlang
#' @import vctrs
#' @importFrom Rcpp sourceCpp
#' @importFrom tibble tibble
#' @importFrom hms hms
NULL

#' Read and write SAS files.
#'
#' `read_sas()` supports both sas7bdat files and the accompanying sas7bcat files
#' that SAS uses to record value labels. `write_sas()` is currently experimental
#' and only works for limited datasets.
#'
#' @param data_file,catalog_file Path to data and catalog files. The files are
#'   processed with [readr::datasource()].
#' @param data Data frame to write.
#' @param path Path to file where the data will be written.
#' @param encoding,catalog_encoding The character encoding used for the
#'   `data_file` and `catalog_encoding` respectively. A value of `NULL` uses the
#'   encoding specified in the file; use this argument to override it if it is
#'   incorrect.
#' @inheritParams tibble::as_tibble
#' @param col_select One or more selection expressions, like in
#'   [dplyr::select()]. Use `c()` or `list()` to use more than one expression.
#'   See `?dplyr::select` for details on available selection options. Only the
#'   specified columns will be read from `data_file`.
#' @param skip Number of lines to skip before reading data.
#' @param n_max Maximum number of lines to read.
#' @param cols_only **Deprecated**: Use `col_select` instead.
#' @return A tibble, data frame variant with nice defaults.
#'
#'   Variable labels are stored in the "label" attribute of each variable. It is
#'   not printed on the console, but the RStudio viewer will show it.
#'
#'   `write_sas()` returns the input `data` invisibly.
#' @export
#' @examples
#' path <- system.file("examples", "iris.sas7bdat", package = "haven")
#' read_sas(path)
read_sas <- function(data_file, catalog_file = NULL,
                     encoding = NULL, catalog_encoding = encoding,
                     col_select = NULL, skip = 0L, n_max = Inf, cols_only = "DEPRECATED",
                     .name_repair = "unique"
  ) {
  if (!missing(cols_only)) {
    warning("`cols_only` is deprecated. Please use `col_select` instead.", call. = FALSE)
    stopifnot(is.character(cols_only)) # used to only work with a char vector

    # guarantee a quosure to keep NULL and tidyselect logic clean downstream
    col_select <- quo(c(!!!cols_only))
  } else {
    col_select <- enquo(col_select)
  }

  if (is.null(encoding)) {
    encoding <- ""
  }

  cols_skip <- skip_cols(read_sas, !!col_select, data_file, encoding = encoding)
  n_max <- validate_n_max(n_max)

  spec_data <- readr::datasource(data_file)
  if (is.null(catalog_file)) {
    spec_cat <- list()
  } else {
    spec_cat <- readr::datasource(catalog_file)
  }

  switch(class(spec_data)[1],
    source_file = df_parse_sas_file(spec_data, spec_cat, encoding = encoding, catalog_encoding = catalog_encoding, cols_skip = cols_skip, n_max = n_max, rows_skip = skip, name_repair = .name_repair),
    source_raw = df_parse_sas_raw(spec_data, spec_cat, encoding = encoding, catalog_encoding = catalog_encoding, cols_skip = cols_skip, n_max = n_max, rows_skip = skip, name_repair = .name_repair),
    stop("This kind of input is not handled", call. = FALSE)
  )
}

#' @export
#' @rdname read_sas
write_sas <- function(data, path) {
  validate_sas(data)
  write_sas_(data, normalizePath(path, mustWork = FALSE))
  invisible(data)
}

#' Read and write SAS transport files
#'
#' The SAS transport format is a open format, as is required for submission
#' of the data to the FDA.
#'
#' @inheritParams read_spss
#' @return A tibble, data frame variant with nice defaults.
#'
#'   Variable labels are stored in the "label" attribute of each variable.
#'   It is not printed on the console, but the RStudio viewer will show it.
#'
#'   `write_xpt()` returns the input `data` invisibly.
#' @export
#' @examples
#' tmp <- tempfile(fileext = ".xpt")
#' write_xpt(mtcars, tmp)
#' read_xpt(tmp)
read_xpt <- function(file, col_select = NULL, skip = 0, n_max = Inf, .name_repair = "unique"
) {
  cols_skip <- skip_cols(read_xpt, {{ col_select }}, file)
  n_max <- validate_n_max(n_max)

  spec <- readr::datasource(file)
  switch(class(spec)[1],
    source_file = df_parse_xpt_file(spec, cols_skip, n_max, skip, name_repair = .name_repair),
    source_raw = df_parse_xpt_raw(spec, cols_skip, n_max, skip, name_repair = .name_repair),
    stop("This kind of input is not handled", call. = FALSE)
  )
}

#' @export
#' @rdname read_xpt
#' @param version Version of transport file specification to use: either 5 or 8.
#' @param name Member name to record in file. Defaults to file name sans
#'   extension. Must be <= 8 characters for version 5, and <= 32 characters
#'   for version 8.
write_xpt <- function(data, path, version = 8, name = NULL) {
  stopifnot(version %in% c(5, 8))

  if (is.null(name)) {
    name <- tools::file_path_sans_ext(basename(path))
  }
  name <- validate_xpt_name(name, version)

  write_xpt_(
    data,
    normalizePath(path, mustWork = FALSE),
    version = version,
    name = name
  )
  invisible(data)
}

validate_xpt_name <- function(name, version) {
  if (version == 5) {
    if (nchar(name) > 8) {
      stop("`name` must be 8 characters or fewer", call. = FALSE)
    }

  } else {
    if (nchar(name) > 32) {
      stop("`name` must be 32 characters or fewer", call. = FALSE)
    }
  }
  name
}

#' Read SPSS (`.sav`, `.zsav`, `.por`) files. Write `.sav` and `.zsav` files.
#'
#' `read_sav()` reads both `.sav` and `.zsav` files; `write_sav()` creates
#' `.zsav` files when `compress = TRUE`. `read_por()` reads `.por` files.
#' `read_spss()` uses either `read_por()` or `read_sav()` based on the
#' file extension.
#'
#' Currently haven can read and write logical, integer, numeric, character
#' and factors. See [labelled_spss()] for how labelled variables in
#' SPSS are handled in R.
#'
#' @inheritParams read_sas
#' @inheritParams readr::datasource
#' @param path Path to a file where the data will be written.
#' @param data Data frame to write.
#' @param encoding The character encoding used for the file. The default,
#'   `NULL`, use the encoding specified in the file, but sometimes this
#'   value is incorrect and it is useful to be able to override it.
#' @return A tibble, data frame variant with nice defaults.
#'
#'   Variable labels are stored in the "label" attribute of each variable.
#'   It is not printed on the console, but the RStudio viewer will show it.
#'
#'   `write_sav()` returns the input `data` invisibly.
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
read_sav <- function(file, encoding = NULL, user_na = FALSE, col_select = NULL, skip = 0, n_max = Inf, .name_repair = "unique") {
  if (is.null(encoding)) {
    encoding <- ""
  }

  cols_skip <- skip_cols(read_sav, {{ col_select }}, file, encoding)
  n_max <- validate_n_max(n_max)

  spec <- readr::datasource(file)
  switch(class(spec)[1],
    source_file = df_parse_sav_file(spec, encoding, user_na, cols_skip, n_max, skip, name_repair = .name_repair),
    source_raw = df_parse_sav_raw(spec, encoding, user_na, cols_skip, n_max, skip, name_repair = .name_repair),
    stop("This kind of input is not handled", call. = FALSE)
  )
}

#' @export
#' @rdname read_spss
read_por <- function(file, user_na = FALSE, col_select = NULL, skip = 0, n_max = Inf, .name_repair = "unique") {
  cols_skip <- skip_cols(read_por, {{ col_select }}, file)
  n_max <- validate_n_max(n_max)

  spec <- readr::datasource(file)
  switch(class(spec)[1],
    source_file = df_parse_por_file(spec, encoding = "", user_na = user_na, cols_skip, n_max, skip, name_repair = .name_repair),
    source_raw = df_parse_por_raw(spec, encoding = "", user_na = user_na, cols_skip, n_max, skip, name_repair = .name_repair),
    stop("This kind of input is not handled", call. = FALSE)
  )
}

#' @export
#' @rdname read_spss
#' @param compress If `TRUE`, will compress the file, resulting in a `.zsav`
#'   file.
write_sav <- function(data, path, compress = FALSE) {
  validate_sav(data)
  write_sav_(data, normalizePath(path, mustWork = FALSE), compress = compress)
  invisible(data)
}


#' @export
#' @rdname read_spss
#' @param user_na If `TRUE` variables with user defined missing will
#'   be read into [labelled_spss()] objects. If `FALSE`, the
#'   default, user-defined missings will be converted to `NA`.
read_spss <- function(file, user_na = FALSE, col_select = NULL, skip = 0, n_max = Inf, .name_repair = "unique") {
  ext <- tolower(tools::file_ext(file))

  switch(ext,
    sav = read_sav(file, user_na = user_na, col_select = {{ col_select }}, n_max = n_max, skip = skip, .name_repair = .name_repair),
    zsav = read_sav(file, user_na = user_na, col_select = {{ col_select }}, n_max = n_max, skip = skip, .name_repair = .name_repair),
    por = read_por(file, user_na = user_na, col_select = {{ col_select }}, n_max = n_max, skip = skip, .name_repair = .name_repair),
    stop("Unknown extension '.", ext, "'", call. = FALSE)
  )
}

#' Read and write Stata DTA files.
#'
#' Currently haven can read and write logical, integer, numeric, character
#' and factors. See [labelled()] for how labelled variables in
#' Stata are handled in R.
#'
#' @section Character encoding:
#' Prior to Stata 14, files did not declare a text encoding, and the
#' default encoding differed across platforms. If `encoding = NULL`,
#' haven assumes the encoding is windows-1252, the text encoding used by
#' Stata on Windows. Unfortunately Stata on Mac and Linux use a different
#' default encoding, "latin1". If you encounter an error such as
#' "Unable to convert string to the requested encoding", try
#' `encoding = "latin1"`
#'
#' For Stata 14 and later, you should not need to manually specify `encoding`
#' value unless the value was incorrectly recorded in the source file.
#'
#' @inheritParams readr::datasource
#' @inheritParams read_spss
#' @param encoding The character encoding used for the file. Generally,
#'   only needed for Stata 13 files and earlier. See Encoding section
#'   for details.
#' @return A tibble, data frame variant with nice defaults.
#'
#'   Variable labels are stored in the "label" attribute of each variable.
#'   It is not printed on the console, but the RStudio viewer will show it.
#'
#'   If a dataset label is defined in Stata, it will stored in the "label"
#'   attribute of the tibble.
#'
#'   `write_dta()` returns the input `data` invisibly.
#' @export
#' @examples
#' path <- system.file("examples", "iris.dta", package = "haven")
#' read_dta(path)
#'
#' tmp <- tempfile(fileext = ".dta")
#' write_dta(mtcars, tmp)
#' read_dta(tmp)
#' read_stata(tmp)
read_dta <- function(file, encoding = NULL, col_select = NULL, skip = 0, n_max = Inf, .name_repair = "unique") {
  if (is.null(encoding)) {
    encoding <- ""
  }

  cols_skip <- skip_cols(read_dta, {{ col_select }}, file, encoding)
  n_max <- validate_n_max(n_max)

  spec <- readr::datasource(file)
  switch(class(spec)[1],
    source_file = df_parse_dta_file(spec, encoding, cols_skip, n_max, skip, name_repair = .name_repair),
    source_raw = df_parse_dta_raw(spec, encoding, cols_skip, n_max, skip, name_repair = .name_repair),
    stop("This kind of input is not handled", call. = FALSE)
  )
}

#' @export
#' @rdname read_dta
read_stata <- read_dta

#' @export
#' @rdname read_dta
#' @param version File version to use. Supports versions 8-15.
#' @param label Dataset label to use, or `NULL`. Defaults to the value stored in
#'   the "label" attribute of `data`. Must be <= 80 characters.
write_dta <- function(data, path, version = 14, label = attr(data, "label")) {
  validate_dta(data, version = version)
  validate_dta_label(label)
  write_dta_(data,
    normalizePath(path, mustWork = FALSE),
    version = stata_file_format(version),
    label = label
  )
  invisible(data)
}

stata_file_format <- function(version) {
  stopifnot(is.numeric(version), length(version) == 1)
  version <- as.integer(version)

  if (version == 15L) {
    119
  } else if (version == 14L) {
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

validate_dta <- function(data, version) {
  stopifnot(is.data.frame(data))

  # Check variable names
  bad_names <- !grepl("^[A-Za-z_]{1}[A-Za-z0-9_]{0,31}$", names(data))
  if (version < 14 && any(bad_names)) {
    stop(
      "The following variable names are not valid Stata variables: ",
      var_names(data, bad_names),
      call. = FALSE
    )
  }

  # Check double vectors can only have labelled integers
  bad_labels <- vapply(data, has_non_integer_labels, logical(1))
  if (any(bad_labels)) {
    stop(
      "Stata only supports labelling with integers.\nProblems: ",
      var_names(data, bad_labels),
      call. = FALSE
    )
  }
}

validate_dta_label <- function(label) {
  if (!is.null(label)) {
    stopifnot(is.character(label), length(label) == 1)

    if (nchar(label) > 80) {
      stop("Stata data labels must be 80 characters or fewer", call. = FALSE)
    }
  }
}

has_non_integer_labels <- function(x) {
  if (!is.labelled(x)) {
    return(FALSE)
  }

  if (!is.double(x)) {
    return(FALSE)
  }

  !is_integerish(attr(x, "labels"))
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

skip_cols <- function(reader, col_select = NULL, ...) {
  col_select <- enquo(col_select)
  if (quo_is_null(col_select)) {
    return(character())
  }

  cols <- names(reader(..., n_max = 0L))
  sels <- tidyselect::vars_select(cols, !!col_select)

  if (length(sels) == 0) {
    stop("Can't find any columns matching `col_select` in data.", call. = FALSE)
  }

  setdiff(cols, sels)
}

validate_n_max <- function(n) {
  if (!is.numeric(n) && !is.na(n)) {
    stop("`n_max` must be numeric, not ", class(n)[1], ".", call. = FALSE)
  }

  if (length(n) != 1) {
    stop("`n_max` must have length 1, not ", length(n), ".", call. = FALSE)
  }

  if (is.na(n) || is.infinite(n) || n < 0) {
    return(-1L)
  }

  as.integer(n)
}
