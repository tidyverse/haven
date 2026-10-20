#' @useDynLib haven, .registration = TRUE
#' @keywords internal
"_PACKAGE"

## usethis namespace: start
#' @import rlang
#' @import vctrs
#' @importFrom cli cli_abort
#' @importFrom cli cli_warn
#' @importFrom hms hms
#' @importFrom lifecycle deprecated
#' @importFrom tibble tibble
## usethis namespace: end
NULL

# Enable `@` for S7 objects in R < 4.3.0
#' @rawNamespace if (getRversion() < "4.3.0") importFrom("S7", "@")
NULL
