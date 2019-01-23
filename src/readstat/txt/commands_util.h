
typedef enum {
    LABEL_TYPE_NAN = -1,
    LABEL_TYPE_DOUBLE,
    LABEL_TYPE_STRING,
    LABEL_TYPE_RANGE
} label_type_t;

readstat_error_t submit_columns(readstat_parser_t *parser, readstat_schema_t *dct, void *user_ctx);
readstat_error_t submit_value_label(readstat_parser_t *parser, const char *labelset,
        label_type_t label_type, int first_integer, int integer, const char *string_value,
        const char *buf, void *user_ctx);
