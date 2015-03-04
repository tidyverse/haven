#' Hours, minutes, seconds.
#'
#' A "time" variable for R, recording the number of seconds since midnight.
#'
#' @param x An integer vector
#' @export
#' @examples
#' hms(100)
#' hms(3666)
hms <- function(x) {
  structure(x, class = "hms")
}

#' @export
format.hms <- function(x, ...) {
  x <- as.integer(x)
  h <- x %/% 3600L
  m <- (x - h * 3600L) %/% 60L
  s <- x %% 60L

  hms <- paste0(
    format(h, align = "right"), ":",
    sprintf("%02d", m), ":",
    sprintf("%02d", s)
  )

  ifelse(is.na(x), "NA", hms)
}

#' @export
print.hms <- function(x, ...) {
  print(format(x), quote = FALSE)
}

#' @export
`[.hms` <- function(x, ...) {
  hms(NextMethod(x))
}

#' @export
`[[.hms` <- function(x, ...) {
  hms(NextMethod(x))
}
