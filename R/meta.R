# Property helpers ---------------------------------------------------------

prop_string <- function(values = NULL) {
  S7::new_property(
    NULL | S7::class_character,
    validator = function(value) {
      if (is.null(value)) {
        return()
      }
      if (length(value) != 1 || is.na(value)) {
        return("must be a single string or NULL")
      }
      if (!is.null(values) && !value %in% values) {
        return(paste0(
          "must be one of ",
          paste0('"', values, '"', collapse = ", "),
          ", not \"",
          value,
          "\""
        ))
      }
    }
  )
}

prop_int <- function(name, min = 1, max = Inf, values = NULL) {
  S7::new_property(
    NULL | S7::class_integer,
    setter = function(self, value) {
      if (is.double(value) && all(value == trunc(value), na.rm = TRUE)) {
        value <- as.integer(value)
      }
      S7::prop(self, name) <- value
      self
    },
    validator = function(value) {
      if (is.null(value)) {
        return()
      }
      if (length(value) != 1 || is.na(value)) {
        return("must be a single integer or NULL")
      }
      if (!is.null(values)) {
        if (!value %in% values) {
          return(paste0("must be one of ", paste0(values, collapse = ", ")))
        }
      } else if (value < min || value > max) {
        if (is.infinite(max)) {
          return(paste0("must be ", min, " or greater"))
        } else {
          return(paste0("must be between ", min, " and ", max))
        }
      }
    }
  )
}

prop_timestamp <- function() {
  S7::new_property(
    NULL | S7::class_POSIXct,
    validator = function(value) {
      if (!is.null(value) && length(value) != 1) {
        "must be a single date-time or NULL"
      }
    }
  )
}

prop_notes <- function() {
  S7::new_property(
    NULL | S7::class_character,
    validator = function(value) {
      if (!is.null(value) && anyNA(value)) {
        "must not contain missing values"
      }
    }
  )
}

# Column metadata ----------------------------------------------------------

#' @export
meta_var <- S7::new_class(
  "meta_var",
  package = "haven",
  abstract = TRUE,
  properties = list(
    format = prop_string()
  )
)

#' @export
meta_var_sas <- S7::new_class(
  "meta_var_sas",
  package = "haven",
  parent = meta_var,
  properties = list(
    informat = prop_string(),
    width = prop_int("width")
  )
)

#' @export
meta_var_spss <- S7::new_class(
  "meta_var_spss",
  package = "haven",
  parent = meta_var,
  properties = list(
    display_width = prop_int("display_width"),
    width = prop_int("width"),
    measure = prop_string(c("nominal", "ordinal", "scale")),
    alignment = prop_string(c("left", "right", "center"))
  )
)

#' @export
meta_var_stata <- S7::new_class(
  "meta_var_stata",
  package = "haven",
  parent = meta_var,
  properties = list(
    type = prop_string(c("byte", "int", "long", "float", "double", "strL"))
  )
)

# Data frame metadata ------------------------------------------------------

#' @export
meta_df <- S7::new_class(
  "meta_df",
  package = "haven",
  abstract = TRUE,
  properties = list(
    created = prop_timestamp(),
    modified = prop_timestamp()
  )
)

#' @export
meta_df_sas <- S7::new_class(
  "meta_df_sas",
  package = "haven",
  parent = meta_df,
  properties = list(
    table_name = prop_string(),
    xpt_version = prop_int("xpt_version", values = c(5L, 8L))
  )
)

#' @export
meta_df_spss <- S7::new_class(
  "meta_df_spss",
  package = "haven",
  parent = meta_df,
  properties = list(
    notes = prop_notes(),
    mr_sets = NULL | S7::class_list
  )
)

#' @export
meta_df_stata <- S7::new_class(
  "meta_df_stata",
  package = "haven",
  parent = meta_df,
  properties = list(
    notes = prop_notes(),
    version = prop_int("version", min = 8, max = 15)
  )
)

meta_class <- function(vendor, df) {
  if (df) {
    switch(
      vendor,
      sas = meta_df_sas,
      spss = meta_df_spss,
      stata = meta_df_stata
    )
  } else {
    switch(
      vendor,
      sas = meta_var_sas,
      spss = meta_var_spss,
      stata = meta_var_stata
    )
  }
}

# Accessors ----------------------------------------------------------------

#' @export
meta <- function(x, vendor = NULL) {
  vendor <- check_vendor(vendor)

  out <- attr(x, "haven_meta", exact = TRUE) %||% legacy_meta(x, vendor)
  if (is.null(vendor)) {
    return(out)
  }

  class <- meta_class(vendor, is.data.frame(x))
  if (is.null(out)) {
    class()
  } else {
    check_meta_vendor(out, class)
    out
  }
}

#' @export
`meta<-` <- function(x, vendor = NULL, value) {
  vendor <- check_vendor(vendor)

  if (!is.null(value)) {
    if (is.data.frame(x)) {
      if (!S7::S7_inherits(value, meta_df)) {
        stop_input_type(
          value,
          "a <meta_df> object or `NULL`",
          arg = "value",
          call = current_env()
        )
      }
    } else {
      if (!S7::S7_inherits(value, meta_var)) {
        stop_input_type(
          value,
          "a <meta_var> object or `NULL`",
          arg = "value",
          call = current_env()
        )
      }
    }

    if (!is.null(vendor)) {
      check_meta_vendor(value, meta_class(vendor, is.data.frame(x)))
    }
  }

  attr(x, "haven_meta") <- value
  x
}

check_vendor <- function(vendor, call = caller_env()) {
  if (is.null(vendor)) {
    return(NULL)
  }
  arg_match0(vendor, c("sas", "spss", "stata"), error_call = call)
}

check_meta_vendor <- function(x, expected, call = caller_env()) {
  if (!S7::S7_inherits(x, expected)) {
    cli_abort(
      "Metadata must be {.cls {S7_class_name(expected)}}, not {.cls {class(x)[[1]]}}.",
      call = call
    )
  }
}

S7_class_name <- function(class) {
  paste0(class@package, "::", class@name)
}

# Legacy attributes --------------------------------------------------------

legacy_meta <- function(x, vendor = NULL) {
  if (is.data.frame(x)) {
    legacy_meta_df(x, vendor)
  } else {
    legacy_meta_var(x, vendor)
  }
}

legacy_meta_var <- function(x, vendor = NULL) {
  vendor <- legacy_vendor(x) %||% vendor
  if (is.null(vendor)) {
    return(NULL)
  }

  format <- attr(x, paste0("format.", vendor), exact = TRUE)
  width <- attr(x, "width", exact = TRUE)
  display_width <- attr(x, "display_width", exact = TRUE)

  switch(
    vendor,
    sas = meta_var_sas(format = format, width = width),
    spss = meta_var_spss(
      format = format,
      width = width,
      display_width = display_width
    ),
    stata = meta_var_stata(format = format)
  )
}

legacy_meta_df <- function(x, vendor = NULL) {
  vendors <- lapply(x, legacy_vendor)
  vendors <- Filter(Negate(is.null), vendors)
  vendor <- if (length(vendors)) vendors[[1]] else vendor
  if (is.null(vendor)) {
    return(NULL)
  }

  created <- attr(x, "creation_timestamp", exact = TRUE)
  modified <- attr(x, "modified_timestamp", exact = TRUE)
  notes <- attr(x, "notes", exact = TRUE)

  switch(
    vendor,
    sas = meta_df_sas(created = created, modified = modified),
    spss = meta_df_spss(created = created, modified = modified, notes = notes),
    stata = meta_df_stata(created = created, modified = modified, notes = notes)
  )
}

legacy_vendor <- function(x) {
  if (!is.null(attr(x, "format.sas", exact = TRUE))) {
    "sas"
  } else if (
    !is.null(attr(x, "format.spss", exact = TRUE)) ||
      !is.null(attr(x, "display_width", exact = TRUE))
  ) {
    "spss"
  } else if (!is.null(attr(x, "format.stata", exact = TRUE))) {
    "stata"
  } else {
    NULL
  }
}
