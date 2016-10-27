
extern int8_t   por_ascii_lookup[256];
extern uint16_t por_unicode_lookup[256];

typedef struct por_ctx_s {
    readstat_info_handler           info_handler;
    readstat_metadata_handler       metadata_handler;
    readstat_variable_handler       variable_handler;
    readstat_note_handler           note_handler;
    readstat_fweight_handler        fweight_handler;
    readstat_value_handler          value_handler;
    readstat_value_label_handler    value_label_handler;
    readstat_error_handler          error_handler;
    readstat_progress_handler       progress_handler;
    size_t                          file_size;
    void                           *user_ctx;

    int            pos;
    readstat_io_t *io;
    char           space;
    long           num_spaces;
    time_t         timestamp;
    long           version;
    char           fweight_name[9];
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
    readstat_variable_t **variables;
    spss_varinfo_t *varinfo;
    ck_hash_table_t *var_dict;
} por_ctx_t;

por_ctx_t *por_ctx_init();
void por_ctx_free(por_ctx_t *ctx);
ssize_t por_utf8_encode(const unsigned char *input, size_t input_len, 
        char *output, size_t output_len, uint16_t lookup[256]);
ssize_t por_utf8_decode(
        const char *input, size_t input_len,
        char *output, size_t output_len,
        uint8_t *lookup, size_t lookup_len);
