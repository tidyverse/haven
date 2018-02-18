#' Create a labelled vector.
#'
#' A labelled vector is a common data structure in other statistical
#' environments, allowing you to assign text labels to specific values.
#' This class makes it possible to import such labelled vectors in to R
#' without loss of fidelity. This class provides few methods, as I
#' expect you'll coerce to a standard R class (e.g. a \code{\link{factor}})
#' soon after importing.
#'
#' @param x A vector to label. Must be either numeric (integer or double) or
#'   character.
#' @param labels A named vector. The vector should be the same type as
#'   \code{x}. Unlike factors, labels don't need to be exhaustive: only a fraction
#'   of the values might be labelled.
#' @param ... Ignored
#' @export
#' @examples
#' s1 <- labelled(c("M", "M", "F"), c(Male = "M", Female = "F"))
#' s2 <- labelled(c(1, 1, 2), c(Male = 1, Female = 2))
#'
#' # Unfortunately it's not possible to make as.factor work for labelled objects
#' # so instead use as_factor. This works for all types of labelled vectors.
#' as_factor(s1)
#' as_factor(s1, labels = "values")
#' as_factor(s2)
#'
#' # Other statistical software supports multiple types of missing values
#' s3 <- labelled(c("M", "M", "F", "X", "N/A"),
#'   c(Male = "M", Female = "F", Refused = "X", "Not applicable" = "N/A")
#' )
#' s3
#' as_factor(s3)
#'
#' # Often when you have a partially labelled numeric vector, labelled values
#' # are special types of missing. Use zap_labels to replace labels with missing
#' # values
#' x <- labelled(c(1, 2, 1, 2, 10, 9), c(Unknown = 9, Refused = 10))
#' zap_labels(x)
labelled <- function(x, labels) {
  if (!is.numeric(x) && !is.character(x)) {
    stop("`x` must be a numeric or a character vector", call. = FALSE)
  }
  if (!is_coercible(x, labels)) {
    stop("`x` and `labels` must be same type", call. = FALSE)
  }
  if (is.null(names(labels))) {
    stop("`labels` must have names", call. = FALSE)
  }

  structure(x,
    labels = labels,
    class = "labelled"
  )
}

is_coercible <- function(x, labels) {
  if (typeof(x) == typeof(labels)) {
    return(TRUE)
  }

  if (is.numeric(x) && is.numeric(labels)) {
    return(TRUE)
  }

  FALSE
}

#' @export
#' @rdname labelled
is.labelled <- function(x) inherits(x, "labelled")

#' @export
`[.labelled` <- function(x, ...) {
  labelled(NextMethod(), attr(x, "labels"))
}

#' @export
`[.labelled_spss` <- function(x, ...) {
  labelled_spss(NextMethod(), attr(x, "labels"), attr(x, "na_values"), attr(x, "na_range"))
}

#' @export
print.labelled <- function(x, ..., digits = getOption("digits")) {
  cat("<Labelled ", typeof(x), ">\n", sep = "")

  if (is.double(x)) {
    print_tagged_na(x, digits = digits)
  } else {
    xx <- x
    attributes(xx) <- NULL
    print.default(xx, quote = FALSE)
  }

  print_labels(x)

  invisible()
}

#' Print the labels of a labelled vector
#'
#' This is a convenience function, useful to explore the variables of
#' a newly imported dataset.
#' @param x A labelled vector
#' @param name The name of the vector (optional)
#' @export
#' @examples
#' s1 <- labelled(c("M", "M", "F"), c(Male = "M", Female = "F"))
#' s2 <- labelled(c(1, 1, 2), c(Male = 1, Female = 2))
#' labelled_df <- tibble::data_frame(s1, s2)
#'
#' for (var in names(labelled_df)) {
#'   print_labels(labelled_df[[var]], var)
#' }
print_labels <- function(x, name = NULL) {
  if (!is.labelled(x)) {
    stop("x must be a labelled vector", call. = FALSE)
  }
  labels <- attr(x, "labels")
  if (length(labels) == 0) {
    return(invisible(x))
  }

  cat("\nLabels:", name, "\n", sep = "")

  value <- if (is.double(labels)) format_tagged_na(labels) else unname(labels)

  lab_df <- data.frame(value = value, label = names(labels))
  print(lab_df, row.names = FALSE)

  invisible(x)
}

#' @export
as.data.frame.labelled <- function(x, ...) {
  df <- list(x)
  class(df) <- "data.frame"
  attr(df, "row.names") <- .set_row_names(length(x))

  df
}

label_length <- function(x) {
  if (!is.labelled(x)) {
    0L
  } else {
    max(nchar(names(attr(x, "labels"))))
  }
}

#' @export
#' @importFrom tibble type_sum
type_sum.labelled <- function(x) {
  paste0(tibble::type_sum(unclass(x)), "+lbl")
}

#' @importFrom pillar pillar_shaft
#' @export

#' @export
pillar_shaft.labelled <- function(
  x,
  show_labels = getOption("haven.show_pillar_labels", TRUE),
  ...
) {
  if (!isTRUE(show_labels)) {
    return(pillar::pillar_shaft(unclass(x)))
  }
  val <- val_pillar_info(x)
  na <- na_pillar_info(x)
  lbl <- lbl_pillar_info(x)

  pillar::new_pillar_shaft(
    list(val, na, lbl),
    min_width = max(val$wid_short + na$wid_full + lbl$wid_short),
    width =  max(val$wid_full + na$wid_full + lbl$wid_full),
    subclass = "pillar_shaft_labelled"
  )
}

#' @export
format.pillar_shaft_labelled <- function(x, width, ...) {
  pillar::new_ornament(
    truncate_labelled_pillar(x[[1]], x[[2]], x[[3]], attr(x, "width"), width),
    width = width,
    align = "left"
  )
}

str_trunc <- function(x, widths, subtle = FALSE) {
  str_width <- pillar::get_extent(x)
  too_wide <- which(!is.na(x) & str_width > widths)

  continue_symbol <- cli::symbol$continue
  if (subtle) continue_symbol <- pillar::style_subtle(continue_symbol)

  truncated <- Map(x[too_wide], widths[too_wide], f = function(item, wid) {
    paste0(crayon::col_substr(item, 1, wid - 1), continue_symbol)
  })
  truncated <- as.vector(truncated, "character")
  x[too_wide] <- truncated

  x
}

val_pillar_info <- function(x) {
  MIN_CHR_DISPLAY <- 4
  val_pillar <- pillar::pillar_shaft(zap_labels.labelled(x))
  if (is.numeric(x)) {
    disp_short <- trimws(format(val_pillar, attr(val_pillar, "min_width")), "right")
    disp_full <- trimws(format(val_pillar, attr(val_pillar, "width")), "right")

    ret <- list(
      type = "numeric",
      disp_short = disp_short,
      wid_short = pillar::get_extent(disp_short),
      disp_full = disp_full,
      wid_full = pillar::get_extent(disp_full)
    )
  } else {
    disp_full <- trimws(format(val_pillar, attr(val_pillar, "width")), "right")
    wid_full <- pillar::get_extent(disp_full)

    ret <- list(
      type = "character",
      val_pillar = val_pillar,
      wid_short = pmin(MIN_CHR_DISPLAY, wid_full),
      disp_full = disp_full,
      wid_full = wid_full
    )
  }
  ret
}

na_pillar_info <- function(x) {
  na_display <- character(length(x))
  if (is.double(x)) {
    na_display[is_tagged_na(x)] <- paste0("(", na_tag(x[is_tagged_na(x)]), ")")
  }
  if (inherits(x, "labelled_spss")) {
    is_spss_na <- is.na(x) & !is.na(unclass(x))
    na_display[is_spss_na] <- "(NA)"
  }
  na_display <- pillar::style_na(na_display)
  na_display_lens <- pillar::get_extent(na_display)
  ret <- list(
    disp_full = na_display,
    wid_full = na_display_lens
  )
  ret
}

lbl_pillar_info <- function(x) {
  MIN_LBL_DISPLAY <- 6
  labels <- attr(x, "labels")
  if (length(labels) > 0) {
    names(labels) <- pillar::style_subtle(paste0(" [", names(labels), "]"))
    attr(x, "labels") <- labels
    label_display <- as.character(as_factor(x, "labels"))
    label_display[is.na(label_display)] <- ""
  } else {
    label_display <- character(length(x))
  }
  label_widths <- pillar::get_extent(label_display)
  label_min_widths <- ifelse(label_widths > 0, pmin(MIN_LBL_DISPLAY, label_widths), 0)

  ret <- list(
    wid_short = label_min_widths,
    disp_full = label_display,
    wid_full = label_widths
  )
  ret
}

truncate_labelled_pillar <- function(vals, nas, lbls, desired_width, width) {
  if (width >= desired_width) {
    return(paste0(vals$disp_full, nas$disp_full, lbls$disp_full))
  }
  if (vals$type == "numeric") {
    if (width < max(vals$wid_full + nas$wid_full + lbls$wid_short)) {
      val_display <- vals$disp_short
      lbls_display <- str_trunc(lbls$disp_full, width - nas$wid_full - vals$wid_short, subtle = TRUE)
    } else {
      val_display <- vals$disp_full
      lbls_display <- str_trunc(lbls$disp_full, width - nas$wid_full - vals$wid_full, subtle = TRUE)
    }
  } else {
    val_widths <- pmin(vals$wid_full, width - nas$wid_full - lbls$wid_short)
    val_display <- str_trunc(vals$disp_full, val_widths)

    lbls_display <- str_trunc(lbls$disp_full, width - nas$wid_full - val_widths, subtle = TRUE)
  }
  paste0(val_display, nas$disp_full, lbls_display)
}

