#' "Tagged" missing values
#'
#' "Tagged" missing values work exactly like regular R missing values except
#' that they store one additional byte of information a tag, which is usually
#' a letter ("A" to "Z").
#'
#' @param x A vector of single characters. The letter will be used to
#'   "tag" the missing value.
#' @useDynLib haven tagged_na_
#' @export
#' @examples
#' x <- c(1:5, tagged_na("a"), tagged_na("z"), NA)
#'
#' # Tagged NA's work identically to regular NAs
#' x
#' is.na(x)
#'
#' # To see that they're special, you need to use na_tag() or
#' # is_tagged_na():
#' is_tagged_na(x)
#' na_tag(x)
#'
#' # You can test for specific tagged NAs with the second argument
#' is_tagged_na(x, "a")
#'
#' # Because the support for tagged's NAs is somewhat tagged on to R,
#' # the left-most NA will tend to be preserved in arithmetic operations.
#' na_tag(tagged_na("a") + tagged_na("z"))
tagged_na <- function(x) {
  .Call(tagged_na_, x)
}

#' @useDynLib haven na_tag_
#' @rdname tagged_na
#' @export
na_tag <- function(x) {
  .Call(na_tag_, x)
}

#' @param tag If \code{NULL}, will only return true if the tag has this value.
#' @useDynLib haven is_tagged_na_
#' @rdname tagged_na
#' @export
is_tagged_na <- function(x, tag = NULL) {
  .Call(is_tagged_na_, x, tag)
}
