
typedef enum {
    LABEL_TYPE_NAN = -1,
    LABEL_TYPE_DOUBLE,
    LABEL_TYPE_STRING,
    LABEL_TYPE_RANGE,
    LABEL_TYPE_OTHER
} label_type_t;

readstat_error_t submit_columns(readstat_parser_t *parser, readstat_schema_t *dct, void *user_ctx);
/* A value label range such as "1 - 10" is expanded into one label per
 * integer. Cap the total number of labels a single file may expand to so a
 * range like "0 - 99999999999999999" cannot spin for hours. */
#define MAX_VALUE_LABEL_RANGE_VALUES (1 << 20)

readstat_error_t submit_value_label(readstat_parser_t *parser, const char *labelset,
        label_type_t label_type, int64_t first_integer, int64_t last_integer,
        double double_value, const char *string_value, const char *buf,
        uint64_t *range_values_remaining, void *user_ctx);
