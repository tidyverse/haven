#include <stdlib.h>
#include <zlib.h>

#include "../readstat.h"
#include "../readstat_bits.h"
#include "../readstat_iconv.h"
#include "../readstat_malloc.h"
#include "readstat_sav.h"
#include "readstat_sav_compress.h"

struct zheader {
    uint64_t zheader_ofs;
    uint64_t ztrailer_ofs;
    uint64_t ztrailer_len;
};

struct ztrailer {
    int64_t bias;
    int64_t zero;
    int32_t block_size;
    int32_t n_blocks;
};

struct ztrailer_entry {
    int64_t uncompressed_ofs;
    int64_t compressed_ofs;
    int32_t uncompressed_size;
    int32_t compressed_size;
};

readstat_error_t zsav_read_compressed_data(sav_ctx_t *ctx,
        readstat_error_t (*row_handler)(unsigned char *, size_t, sav_ctx_t *)) {
    readstat_error_t retval = READSTAT_OK;
    readstat_io_t *io = ctx->io;
    readstat_off_t data_offset = 0;

    size_t uncompressed_row_len = ctx->var_offset * 8;
    readstat_off_t uncompressed_offset = 0;
    unsigned char *uncompressed_row = NULL;

    uLongf uncompressed_block_len = 0;
    unsigned char *compressed_block = NULL, *uncompressed_block = NULL;

    struct sav_row_stream_s state = { 
        .missing_value = ctx->missing_double,
        .bias = ctx->bias,
        .bswap = ctx->bswap };

    struct zheader zheader;
    struct ztrailer ztrailer;
    struct ztrailer_entry *ztrailer_entries = NULL;

    int n_blocks = 0;
    int block_i = 0;
    int i;

    if (io->read(&zheader, sizeof(struct zheader), io->io_ctx) < sizeof(struct zheader)) {
        retval = READSTAT_ERROR_READ;
        goto cleanup;
    }

    zheader.zheader_ofs = ctx->bswap ? byteswap8(zheader.zheader_ofs) : zheader.zheader_ofs;
    zheader.ztrailer_ofs = ctx->bswap ? byteswap8(zheader.ztrailer_ofs) : zheader.ztrailer_ofs;
    zheader.ztrailer_len = ctx->bswap ? byteswap8(zheader.ztrailer_len) : zheader.ztrailer_len;

    if (zheader.zheader_ofs != io->seek(0, READSTAT_SEEK_CUR, io->io_ctx) - sizeof(struct zheader)) {
        retval = READSTAT_ERROR_PARSE;
        goto cleanup;
    }

    n_blocks = (zheader.ztrailer_len - 24) / 24;

    if (io->seek(zheader.ztrailer_ofs, READSTAT_SEEK_SET, io->io_ctx) == -1) {
        retval = READSTAT_ERROR_SEEK;
        goto cleanup;
    }

    if (io->read(&ztrailer, sizeof(struct ztrailer), io->io_ctx) < sizeof(struct ztrailer)) {
        retval = READSTAT_ERROR_READ;
        goto cleanup;
    }

    ztrailer.bias = ctx->bswap ? byteswap8(ztrailer.bias) : ztrailer.bias;
    ztrailer.zero = ctx->bswap ? byteswap8(ztrailer.zero) : ztrailer.zero;
    ztrailer.block_size = ctx->bswap ? byteswap4(ztrailer.block_size) : ztrailer.block_size;
    ztrailer.n_blocks = ctx->bswap ? byteswap4(ztrailer.n_blocks) : ztrailer.n_blocks;

    if (n_blocks != ztrailer.n_blocks) {
        retval = READSTAT_ERROR_PARSE;
        goto cleanup;
    }

    if (n_blocks && (ztrailer_entries = readstat_malloc(n_blocks * sizeof(struct ztrailer_entry))) == NULL) {
        retval = READSTAT_ERROR_MALLOC;
        goto cleanup;
    }

    if (io->read(ztrailer_entries, n_blocks * sizeof(struct ztrailer_entry), io->io_ctx) < 
            n_blocks * sizeof(struct ztrailer_entry)) {
        retval = READSTAT_ERROR_READ;
        goto cleanup;
    }

    for (i=0; i<n_blocks; i++) {
        struct ztrailer_entry *entry = &ztrailer_entries[i];

        entry->uncompressed_ofs = ctx->bswap ? byteswap8(entry->uncompressed_ofs) : entry->uncompressed_ofs;
        entry->compressed_ofs = ctx->bswap ? byteswap8(entry->compressed_ofs) : entry->compressed_ofs;
        entry->uncompressed_size = ctx->bswap ? byteswap4(entry->uncompressed_size) : entry->uncompressed_size;
        entry->compressed_size = ctx->bswap ? byteswap4(entry->compressed_size) : entry->compressed_size;
    }

    if (uncompressed_row_len && (uncompressed_row = readstat_malloc(uncompressed_row_len)) == NULL) {
        retval = READSTAT_ERROR_MALLOC;
        goto cleanup;
    }

    while (1) {
        if (block_i == n_blocks)
            goto cleanup;

        struct ztrailer_entry *entry = &ztrailer_entries[block_i];
        if (io->seek(entry->compressed_ofs, READSTAT_SEEK_SET, io->io_ctx) == -1) {
            retval = READSTAT_ERROR_SEEK;
            goto cleanup;
        }
        if ((compressed_block = readstat_realloc(compressed_block, entry->compressed_size)) == NULL) {
            retval = READSTAT_ERROR_MALLOC;
            goto cleanup;
        }
        if (io->read(compressed_block, entry->compressed_size, io->io_ctx) != entry->compressed_size) {
            retval = READSTAT_ERROR_READ;
            goto cleanup;
        }

        uncompressed_block_len = entry->uncompressed_size;
        if ((uncompressed_block = readstat_realloc(uncompressed_block, uncompressed_block_len)) == NULL) {
            retval = READSTAT_ERROR_MALLOC;
            goto cleanup;
        }
        int status = uncompress(uncompressed_block, &uncompressed_block_len,
                compressed_block, entry->compressed_size);
        if (status != Z_OK || uncompressed_block_len != entry->uncompressed_size) {
            retval = READSTAT_ERROR_PARSE;
            goto cleanup;
        }

        block_i++;
        state.status = SAV_ROW_STREAM_HAVE_DATA;
        data_offset = 0;

        while (state.status != SAV_ROW_STREAM_NEED_DATA) {
            state.next_in = &uncompressed_block[data_offset];
            state.avail_in = uncompressed_block_len - data_offset;

            state.next_out = &uncompressed_row[uncompressed_offset];
            state.avail_out = uncompressed_row_len - uncompressed_offset;

            sav_decompress_row(&state);

            uncompressed_offset = uncompressed_row_len - state.avail_out;
            data_offset = uncompressed_block_len - state.avail_in;

            if (state.status == SAV_ROW_STREAM_FINISHED_ROW) {
                retval = row_handler(uncompressed_row, uncompressed_row_len, ctx);
                if (retval != READSTAT_OK)
                    goto cleanup;

                uncompressed_offset = 0;
            }

            if (state.status == SAV_ROW_STREAM_FINISHED_ALL)
                goto cleanup;
            if (ctx->row_limit > 0 && ctx->current_row == ctx->row_limit)
                goto cleanup;
        }
    }

cleanup:
    if (uncompressed_row)
        free(uncompressed_row);
    if (ztrailer_entries)
        free(ztrailer_entries);
    if (compressed_block)
        free(compressed_block);
    if (uncompressed_block)
        free(uncompressed_block);

    return retval;
}
