#' "Tagged" missing values
#'
#' "Tagged" missing values work exactly like regular R missing values except
#' that they store one additional byte of information a tag, which is usually
#' a letter ("a" to "z"). When by loading a SAS and Stata file, the tagged
#' missing values always use lower case values.
#'
#' \code{format_tagged_na()} and \code{print_tagged_na()} format tagged
#' NA's as NA(a), NA(b), etc.
#'
#' @param ... Vectors containing single character. The letter will be used to
#'   "tag" the missing value.
#' @param x A numeric vector
#' @param digits Number of digits to use in string representation
#' @useDynLib haven tagged_na_
#' @export
#' @examples
#' x <- c(1:5, tagged_na("a"), tagged_na("z"), NA)
#'
#' # Tagged NA's work identically to regular NAs
#' x
#' is.na(x)
#'
#' # To see that they're special, you need to use na_tag(),
#' # is_tagged_na(), or print_tagged_na():
#' is_tagged_na(x)
#' na_tag(x)
#' print_tagged_na(x)
#'
#' # You can test for specific tagged NAs with the second argument
#' is_tagged_na(x, "a")
#'
#' # Because the support for tagged's NAs is somewhat tagged on to R,
#' # the left-most NA will tend to be preserved in arithmetic operations.
#' na_tag(tagged_na("a") + tagged_na("z"))
tagged_na <- function(...) {
  .Call(tagged_na_, c(...))
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

#' @rdname tagged_na
#' @export
format_tagged_na <- function(x, digits = getOption("digits")) {
  out <- format(x, digits = digits)
  out[is_tagged_na(x)] <- paste0("NA(", na_tag(x[is_tagged_na(x)]), ")")

  # format again to make sure all elements have same width
  format(out, justify = "right")
}

#' @rdname tagged_na
#' @export
print_tagged_na <- function(x, digits = getOption("digits")) {
  print(format_tagged_na(x), quote = FALSE)
}
