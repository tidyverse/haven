# properties are validated

    Code
      meta_var_sas(format = c("a", "b"))
    Condition
      Error:
      ! <haven::meta_var_sas> object properties are invalid:
      - @format must be a single string or NULL
    Code
      meta_var_sas(width = 0)
    Condition
      Error:
      ! <haven::meta_var_sas>@width must be 1 or greater
    Code
      meta_var_sas(width = 1.5)
    Condition
      Error:
      ! <haven::meta_var_sas>@width must be <NULL> or <integer>, not <double>
    Code
      meta_var_spss(measure = "interval")
    Condition
      Error:
      ! <haven::meta_var_spss> object properties are invalid:
      - @measure must be one of "nominal", "ordinal", "scale", not "interval"
    Code
      meta_var_spss(alignment = "middle")
    Condition
      Error:
      ! <haven::meta_var_spss> object properties are invalid:
      - @alignment must be one of "left", "right", "center", not "middle"
    Code
      meta_var_stata(type = "str10")
    Condition
      Error:
      ! <haven::meta_var_stata> object properties are invalid:
      - @type must be one of "byte", "int", "long", "float", "double", "strL", not "str10"
    Code
      meta_df_sas(xpt_version = 6)
    Condition
      Error:
      ! <haven::meta_df_sas>@xpt_version must be one of 5, 8
    Code
      meta_df_stata(version = 16)
    Condition
      Error:
      ! <haven::meta_df_stata>@version must be between 8 and 15
    Code
      meta_df_spss(notes = NA_character_)
    Condition
      Error:
      ! <haven::meta_df_spss> object properties are invalid:
      - @notes must not contain missing values
    Code
      meta_df_spss(created = Sys.time() + 1:2)
    Condition
      Error:
      ! <haven::meta_df_spss> object properties are invalid:
      - @created must be a single date-time or NULL

# meta() and meta<- check vendor and type

    Code
      meta(x, "spss")
    Condition
      Error in `meta()`:
      ! Metadata must be <haven::meta_var_spss>, not <haven::meta_var_sas>.
    Code
      meta(x, "spps")
    Condition
      Error in `meta()`:
      ! `vendor` must be one of "sas", "spss", or "stata", not "spps".
      i Did you mean "spss"?
    Code
      meta(x, "stata") <- meta_var_sas()
    Condition
      Error in `meta<-`:
      ! Metadata must be <haven::meta_var_stata>, not <haven::meta_var_sas>.
    Code
      meta(x) <- meta_df_sas()
    Condition
      Error in `meta<-`:
      ! `value` must be a <meta_var> object or `NULL`, not a <haven::meta_df_sas> object.
    Code
      meta(df) <- meta_var_sas()
    Condition
      Error in `meta<-`:
      ! `value` must be a <meta_df> object or `NULL`, not a <haven::meta_var_sas> object.
    Code
      meta(x) <- list()
    Condition
      Error in `meta<-`:
      ! `value` must be a <meta_var> object or `NULL`, not an empty list.

# legacy vendor mismatch is an error

    Code
      meta(x, "stata")
    Condition
      Error in `meta()`:
      ! Metadata must be <haven::meta_var_stata>, not <haven::meta_var_sas>.

