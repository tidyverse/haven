
#include <zlib.h>
#include <stdlib.h>
#include <stdint.h>

#include "readstat_zsav_compress.h"

zsav_ctx_t *zsav_ctx_init(size_t max_row_len, int64_t offset) {
    zsav_ctx_t *ctx = calloc(1, sizeof(zsav_ctx_t));

    ctx->buffer = malloc(max_row_len);

    ctx->blocks_capacity = 10;
    ctx->blocks = calloc(ctx->blocks_capacity, sizeof(zsav_block_t *));

    ctx->uncompressed_block_size = 0x3FF000;
    ctx->zheader_ofs = offset;

    ctx->compression_level = Z_DEFAULT_COMPRESSION;

    return ctx;
}

void zsav_ctx_free(zsav_ctx_t *ctx) {
    int i;
    for (i=0; i<ctx->blocks_count; i++) {
        zsav_block_t *block = ctx->blocks[i];
        deflateEnd(&block->stream);
        free(block->compressed_data);
        free(block);
    }
    free(ctx->blocks);
    free(ctx->buffer);
    free(ctx);
}

zsav_block_t *zsav_add_block(zsav_ctx_t *ctx) {
    zsav_block_t *block = NULL;
    if (ctx->blocks_count == ctx->blocks_capacity) {
        ctx->blocks = realloc(ctx->blocks, (ctx->blocks_capacity *= 2 ) * sizeof(zsav_block_t *));
    }

    block = calloc(1, sizeof(zsav_block_t));
    ctx->blocks[ctx->blocks_count++] = block;

    deflateInit(&block->stream, ctx->compression_level);

    block->compressed_data_capacity = deflateBound(&block->stream, ctx->uncompressed_block_size);
    block->compressed_data = malloc(block->compressed_data_capacity);

    return block;
}

zsav_block_t *zsav_current_block(zsav_ctx_t *ctx) {
    if (ctx->blocks_count == 0)
        return NULL;

    return ctx->blocks[ctx->blocks_count-1];
}

int zsav_compress_row(void *input, size_t input_len, int finish, zsav_ctx_t *ctx) {
    off_t row_off = 0;
    unsigned char *row_buffer = input;
    size_t row_len = input_len;
    zsav_block_t *block = zsav_current_block(ctx);
    int deflate_status = Z_OK;
    if (block == NULL) {
        block = zsav_add_block(ctx);
    }

    block->stream.next_in = row_buffer;
    block->stream.avail_in = row_len;

    block->stream.next_out = &block->compressed_data[block->compressed_size];
    block->stream.avail_out = block->compressed_data_capacity - block->compressed_size;

    /* If the row won't fit into this block, keep writing and flushing
     * until the remainder fits. */
    while (row_len - row_off > ctx->uncompressed_block_size - block->uncompressed_size) {

        block->stream.avail_in = ctx->uncompressed_block_size - block->uncompressed_size;
        row_off += ctx->uncompressed_block_size - block->uncompressed_size;

        if ((deflate_status = deflate(&block->stream, Z_FINISH)) != Z_STREAM_END) {
            goto cleanup;
        }

        block->compressed_size = block->compressed_data_capacity - block->stream.avail_out;
        block->uncompressed_size = ctx->uncompressed_block_size - block->stream.avail_in;

        block = zsav_add_block(ctx);

        block->stream.next_in = &row_buffer[row_off];
        block->stream.avail_in = row_len - row_off;

        block->stream.next_out = block->compressed_data;
        block->stream.avail_out = block->compressed_data_capacity;
    }

    /* Now the rest of the row will fit in the block */
    deflate_status = deflate(&block->stream, finish ? Z_FINISH : Z_NO_FLUSH);

    block->compressed_size = block->compressed_data_capacity - block->stream.avail_out;
    block->uncompressed_size += (row_len - row_off) - block->stream.avail_in;

cleanup:
    return deflate_status;
}
