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
#'   file.  Otherwise the `.sav` file will be bytecode compressed.
write_sav <- function(data, path, compress = FALSE) {
  data <- validate_sav(data)
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

  adjust_tz(data)
}
