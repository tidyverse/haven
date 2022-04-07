#' Read and write SAS files
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
#' @param cols_only `r lifecycle::badge("deprecated")` `cols_only` is no longer
#'   supported; use `col_select` instead.
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
                     col_select = NULL, skip = 0L, n_max = Inf, cols_only = deprecated(),
                     .name_repair = "unique") {
  if (lifecycle::is_present(cols_only)) {
    lifecycle::deprecate_warn("2.2.0", "read_sas(cols_only)", "read_sas(col_select)")
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
    cli_abort("This kind of input is not handled.")
  )
}

#' @export
#' @rdname read_sas
write_sas <- function(data, path) {
  data <- validate_sas(data)
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
#'   If a dataset label is defined, it will be stored in the "label" attribute
#'   of the tibble.
#'
#'   `write_xpt()` returns the input `data` invisibly.
#' @export
#' @examples
#' tmp <- tempfile(fileext = ".xpt")
#' write_xpt(mtcars, tmp)
#' read_xpt(tmp)
read_xpt <- function(file, col_select = NULL, skip = 0, n_max = Inf, .name_repair = "unique") {
  cols_skip <- skip_cols(read_xpt, {{ col_select }}, file)
  n_max <- validate_n_max(n_max)

  spec <- readr::datasource(file)
  switch(class(spec)[1],
    source_file = df_parse_xpt_file(spec, cols_skip, n_max, skip, name_repair = .name_repair),
    source_raw = df_parse_xpt_raw(spec, cols_skip, n_max, skip, name_repair = .name_repair),
    cli_abort("This kind of input is not handled.")
  )
}

#' @export
#' @rdname read_xpt
#' @param version Version of transport file specification to use: either 5 or 8.
#' @param name Member name to record in file. Defaults to file name sans
#'   extension. Must be <= 8 characters for version 5, and <= 32 characters
#'   for version 8.
#' @param label Dataset label to use, or `NULL`. Defaults to the value stored in
#'   the "label" attribute of `data`.
#'
#'   Note that although SAS itself supports dataset labels up to 256 characters
#'   long, dataset labels in SAS transport files must be <= 40 characters.
write_xpt <- function(data, path, version = 8, name = NULL, label = attr(data, "label")) {
  if (!version %in% c(5, 8)) {
    cli_abort("SAS transport file version {.val {version}} is not currently supported.")
  }

  if (is.null(name)) {
    name <- tools::file_path_sans_ext(basename(path))
  }
  name <- validate_xpt_name(name, version)
  label <- validate_xpt_label(label)

  data <- validate_sas(data)
  write_xpt_(
    data,
    normalizePath(path, mustWork = FALSE),
    version = version,
    name = name,
    label = label
  )
  invisible(data)
}


# Validation --------------------------------------------------------------

validate_sas <- function(data) {
  stopifnot(is.data.frame(data))

  adjust_tz(data)
}

validate_xpt_name <- function(name, version, call = caller_env()) {
  if (version == 5) {
    if (nchar(name) > 8) {
      cli_abort("{.arg name} must be 8 characters or fewer.", call = call)
    }
  } else {
    if (nchar(name) > 32) {
      cli_abort("{.arg name} must be 32 characters or fewer.", call = call)
    }
  }
  name
}

validate_xpt_label <- function(label, call = caller_env()) {
  if (!is.null(label)) {
    stopifnot(is.character(label), length(label) == 1)

    if (nchar(label) > 40) {
      cli_abort("{.arg label} must be 40 characters or fewer.", call = call)
    }
  }
  label
}
