#' Read and write SPSS files
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
    cli_abort("This kind of input is not handled.")
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
    cli_abort("This kind of input is not handled.")
  )
}

#' @export
#' @rdname read_spss
#' @param compress Compression type to use:
#'
#'   * "byte": the default, uses byte compression.
#'   * "none": no compression. This is useful for software that has issues with
#'     byte compressed `.sav` files (e.g. SAS).
#'   * "zsav": uses zlib compression and produces a `.zsav` file. zlib
#'     compression is supported by SPSS version 21.0 and above.
#'
#'   `TRUE` and `FALSE` can be used for backwards compatibility, and correspond
#'   to the "zsav" and "none" options respectively.
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
write_sav <- function(data, path, compress = c("byte", "none", "zsav"), adjust_tz = TRUE) {
  if (isTRUE(compress)) {
    compress <- "zsav"
  } else if (isFALSE(compress)) {
    compress <- "none"
  } else {
    compress <- arg_match(compress)
  }

  data_out <- validate_sav(data)

  if (isTRUE(adjust_tz)) {
    data_out <- adjust_tz(data_out)
  }

  write_sav_(data_out, normalizePath(path, mustWork = FALSE), compress = compress)
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
    cli_abort("Unknown extension {.file .{ext}}.")
  )
}

validate_sav <- function(data, call = caller_env()) {
  stopifnot(is.data.frame(data))

  # Check variable names
  # https://www.ibm.com/docs/en/spss-statistics/28.0.0?topic=variables-variable-names
  #
  # SPSS variable names support non-ASCII characters.
  # The first character must be a letter or @ symbol. After the first letter,
  # SPSS ostensibly supports "letters, numbers, nonpunctuation characters, and a
  # period". In practice it's hard to tell exactly what non-punctuation
  # characters are supported.
  #
  # For simplicity, we allow characters with the Unicode properties
  # letters (\pL), numbers (\pN) and currency symbols (\pSc).
  bad_name <- !grepl("^[\\pL@]([\\pL\\pN\\pSc._$#@]*[\\pL\\pN\\pSc_$#@])?$", names(data), perl = TRUE)
  reserved_keyword <-
    toupper(names(data)) %in% c(
      "ALL", "AND", "BY", "EQ", "GE", "GT", "LE",
      "LT", "NE", "NOT", "OR", "TO", "WITH"
    )
  bad_length <- nchar(names(data), type = "bytes") > 64
  bad_vars <- bad_length | bad_name | reserved_keyword
  if (any(bad_vars)) {
    cli_abort(
      c(
        "Variables in {.arg data} must have valid SPSS variable names.",
        x = "Problems: {.var {var_names(data, bad_vars)}}"
      ),
      call = call
    )
  }

  # Check variable name duplication
  dupe_vars <- duplicated(tolower(names(data))) |
    duplicated(tolower(names(data)), fromLast = TRUE)
  if (any(dupe_vars)) {
    cli_abort(
      c(
        "SPSS does not allow duplicate variable names.",
        i = "Variable names are case-insensitive in SPSS.",
        x = "Problems: {.var {var_names(data, dupe_vars)}}"
      ),
      call = call
    )
  }

  # Check factor lengths
  level_lengths <- vapply(data, max_level_length, integer(1))

  bad_lengths <- level_lengths > 120
  if (any(bad_lengths)) {
    cli_abort(
      c(
        "SPSS only supports levels with <= 120 characters.",
        x = "Problems: {.var {var_names(data, bad_lengths)}}"
      ),
      call = call
    )
  }
  invisible(data)
}

# Helpers -----------------------------------------------------------------

max_level_length <- function(x) {
  if (!is.factor(x)) {
    return(0L)
  }

  max(0L, nchar(levels(x)), na.rm = TRUE)
}
