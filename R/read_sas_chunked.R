#' Read a SAS file by chunks
#'
#' Read a sas7bdat file by chunks
#'
#' @param ... Arguments passed on to [haven::read_sas()]
#' @param chunk_size The number of rows to include in each chunk
#' @param callback A callback function to call on each chunk
#'
#' @details
#' The callback is a function that will work on the current chunk of data and
#' should return a data.frame.
#'
#' @return A tibble, as per [haven::read_sas()]
#'
#' @export
#'
#' @examples
#' path <- system.file("examples", "iris.sas7bdat", package = "haven")
#' sepal_callback <- function(x) { x[x$Sepal_Length <= 6,] }
#' iris_chunked_filter <- read_sas_chunked(path, chunk_size = 20, callback = sepal_callback)
read_sas_chunked <- function(..., chunk_size = 10000, callback){

  stopifnot('callback must be a function' = is.function(callback))

  # initialise
  skip = 0L
  i = 1L
  data_list <- list()
  # n_max in read_sas is number of rows from skip point == chunk_size
  n_max = chunk_size

  while (TRUE) {

    current_chunk <- read_sas(..., skip = skip, n_max = n_max)

    # check end of file
    eof = nrow(current_chunk) == 0

    if (eof) break

    # evaluate call back
    data_list[[i]] <- callback(current_chunk)

    # increment
    skip = chunk_size * i
    i = i + 1L
  }

  # rbind results
  do.call(rbind.data.frame, data_list)
}
