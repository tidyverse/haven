#include <stdlib.h>
#include <stdint.h>
#include <zlib.h>

#include "../readstat.h"
#include "../readstat_writer.h"
#include "readstat_sav_compress.h"
#include "readstat_zsav_compress.h"
#include "readstat_zsav_write.h"

readstat_error_t zsav_write_compressed_row(void *writer_ctx, void *row, size_t len) {
    readstat_writer_t *writer = (readstat_writer_t *)writer_ctx;
    zsav_ctx_t *zctx = writer->module_ctx;
    /* Kind of frustrating that SPSS does double compression.  If they just
     * z-compressed the uncompressed data, we could calculate the block count
     * in advance and write out the file in a streaming manner. As things stand
     * we have to build up the file in memory until we know the final block
     * count. A possible streaming solution would be to declare the number of
     * blocks required to hold the maximum possible row-compressed size and
     * then fill out the end with no-op zero bytes (that get z-compressed very
     * small).
     */
    size_t row_len = sav_compress_row(zctx->buffer, row, len, writer);
    int deflate_status = zsav_compress_row(zctx->buffer, row_len,
            writer->current_row + 1 == writer->row_count, zctx);

    if (deflate_status != Z_OK && deflate_status != Z_STREAM_END)
        return READSTAT_ERROR_WRITE;

    return  READSTAT_OK;
}

static readstat_error_t zsav_write_data_header(readstat_writer_t *writer, zsav_ctx_t *zctx) {
    readstat_error_t retval = READSTAT_OK;
    uint64_t zheader_ofs = zctx->zheader_ofs;
    uint64_t ztrailer_ofs = zheader_ofs + 24;
    uint64_t ztrailer_len = 24 + zctx->blocks_count * 24;
    int i;

    for (i=0; i<zctx->blocks_count; i++) {
        zsav_block_t *block = zctx->blocks[i];
        ztrailer_ofs += block->compressed_size;
    }

    if ((retval = readstat_write_bytes(writer, &zheader_ofs, sizeof(uint64_t))) != READSTAT_OK)
        goto cleanup;

    if ((retval = readstat_write_bytes(writer, &ztrailer_ofs, sizeof(uint64_t))) != READSTAT_OK)
        goto cleanup;

    if ((retval = readstat_write_bytes(writer, &ztrailer_len, sizeof(uint64_t))) != READSTAT_OK)
        goto cleanup;

cleanup:
    return retval;
}

static readstat_error_t zsav_write_data_blocks(readstat_writer_t *writer, zsav_ctx_t *zctx) {
    readstat_error_t retval = READSTAT_OK;
    int i;
    for (i=0; i<zctx->blocks_count; i++) {
        zsav_block_t *block = zctx->blocks[i];

        if ((retval = readstat_write_bytes(writer, block->compressed_data, block->compressed_size)) != READSTAT_OK)
            goto cleanup;
    }

cleanup:
    return retval;
}

static readstat_error_t zsav_write_data_trailer(readstat_writer_t *writer, zsav_ctx_t *zctx) {
    readstat_error_t retval = READSTAT_OK;
    int64_t bias = -100;
    int64_t zero = 0;
    int32_t block_size = zctx->uncompressed_block_size;
    int32_t n_blocks = zctx->blocks_count;

    if ((retval = readstat_write_bytes(writer, &bias, sizeof(int64_t))) != READSTAT_OK)
        goto cleanup;

    if ((retval = readstat_write_bytes(writer, &zero, sizeof(int64_t))) != READSTAT_OK)
        goto cleanup;

    if ((retval = readstat_write_bytes(writer, &block_size, sizeof(int32_t))) != READSTAT_OK)
        goto cleanup;

    if ((retval = readstat_write_bytes(writer, &n_blocks, sizeof(int32_t))) != READSTAT_OK)
        goto cleanup;

    int i;
    int64_t uncompressed_ofs = zctx->zheader_ofs;
    int64_t compressed_ofs = zctx->zheader_ofs + 24;
    for (i=0; i<zctx->blocks_count; i++) {
        zsav_block_t *block = zctx->blocks[i];
        int32_t uncompressed_size = block->uncompressed_size;
        int32_t compressed_size = block->compressed_size;

        if ((retval = readstat_write_bytes(writer, &uncompressed_ofs, sizeof(int64_t))) != READSTAT_OK)
            goto cleanup;

        if ((retval = readstat_write_bytes(writer, &compressed_ofs, sizeof(int64_t))) != READSTAT_OK)
            goto cleanup;

        if ((retval = readstat_write_bytes(writer, &uncompressed_size, sizeof(int32_t))) != READSTAT_OK)
            goto cleanup;

        if ((retval = readstat_write_bytes(writer, &compressed_size, sizeof(int32_t))) != READSTAT_OK)
            goto cleanup;

        uncompressed_ofs += uncompressed_size;
        compressed_ofs += compressed_size;
    }

cleanup:
    return retval;
}

readstat_error_t zsav_end_data(void *writer_ctx) {
    readstat_writer_t *writer = (readstat_writer_t *)writer_ctx;
    zsav_ctx_t *zctx = writer->module_ctx;
    readstat_error_t retval = READSTAT_OK;

    retval = zsav_write_data_header(writer, zctx);
    if (retval != READSTAT_OK)
        goto cleanup;

    retval = zsav_write_data_blocks(writer, zctx);
    if (retval != READSTAT_OK)
        goto cleanup;

    retval = zsav_write_data_trailer(writer, zctx);
    if (retval != READSTAT_OK)
        goto cleanup;


cleanup:
    return retval;
}

