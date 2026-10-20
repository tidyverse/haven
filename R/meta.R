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

# Abstract parents -----------------------------------------------------------

meta_var <- S7::new_class(
  "meta_var",
  package = "haven",
  abstract = TRUE,
  properties = list(
    format = prop_string()
  )
)

meta_df <- S7::new_class(
  "meta_df",
  package = "haven",
  abstract = TRUE,
  properties = list(
    created = prop_timestamp(),
    modified = prop_timestamp()
  )
)

# SAS ------------------------------------------------------------------------

#' SAS metadata
#'
#' `meta_var_sas` (column) and `meta_df_sas` (data frame) hold
#' metadata read from and written to SAS files. See [meta()] for how to
#' access them.
#'
#' @param format The display format (e.g. `"DATE9."`).
#' @param informat The input format.
#' @param width The variable width.
#' @param created,modified File creation and modification timestamps.
#' @param table_name The SAS table name.
#' @param xpt_version The transport file version (5 or 8).
#' @examples
#' df <- read_sas(system.file("examples", "iris.sas7bdat", package = "haven"))
#' meta(df)
#' meta(df)@table_name
#' meta(df$Species)
#' meta(df$Species)@format
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
#' @rdname meta_var_sas
meta_df_sas <- S7::new_class(
  "meta_df_sas",
  package = "haven",
  parent = meta_df,
  properties = list(
    table_name = prop_string(),
    xpt_version = prop_int("xpt_version", values = c(5L, 8L))
  )
)

# SPSS -----------------------------------------------------------------------

#' SPSS metadata
#'
#' `meta_var_spss` (column) and `meta_df_spss` (data frame) hold
#' metadata read from and written to SPSS files. See [meta()] for how to
#' access them.
#'
#' @param format The display format.
#' @param display_width The display width.
#' @param width The variable width.
#' @param measure The measure level: `"nominal"`, `"ordinal"`, or `"scale"`.
#' @param alignment The display alignment: `"left"`, `"right"`, or `"center"`.
#' @param created,modified File creation and modification timestamps.
#' @param notes File notes.
#' @param mr_sets Multiple response sets.
#' @examples
#' df <- read_sav(system.file("examples", "iris.sav", package = "haven"))
#' meta(df)
#' meta(df)@created
#' meta(df$Species)
#' meta(df$Species)@measure
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
#' @rdname meta_var_spss
meta_df_spss <- S7::new_class(
  "meta_df_spss",
  package = "haven",
  parent = meta_df,
  properties = list(
    notes = prop_notes(),
    mr_sets = NULL | S7::class_list
  )
)

# Stata ----------------------------------------------------------------------

#' Stata metadata
#'
#' `meta_var_stata` (column) and `meta_df_stata` (data frame) hold
#' metadata read from and written to Stata files. See [meta()] for how to
#' access them.
#' @param format The display format.
#' @param type The storage type: `"byte"`, `"int"`, `"long"`, `"float"`,
#'   `"double"`, or `"strL"`.
#' @param created,modified File creation and modification timestamps.
#' @param notes File notes.
#' @param version The Stata file version.
#' @examples
#' df <- read_dta(system.file("examples", "iris.dta", package = "haven"))
#' meta(df)
#' meta(df)@version
#' meta(df$species)
#' meta(df$species)@format
#' @export
meta_var_stata <- S7::new_class(
  "meta_var_stata",
  package = "haven",
  parent = meta_var,
  properties = list(
    type = prop_string(c("byte", "int", "long", "float", "double", "strL"))
  )
)

#' @export
#' @rdname meta_var_stata
meta_df_stata <- S7::new_class(
  "meta_df_stata",
  package = "haven",
  parent = meta_df,
  properties = list(
    notes = prop_notes(),
    version = prop_int("version", min = 8, max = 15)
  )
)

# Printing -------------------------------------------------------------------

S7::method(print, meta_df) <- function(x, ...) print_meta(x)
S7::method(print, meta_var) <- function(x, ...) print_meta(x)

print_meta <- function(x) {
  cat_line("<", S7_class_name(S7::S7_class(x)), ">")

  props <- compact(S7::props(x))
  names <- format(paste0("@", names(props)), justify = "left")
  for (i in seq_along(props)) {
    cat_line(names[[i]], ": ", format_meta_prop(props[[i]]))
  }
  invisible(x)
}

format_meta_prop <- function(x) {
  if (is.null(x)) {
    "NULL"
  } else if (inherits(x, "POSIXct")) {
    format(x, usetz = TRUE)
  } else if (is.character(x)) {
    paste0(encodeString(x, quote = '"'), collapse = " ")
  } else if (is.atomic(x)) {
    paste0(format(x), collapse = " ")
  } else {
    paste0("<", class(x)[[1]], ">")
  }
}

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

#' File metadata
#'
#' @description
#' `meta()` gets and sets the metadata attached to a vector or data frame
#' by [read_sas()], [read_spss()], or [read_stata()]. The metadata is stored
#' in a single `haven_meta` attribute holding an S7 object whose class
#' depends on the vendor and on whether `x` is a data frame:
#'
#' * Column metadata: [meta_var_sas], [meta_var_spss], [meta_var_stata].
#' * Data frame metadata: [meta_df_sas], [meta_df_spss], [meta_df_stata].
#'
#' If no metadata has been set, `meta()` falls back to the legacy
#' `format.*`, `display_width`, `width`, `notes`, and `*_timestamp`
#' attributes, so objects saved by older versions of haven keep working.
#'
#' @param x A vector or data frame.
#' @param vendor If supplied, one of `"sas"`, `"spss"`, or `"stata"`.
#'   Returns an empty object of the appropriate class if no metadata is
#'   present, and errors if the existing metadata belongs to another vendor.
#' @param value `NULL` or a metadata object of the appropriate class.
#' @return `meta()` returns an S7 object inheriting from `meta_var` (for
#'   vectors) or `meta_df` (for data frames), or `NULL`.
#' @examples
#' df <- read_sav(system.file("examples", "iris.sav", package = "haven"))
#'
#' # Data frame metadata
#' meta(df)
#' meta(df)@created
#'
#' # Column metadata
#' meta(df$Species)
#' meta(df$Species)@format
#'
#' # Modify metadata
#' meta(df$Species)@measure <- "ordinal"
#' meta(df$Species)
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
#' @rdname meta
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

# Readers ------------------------------------------------------------------

# Called from C++: the readers fill in the properties of these empty objects
# directly, skipping validation (which is too slow for files with many columns)
meta_proto <- function(vendor, df) {
  meta_class(vendor, df)()
}

# Writers ------------------------------------------------------------------

# Flattens metadata into plain lists for the C++ writers. Columns only use
# metadata from the writer's vendor; data frames also use the timestamps
# from any vendor.
writer_meta <- function(data, vendor) {
  class_name <- S7_class_name(meta_class(vendor, df = FALSE))
  list(
    vars = unname(lapply(
      data,
      writer_meta_var,
      vendor = vendor,
      class_name = class_name
    )),
    df = writer_meta_df(data, vendor)
  )
}

# Called once per column, so avoids slower S7 helpers
writer_meta_var <- function(x, vendor, class_name) {
  meta <- attr(x, "haven_meta", exact = TRUE)
  if (is.null(meta)) {
    legacy_fields_var(x, vendor)
  } else if (inherits(meta, class_name)) {
    meta_fields(meta)
  } else {
    list()
  }
}

writer_meta_df <- function(data, vendor) {
  meta <- attr(data, "haven_meta", exact = TRUE)
  if (is.null(meta)) {
    legacy_fields_df(data, vendor)
  } else if (S7::S7_inherits(meta, meta_class(vendor, df = TRUE))) {
    meta_fields(meta)
  } else {
    compact(meta_fields(meta)[c("created", "modified")])
  }
}

# Properties are stored as attributes, and NULL properties are absent
meta_fields <- function(x) {
  fields <- attributes(x)
  fields$class <- NULL
  fields$S7_class <- NULL
  fields
}

compact <- function(x) {
  x[!vapply(x, is.null, logical(1))]
}

# Legacy attributes --------------------------------------------------------

legacy_meta <- function(x, vendor = NULL) {
  df <- is.data.frame(x)
  if (df) {
    vendors <- compact(lapply(x, legacy_vendor))
    vendor <- if (length(vendors)) vendors[[1]] else vendor
  } else {
    vendor <- legacy_vendor(x) %||% vendor
  }
  if (is.null(vendor)) {
    return(NULL)
  }

  fields <- if (df) {
    legacy_fields_df(x, vendor)
  } else {
    legacy_fields_var(x, vendor)
  }
  inject(meta_class(vendor, df)(!!!fields))
}

legacy_fields_var <- function(x, vendor) {
  if (is.null(attributes(x))) {
    return(list())
  }

  fields <- list(format = attr(x, paste0("format.", vendor), exact = TRUE))
  if (vendor %in% c("sas", "spss")) {
    fields$width <- attr(x, "width", exact = TRUE)
  }
  if (vendor == "spss") {
    fields$display_width <- attr(x, "display_width", exact = TRUE)
  }
  compact(fields)
}

legacy_fields_df <- function(x, vendor) {
  fields <- list(
    created = attr(x, "creation_timestamp", exact = TRUE),
    modified = attr(x, "modified_timestamp", exact = TRUE)
  )
  if (vendor %in% c("spss", "stata")) {
    fields$notes <- attr(x, "notes", exact = TRUE)
  }
  compact(fields)
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

# Used for default arguments of the writers
meta_df_field <- function(data, vendor, name) {
  meta <- attr(data, "haven_meta", exact = TRUE)
  if (S7::S7_inherits(meta, meta_class(vendor, df = TRUE))) {
    S7::prop(meta, name)
  }
}
