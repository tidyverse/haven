#' Read SAS files
#'
#' `read_sas()` supports both sas7bdat files and the accompanying sas7bcat files
#' that SAS uses to record value labels.
#'
#' @param data_file,catalog_file Path to data and catalog files. The files are
#'   processed with [readr::datasource()].
#' @param encoding,catalog_encoding The character encoding used for the
#'   `data_file` and `catalog_encoding` respectively. A value of `NULL` uses the
#'   encoding specified in the file; use this argument to override it if it is
#'   incorrect.
#' @inheritParams tibble::as_tibble
#' @param col_select Columns to include in the results. You can use the same
#'   mini-language as `dplyr::select()` to refer to the columns by name. Use
#'   `c()` to use more than one selection expression. Although this
#'   usage is less common, `col_select` also accepts a numeric column index. See
#'   [`?tidyselect::language`][tidyselect::language] for full details on the
#'   selection language.
#'
#'   Predicates using [`where()`][tidyselect::where] are not supported.
#' @param skip Number of lines to skip before reading data.
#' @param n_max Maximum number of lines to read.
#' @param progress Display a progress bar? The default, `NULL`, displays when
#'   in an interactive session. The automatic progress bar can be disabled
#'   with `options(haven.show_progress = FALSE)`.
#' @param cols_only `r lifecycle::badge("deprecated")` `cols_only` is no longer
#'   supported; use `col_select` instead.
#' @inherit labelled-output return
#' @export
#' @examples
#' path <- system.file("examples", "iris.sas7bdat", package = "haven")
#' read_sas(path)
read_sas <- function(
  data_file,
  catalog_file = NULL,
  encoding = NULL,
  catalog_encoding = encoding,
  col_select = NULL,
  skip = 0L,
  n_max = Inf,
  cols_only = deprecated(),
  progress = NULL,
  .name_repair = "unique"
) {
  check_string(encoding, allow_null = TRUE)
  check_string(catalog_encoding, allow_null = TRUE)
  check_number_whole(skip, min = 0)
  n_max <- check_n_max(n_max)
  check_bool(progress, allow_null = TRUE)
  progress <- progress %||% show_progress()

  if (lifecycle::is_present(cols_only)) {
    lifecycle::deprecate_warn(
      "2.2.0",
      "read_sas(cols_only)",
      "read_sas(col_select)"
    )
    # used to only work with a char vector
    if (!is.character(cols_only)) {
      cli_abort("{.arg cols_only} must be a character vector.")
    }

    # guarantee a quosure to keep NULL and tidyselect logic clean downstream
    col_select <- quo(c(!!!cols_only))
  } else {
    col_select <- enquo(col_select)
  }

  encoding <- encoding %||% ""
  catalog_encoding <- catalog_encoding %||% ""

  spec_data <- readr::datasource(data_file)
  cols <- select_cols(
    read_sas,
    !!col_select,
    spec_data,
    encoding = encoding,
    .name_repair = .name_repair
  )

  if (is.null(catalog_file)) {
    spec_cat <- list()
  } else {
    spec_cat <- readr::datasource(catalog_file)
  }

  data <- switch(
    class(spec_data)[1],
    source_file = df_parse_sas_file(
      spec_data,
      spec_cat,
      encoding = encoding,
      catalog_encoding = catalog_encoding,
      cols_skip = cols$skip,
      n_max = n_max,
      rows_skip = skip,
      progress = progress
    ),
    source_raw = df_parse_sas_raw(
      spec_data,
      spec_cat,
      encoding = encoding,
      catalog_encoding = catalog_encoding,
      cols_skip = cols$skip,
      n_max = n_max,
      rows_skip = skip,
      progress = progress
    ),
    cli_abort("This kind of input is not handled.")
  )

  output_cols(data, cols, .name_repair)
}

#' Write SAS files
#'
#' @description
#' `r lifecycle::badge("deprecated")`
#'
#' `write_sas()` creates sas7bdat files. Unfortunately the SAS file format is
#' complex and undocumented, so `write_sas()` is unreliable and in most cases
#' SAS will not read files that it produces.
#'
#' [write_xpt()] writes files in the open SAS transport format, which has
#' limitations but will be reliably read by SAS.
#'
#' @param data Data frame to write.
#' @param path Path to file where the data will be written.
#' @keywords internal
#' @export
write_sas <- function(data, path) {
  lifecycle::deprecate_warn("2.5.2", "write_sas()", "write_xpt()")

  check_data_frame(data)
  check_string(path)

  data_out <- adjust_tz(data)
  write_sas_(data_out, normalizePath(path, mustWork = FALSE))

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
  progress <- progress %||% show_progress()

  spec <- readr::datasource(file)
  cols <- select_cols(
    read_xpt,
    {{ col_select }},
    spec,
    .name_repair = .name_repair
  )

  data <- switch(
    class(spec)[1],
    source_file = df_parse_xpt_file(spec, cols$skip, n_max, skip, progress),
    source_raw = df_parse_xpt_raw(spec, cols$skip, n_max, skip, progress),
    cli_abort("This kind of input is not handled.")
  )

  output_cols(data, cols, .name_repair)
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
  version = 8,
  name = NULL,
  label = attr(data, "label"),
  adjust_tz = TRUE
) {
  check_data_frame(data)
  check_string(path)
  check_xpt_version(version)

  name <- name %||% tools::file_path_sans_ext(basename(path))
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
    label = label
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
