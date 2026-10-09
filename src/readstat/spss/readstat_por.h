
extern int8_t   por_ascii_lookup[256];
extern uint16_t por_unicode_lookup[256];

typedef struct por_ctx_s {
    readstat_callbacks_t    handle;
    size_t                  file_size;
    void                   *user_ctx;

    int            pos;
    readstat_io_t *io;
    char           space;
    long           num_spaces;
    time_t         timestamp;
    long           version;
    char           fweight_name[9];
    char           file_label[21];
    uint16_t       byte2unicode[256];
    size_t         base30_precision;
    iconv_t        converter;
    unsigned char *string_buffer;
    size_t         string_buffer_len;
    int            labels_offset;
    int            obs_count;
    int            var_count;
    int            var_offset;
    int            row_limit;
    int            row_offset;
    readstat_variable_t **variables;
    spss_varinfo_t *varinfo;
    ck_hash_table_t *var_dict;
} por_ctx_t;

por_ctx_t *por_ctx_init(void);
void por_ctx_free(por_ctx_t *ctx);
ssize_t por_utf8_encode(const unsigned char *input, size_t input_len,
        char *output, size_t output_len, uint16_t lookup[256]);
ssize_t por_utf8_decode(
        const char *input, size_t input_len,
        char *output, size_t output_len,
        uint8_t *lookup, size_t lookup_len);
/* Number of code points in a UTF-8 string, or -1 if it is malformed */
ssize_t por_utf8_count(const char *input, size_t input_len);
/* Exact base-30 expansion of a finite, nonzero double: fills trigs with
 * digit values 0-29 (most significant first, no leading zeros) and sets
 * *out_trig_places to the number of digits before the radix point (which may
 * be negative or exceed the digit count). Returns the digit count or -1. */
int por_double_to_trigs(double value, unsigned char *trigs, int max_trigs, int *out_trig_places);
