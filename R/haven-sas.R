#' Read and write SAS files
#'
#' @description
#' `read_sas()` supports both sas7bdat files and the accompanying sas7bcat files
#' that SAS uses to record value labels. `write_sas()` creates sas7bdat files;
#' it does not write value labels.
#'
#' The sas7bdat format is closed and undocumented, so if you need an open
#' format, instead consider [write_xpt()], the SAS transport format.
#'
#' @param data_file,catalog_file Data and catalog files. Each can be a path,
#'   a URL, a connection, or a raw vector.
#' @param encoding,catalog_encoding The character encoding used for the
#'   `data_file` and `catalog_file` respectively. A value of `NULL` uses the
#'   encoding specified in the file; use this argument to override it if it is
#'   incorrect. Catalog files often don't specify an encoding, in which case
#'   the encoding of `data_file` is used.
#' @inheritParams tibble::as_tibble
#' @inheritParams read_spss
#' @inherit labelled-output return
#' @export
#' @examples
#' path <- system.file("examples", "iris.sas7bdat", package = "haven")
#' read_sas(path)
#'
#' tmp <- tempfile(fileext = ".sas7bdat")
#' write_sas(mtcars, tmp)
#' read_sas(tmp)
read_sas <- function(
  data_file,
  catalog_file = NULL,
  encoding = NULL,
  catalog_encoding = encoding,
  col_select = NULL,
  skip = 0L,
  n_max = Inf,
  progress = NULL,
  .name_repair = "unique"
) {
  check_string(encoding, allow_null = TRUE)
  check_string(catalog_encoding, allow_null = TRUE)
  check_number_whole(skip, min = 0)
  n_max <- check_n_max(n_max)
  check_bool(progress, allow_null = TRUE)
  progress <- local_progress_bar(progress)

  encoding <- encoding %||% ""
  catalog_encoding <- catalog_encoding %||% ""

  spec_data <- datasource(data_file)
  header <- read(spec_data, df_parse_options("sas7bdat", encoding))
  cols <- select_cols(names(header), {{ col_select }}, .name_repair)

  if (is.null(catalog_file)) {
    spec_cat <- NULL
  } else {
    spec_cat <- datasource(catalog_file)
  }

  options <- df_parse_options(
    "sas7bdat",
    encoding = encoding,
    catalog_file = spec_cat,
    catalog_encoding = catalog_encoding,
    cols_skip = cols$skip,
    n_max = n_max,
    rows_skip = skip,
    progress = progress
  )
  data <- read(spec_data, options)
  output_cols(data, cols)
}

#' @param data Data frame to write. File and variable metadata stored with
#'   [meta()] is used where possible; see [meta_df_sas()] and [meta_var_sas()]
#'   for the fields that affect the output.
#' @param path Path to file where the data will be written.
#' @rdname read_sas
#' @export
write_sas <- function(data, path) {
  check_data_frame(data)
  check_string(path)

  data_out <- adjust_tz(data)
  write_sas_(
    data_out,
    normalizePath(path, mustWork = FALSE),
    meta = writer_meta(data_out, "sas")
  )

  invisible(data)
}


#' Read and write SAS transport files
#'
#' The SAS transport format is an open format, as is required for submission
#' of data to the FDA.
#'
#' Value labels are not supported by the transport format, and are silently
#' ignored by `write_xpt()`.
#'
#' Note that character limits are expressed in bytes. The number of bytes
#' will often be the same as the number of characters, but strings with
#' multibyte character sequences will count some symbols as more than one
#' character. For example, the string "café" is 5 bytes long in UTF-8.
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
#'   The data frame and each variable also carry metadata read from the file;
#'   access it with [meta()] and see [meta_var_sas()] for the available
#'   fields.
#'
#'   `write_xpt()` returns the input `data` invisibly.
#' @export
#' @examples
#' tmp <- tempfile(fileext = ".xpt")
#' write_xpt(mtcars, tmp)
#' read_xpt(tmp)
read_xpt <- function(
  file,
  col_select = NULL,
  skip = 0,
  n_max = Inf,
  progress = NULL,
  .name_repair = "unique"
) {
  check_number_whole(skip, min = 0)
  n_max <- check_n_max(n_max)
  check_bool(progress, allow_null = TRUE)
  progress <- local_progress_bar(progress)

  spec <- datasource(file)
  header <- read(spec, df_parse_options("xpt"))
  cols <- select_cols(names(header), {{ col_select }}, .name_repair)

  options <- df_parse_options(
    "xpt",
    cols_skip = cols$skip,
    n_max = n_max,
    rows_skip = skip,
    progress = progress
  )
  data <- read(spec, options)

  output_cols(data, cols)
}

#' @export
#' @rdname read_xpt
#' @param version Version of transport file specification to use: either 5 or 8.
#'   Defaults to the version recorded in the metadata of `data` (if it was read
#'   with [read_xpt()]), otherwise 8.
#' @param name Member name to record in file. Defaults to the table name
#'   recorded in the metadata of `data`, otherwise the file name sans
#'   extension. Must be <= 8 characters for version 5, and <= 32 characters
#'   for version 8.
#' @param label Dataset label to use, or `NULL`. Defaults to the value stored in
#'   the "label" attribute of `data`.
#'
#'   Note that although SAS itself supports dataset labels up to 256 characters
#'   long, dataset labels in SAS transport files must be <= 40 characters.
#' @param data Data frame to write. File and variable metadata stored with
#'   [meta()] is used where possible; see [meta_df_sas()] and [meta_var_sas()]
#'   for the fields that affect the output.
#' @param adjust_tz Stata, SPSS and SAS do not have a concept of time zone,
#'   and all [date-time] variables are treated as UTC. `adjust_tz` controls
#'   how the timezone of date-time values is treated when writing.
#'
#'   * If `TRUE` (the default) the timezone of date-time values is ignored, and
#'   they will display the same in R and Stata/SPSS/SAS, e.g.
#'   `"2010-01-01 09:00:00 NZDT"` will be written as `"2010-01-01 09:00:00"`.
#'   Note that this changes the underlying numeric data, so use caution if
#'   preserving between-time-point differences is critical.
#'   * If `FALSE`, date-time values are written as the corresponding UTC value,
#'   e.g. `"2010-01-01 09:00:00 NZDT"` will be written as
#'   `"2009-12-31 20:00:00"`.
write_xpt <- function(
  data,
  path,
  version = NULL,
  name = NULL,
  label = attr(data, "label"),
  adjust_tz = TRUE
) {
  check_data_frame(data)
  check_string(path)
  version <- version %||% meta_df_field(data, "sas", "xpt_version") %||% 8
  check_xpt_version(version)

  name <- name %||%
    meta_df_field(data, "sas", "table_name") %||%
    tools::file_path_sans_ext(basename(path))
  check_xpt_name(name, version)
  check_xpt_label(label)
  check_bool(adjust_tz)

  check_xpt_var_names(data, version)

  if (isTRUE(adjust_tz)) {
    data <- adjust_tz(data)
  }

  write_xpt_(
    data,
    normalizePath(path, mustWork = FALSE),
    version = version,
    name = name,
    label = label,
    meta = writer_meta(data, "sas")
  )
  invisible(data)
}


# Checks ------------------------------------------------------------------

check_xpt_version <- function(
  version,
  arg = caller_arg(version),
  call = caller_env()
) {
  check_number_whole(version, arg = arg, call = call)

  if (!version %in% c(5, 8)) {
    cli_abort(
      "SAS transport file version {.val {version}} is not currently supported.",
      call = call
    )
  }
}

check_xpt_name <- function(name, version, call = caller_env()) {
  check_string(name, allow_null = TRUE, call = call)
  if (version == 5) {
    if (nchar(name, type = "bytes") > 8) {
      cli_abort("{.arg name} must be 8 characters or fewer.", call = call)
    }
  } else {
    if (nchar(name, type = "bytes") > 32) {
      cli_abort("{.arg name} must be 32 characters or fewer.", call = call)
    }
  }
}

check_xpt_label <- function(label, call = caller_env()) {
  check_string(label, call = call, allow_null = TRUE)
  if (!is.null(label) && nchar(label, type = "bytes") > 40) {
    cli_abort("{.arg label} must be 40 characters or fewer.", call = call)
  }
}

check_xpt_var_names <- function(data, version, call = caller_env()) {
  max_len <- if (version == 5) 8L else 32L
  bad_length <- nchar(names(data), type = "bytes") > max_len

  if (any(bad_length)) {
    cli_abort(
      c(
        "Variable names must be {max_len} bytes or fewer for SAS transport format version {version}.",
        x = "Problems: {.var {var_names(data, bad_length)}}"
      ),
      call = call
    )
  }
}
