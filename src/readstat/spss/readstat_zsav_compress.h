
typedef struct zsav_block_s {
    int32_t        uncompressed_size;
    int32_t        compressed_size;

    z_stream       stream;

    unsigned char *compressed_data;
    size_t         compressed_data_capacity;
} zsav_block_t;

typedef struct zsav_ctx_s {
    void           *buffer;
    zsav_block_t  **blocks;
    int             blocks_count;
    int             blocks_capacity;

    int64_t         uncompressed_block_size;
    int64_t         zheader_ofs;

    int             compression_level;
} zsav_ctx_t;

zsav_ctx_t *zsav_ctx_init(size_t max_row_len, int64_t offset);
void zsav_ctx_free(zsav_ctx_t *ctx);

zsav_block_t *zsav_add_block(zsav_ctx_t *ctx);
zsav_block_t *zsav_current_block(zsav_ctx_t *ctx);
int zsav_compress_row(void *input, size_t input_len, int finish, zsav_ctx_t *zctx);
