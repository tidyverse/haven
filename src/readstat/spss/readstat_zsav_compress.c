
#include <zlib.h>
#include <stdlib.h>
#include <stdint.h>

#include "../readstat.h"
#include "readstat_zsav_compress.h"

zsav_ctx_t *zsav_ctx_init(size_t max_row_len, int64_t offset) {
    zsav_ctx_t *ctx = calloc(1, sizeof(zsav_ctx_t));
    if (ctx == NULL)
        return NULL;

    ctx->buffer = malloc(max_row_len);

    ctx->blocks_capacity = 10;
    ctx->blocks = calloc(ctx->blocks_capacity, sizeof(zsav_block_t *));

    if (ctx->buffer == NULL || ctx->blocks == NULL) {
        zsav_ctx_free(ctx);
        return NULL;
    }

    ctx->uncompressed_block_size = 0x3FF000;
    ctx->zheader_ofs = offset;

    ctx->compression_level = Z_DEFAULT_COMPRESSION;

    return ctx;
}

static void zsav_block_free(zsav_block_t *block) {
    if (block == NULL)
        return;
    if (!block->finished)
        deflateEnd(&block->stream);
    free(block->compressed_data);
    free(block);
}

void zsav_ctx_free(zsav_ctx_t *ctx) {
    int i;
    if (ctx == NULL)
        return;
    for (i=0; i<ctx->blocks_count; i++) {
        zsav_block_free(ctx->blocks[i]);
    }
    free(ctx->blocks);
    free(ctx->buffer);
    free(ctx);
}

/* Returns NULL on allocation or deflateInit failure */
zsav_block_t *zsav_add_block(zsav_ctx_t *ctx) {
    zsav_block_t *block = NULL;
    if (ctx->blocks_count == ctx->blocks_capacity) {
        int new_capacity = ctx->blocks_capacity * 2;
        zsav_block_t **new_blocks = realloc(ctx->blocks, new_capacity * sizeof(zsav_block_t *));
        if (new_blocks == NULL)
            return NULL;
        ctx->blocks = new_blocks;
        ctx->blocks_capacity = new_capacity;
    }

    block = calloc(1, sizeof(zsav_block_t));
    if (block == NULL)
        return NULL;

    if (deflateInit(&block->stream, ctx->compression_level) != Z_OK) {
        block->finished = 1; /* nothing to deflateEnd */
        zsav_block_free(block);
        return NULL;
    }

    block->compressed_data_capacity = deflateBound(&block->stream, ctx->uncompressed_block_size);
    block->compressed_data = malloc(block->compressed_data_capacity);
    if (block->compressed_data == NULL) {
        zsav_block_free(block);
        return NULL;
    }

    ctx->blocks[ctx->blocks_count++] = block;

    return block;
}

zsav_block_t *zsav_current_block(zsav_ctx_t *ctx) {
    if (ctx->blocks_count == 0)
        return NULL;

    return ctx->blocks[ctx->blocks_count-1];
}

/* Finish the block's deflate stream, release the zlib state, and shrink the
 * compressed buffer to its final size so that memory does not stay at
 * deflateBound() for every block until the file is closed. */
static readstat_error_t zsav_finish_block(zsav_block_t *block) {
    int deflate_status = deflate(&block->stream, Z_FINISH);
    if (deflate_status != Z_STREAM_END || block->stream.avail_in != 0)
        return READSTAT_ERROR_WRITE;

    block->compressed_size = block->compressed_data_capacity - block->stream.avail_out;

    deflateEnd(&block->stream);
    block->finished = 1;

    if (block->compressed_size > 0) {
        unsigned char *shrunk = realloc(block->compressed_data, block->compressed_size);
        if (shrunk) {
            block->compressed_data = shrunk;
            block->compressed_data_capacity = block->compressed_size;
        }
    }

    return READSTAT_OK;
}

readstat_error_t zsav_compress_row(void *input, size_t input_len, int finish, zsav_ctx_t *ctx) {
    size_t row_off = 0;
    unsigned char *row_buffer = input;
    size_t row_len = input_len;
    zsav_block_t *block = zsav_current_block(ctx);
    readstat_error_t retval = READSTAT_OK;
    int deflate_status = Z_OK;

    if (block == NULL || block->finished) {
        if ((block = zsav_add_block(ctx)) == NULL)
            return READSTAT_ERROR_MALLOC;
    }

    block->stream.next_in = row_buffer;
    block->stream.avail_in = row_len;

    block->stream.next_out = &block->compressed_data[block->compressed_size];
    block->stream.avail_out = block->compressed_data_capacity - block->compressed_size;

    /* If the row won't fit into this block, keep writing and flushing
     * until the remainder fits. */
    while (row_len - row_off > ctx->uncompressed_block_size - block->uncompressed_size) {
        size_t chunk_len = ctx->uncompressed_block_size - block->uncompressed_size;

        block->stream.avail_in = chunk_len;
        row_off += chunk_len;

        if ((retval = zsav_finish_block(block)) != READSTAT_OK)
            goto cleanup;

        block->uncompressed_size = ctx->uncompressed_block_size;

        if ((block = zsav_add_block(ctx)) == NULL) {
            retval = READSTAT_ERROR_MALLOC;
            goto cleanup;
        }

        block->stream.next_in = &row_buffer[row_off];
        block->stream.avail_in = row_len - row_off;

        block->stream.next_out = block->compressed_data;
        block->stream.avail_out = block->compressed_data_capacity;
    }

    /* Now the rest of the row will fit in the block */
    if (finish) {
        if ((retval = zsav_finish_block(block)) != READSTAT_OK)
            goto cleanup;
    } else {
        deflate_status = deflate(&block->stream, Z_NO_FLUSH);
        /* The output buffer is sized by deflateBound() for the whole block,
         * so anything other than complete consumption of the input is an error */
        if (deflate_status != Z_OK || block->stream.avail_in != 0) {
            retval = READSTAT_ERROR_WRITE;
            goto cleanup;
        }
        block->compressed_size = block->compressed_data_capacity - block->stream.avail_out;
    }
    block->uncompressed_size += (row_len - row_off);

cleanup:
    return retval;
}
