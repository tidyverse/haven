#' Remove metadata
#'
#' @description
#' Removes the metadata that haven uses to round-trip variables and data frames
#' between R and SAS/SPSS/Stata (see [meta()]). This removes the `haven_meta`
#' attribute as well as the legacy `format.sas`, `format.spss`,
#' `format.stata`, `display_width`, and `width` attributes on columns and
#' `notes`, `creation_timestamp`, and `modified_timestamp` attributes on data
#' frames.
#'
#' Variable labels, value labels, and user-defined missing values are not
#' affected; use [zap_label()], [zap_labels()], and [zap_missing()] to remove
#' those.
#'
#' @param x A vector or data frame.
#' @family zappers
#' @export
#' @examples
#' x <- 1:3
#' meta(x) <- meta_var_sas(format = "BEST12.")
#' meta(x)
#' meta(zap_meta(x))
#'
#' # zap_meta() also works with data frames, removing metadata from the
#' # data frame and every column
#' df <- tibble::tibble(x)
#' meta(df) <- meta_df_sas(table_name = "DATA")
#' df <- zap_meta(df)
#' meta(df)
#' meta(df$x)
zap_meta <- function(x) {
  UseMethod("zap_meta")
}

#' @export
zap_meta.default <- function(x) {
  attr(x, "haven_meta") <- NULL
  strip_legacy_attrs(x)
}

#' @export
zap_meta.data.frame <- function(x) {
  x[] <- lapply(x, zap_meta)
  attr(x, "haven_meta") <- NULL
  strip_legacy_attrs(x)
}
