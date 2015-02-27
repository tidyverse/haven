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
  h <- x %/% 3600
  m <- (x - h * 3600) %/% 60
  s <- x %% 60

  paste0(
    format(h, align = "right"), ":",
    sprintf("%02d", m), ":",
    sprintf("%02d", s)
  )
}

#' @export
print.hms <- function(x, ...) {
  print(format(hms), quote = FALSE)
}
