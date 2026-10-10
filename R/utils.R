cat_line <- function(...) {
  cat(paste0(..., "\n", collapse = ""))
}

# TODO: Remove once vec_cast() preserves names.
# https://github.com/r-lib/vctrs/issues/623
vec_cast_named <- function(x, to, ...) {
  stats::setNames(vec_cast(x, to, ...), names(x))
}

combine_labels <- function(x_labels, y_labels, x_arg, y_arg) {
  x_common <- x_labels[x_labels %in% y_labels]
  y_common <- y_labels[y_labels %in% x_labels]

  if (length(x_common) > 0) {
    x_common <- x_common[order(x_common)]
    y_common <- y_common[order(y_common)]

    problems <- x_common[names(x_common) != names(y_common)]
    if (length(problems) > 0) {
      problems <- cli::cli_vec(problems, list(vec_trunc = 10))

      cli_warn(c(
        "{.var {x_arg}} and {.var {y_arg}} have conflicting value labels.",
        i = "Labels for these values will be taken from {.var {x_arg}}.",
        x = "Values: {.val {problems}}"
      ))
    }
  }

  c(x_labels, y_labels[!y_labels %in% x_labels])
}

force_utc <- function(x) {
  if (identical(attr(x, "tzone"), "UTC")) {
    x
  } else {
    x_attr <- attributes(x)
    x <- format(x, usetz = FALSE, format = "%Y-%m-%d %H:%M:%S")
    x <- as.POSIXct(x, tz = "UTC", format = "%Y-%m-%d %H:%M:%S")
    attr_miss <- setdiff(names(x_attr), c(names(attributes(x)), "names"))
    attributes(x)[attr_miss] <- x_attr[attr_miss]
    x
  }
}

select_cols <- function(reader, col_select = NULL, ..., call = caller_env()) {
  col_select <- enquo(col_select)
  if (quo_is_null(col_select)) {
    return(list(pos = NULL, skip = integer()))
  }

  cols <- names(reader(..., n_max = 0L))
  data <- as.list(stats::setNames(seq_along(cols), cols))

  pos <- tidyselect::eval_select(
    col_select,
    data = data,
    allow_rename = TRUE,
    allow_empty = FALSE,
    allow_predicates = FALSE,
    error_call = call
  )

  list(
    select = pos,
    skip = as.integer(setdiff(seq_along(cols), pos) - 1L)
  )
}

output_cols <- function(data, cols, name_repair, call = caller_env()) {
  if (is.null(cols$select)) {
    set_names(
      data,
      vctrs::vec_as_names(names(data), repair = name_repair, call = call)
    )
  } else {
    set_names(data[rank(cols$select)], names(cols$select))
  }
}

check_n_max <- function(n, arg = caller_arg(n), call = caller_env()) {
  check_number_decimal(n, allow_na = TRUE, arg = arg, call = call)

  if (is.na(n) || is.infinite(n) || n < 0) {
    return(-1L)
  }

  as.integer(n)
}

adjust_tz <- function(df) {
  datetime <- vapply(df, inherits, "POSIXt", FUN.VALUE = logical(1))
  df[datetime] <- lapply(df[datetime], force_utc)
  df
}

var_names <- function(data, i) {
  names(data)[i]
}

# Returns either a length-1 character vector (a path to a local,
# uncompressed file) or a raw vector containing the file contents.
datasource <- function(file, call = caller_env()) {
  if (is.raw(file)) {
    return(file)
  } else if (inherits(file, "connection")) {
    return(read_connection(file))
  }
  check_string(file, arg = "file", call = call)

  if (grepl("^(https?|ftps?)://", file)) {
    tmp <- tempfile(fileext = compression_ext(file))
    on.exit(unlink(tmp), add = TRUE)
    utils::download.file(file, tmp, quiet = TRUE, mode = "wb")
    return(read_connection(compressed_con(tmp)))
  }

  file <- normalizePath(file, mustWork = FALSE)
  if (!file.exists(file)) {
    cli::cli_abort("{.path {file}} does not exist.", call = call)
  }

  if (compression_ext(file) == "") {
    file
  } else {
    read_connection(compressed_con(file))
  }
}

compression_ext <- function(path) {
  ext <- regmatches(path, regexpr("\\.(gz|bz2|xz|zip)$", path))
  if (length(ext) == 0) "" else ext
}

compressed_con <- function(path) {
  switch(
    compression_ext(path),
    .gz = gzfile(path),
    .bz2 = bzfile(path),
    .xz = xzfile(path),
    .zip = zip_con(path),
    file(path)
  )
}

zip_con <- function(path, call = caller_env()) {
  files <- utils::unzip(path, list = TRUE)$Name
  if (length(files) != 1) {
    cli::cli_abort(
      "{.path {path}} must contain exactly one file, not {length(files)}.",
      call = call
    )
  }
  unz(path, files)
}

read_connection <- function(con, chunk_size = 64 * 1024L) {
  if (!isOpen(con)) {
    open(con, "rb")
    on.exit(close(con), add = TRUE)
  }

  chunks <- list()
  repeat {
    chunk <- readBin(con, "raw", chunk_size)
    if (length(chunk) == 0) {
      break
    }
    chunks[[length(chunks) + 1]] <- chunk
  }
  unlist(chunks, use.names = FALSE) %||% raw()
}
