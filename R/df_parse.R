df_parse_options <- function(
  vendor,
  encoding = "",
  catalog_file = NULL,
  catalog_encoding = "",
  user_na = FALSE,
  cols_skip = integer(),
  n_max = 0L,
  rows_skip = 0L,
  progress = NULL
) {
  list(
    vendor = vendor,
    encoding = encoding,
    catalog_file = catalog_file,
    catalog_encoding = catalog_encoding,
    user_na = user_na,
    cols_skip = cols_skip,
    n_max = n_max,
    rows_skip = rows_skip,
    progress = progress
  )
}
