 //
//  readstat_rdata.c
//  Wizard
//
//  Created by Evan Miller on 3/31/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <math.h>
#include <lzma.h>
#include <zlib.h>

#include "readstat_io.h"
#include "readstat_rdata.h"

#define RDATA_ATOM_LEN 128

#define STREAM_BUFFER_SIZE   65536

typedef struct rdata_atom_table_s {
    int   count;
    char *data;
} rdata_atom_table_t;

typedef struct rdata_ctx_s {
    int                                  machine_needs_byteswap;
    readstat_handle_table_callback       handle_table;
    readstat_handle_column_callback      handle_column;
    readstat_handle_column_name_callback handle_column_name;
    readstat_handle_text_value_callback  handle_text_value;
    readstat_handle_text_value_callback  handle_value_label;
    void                                *user_ctx;
    lzma_stream                         *lzma_strm;
    z_stream                            *z_strm;
    unsigned char                       *strm_buffer;
    int                                  fd;
    
    rdata_atom_table_t                  *atom_table;
    int                                  class_is_posixct;
} rdata_ctx_t;

static int atom_table_add(rdata_atom_table_t *table, char *key);
static char *atom_table_lookup(rdata_atom_table_t *table, int index);

static readstat_error_t read_environment(const char *table_name, rdata_ctx_t *ctx);
static readstat_error_t read_toplevel_object(const char *table_name, const char *key, rdata_ctx_t *ctx);
static readstat_error_t read_sexptype_header(rdata_sexptype_info_t *header, rdata_ctx_t *ctx);
static readstat_error_t read_length(int32_t *outLength, rdata_ctx_t *ctx);
static readstat_error_t read_string_vector(int32_t length, readstat_handle_text_value_callback handle_text, 
        void *callback_ctx, rdata_ctx_t *ctx);
static readstat_error_t read_value_vector(rdata_sexptype_header_t header, const char *name, rdata_ctx_t *ctx);
static readstat_error_t read_character_string(char *key, size_t keylen, rdata_ctx_t *ctx);
static readstat_error_t read_generic_list(int attributes, rdata_ctx_t *ctx);
static readstat_error_t read_attributes(int (*handle_attribute)(char *key, rdata_sexptype_info_t val_info, rdata_ctx_t *ctx),
                           rdata_ctx_t *ctx);
static readstat_error_t recursive_discard(rdata_sexptype_header_t sexptype_header, rdata_ctx_t *ctx);

static int atom_table_add(rdata_atom_table_t *table, char *key) {
    table->data = realloc(table->data, RDATA_ATOM_LEN * (table->count + 1));
    memcpy(&table->data[RDATA_ATOM_LEN*table->count], key, strlen(key) + 1);
    table->count++;
    return table->count;
}

static char *atom_table_lookup(rdata_atom_table_t *table, int index) {
    return &table->data[(index-1)*RDATA_ATOM_LEN];
}

static int read_st_z(rdata_ctx_t *ctx, void *buffer, size_t len) {
    int bytes_written = 0;
    int error = 0;
    int result = Z_OK;
    while (1) {
        long start_out = ctx->z_strm->total_out;

        ctx->z_strm->next_out = buffer + bytes_written;
        ctx->z_strm->avail_out = len - bytes_written;

        result = inflate(ctx->z_strm, Z_SYNC_FLUSH);

        if (result != Z_OK && result != Z_STREAM_END) {
            error = -1;
            break;
        }

        bytes_written += ctx->z_strm->total_out - start_out;
        
        if (result == Z_STREAM_END)
            break;

        if (ctx->z_strm->avail_in == 0) {
            int bytes_read = 0;
            bytes_read = read(ctx->fd, ctx->strm_buffer, STREAM_BUFFER_SIZE);
            if (bytes_read < 0) {
                error = bytes_read;
                break;
            }
            if (bytes_read == 0)
                break;

            ctx->z_strm->next_in = ctx->strm_buffer;
            ctx->z_strm->avail_in = bytes_read;
        }
        if (bytes_written == len)
            break;
    }

    if (error != 0)
        return error;

    return bytes_written;
}

static int read_st_lzma(rdata_ctx_t *ctx, void *buffer, size_t len) {
    int bytes_written = 0;
    int error = 0;
    int result = LZMA_OK;
    while (1) {
        long start_out = ctx->lzma_strm->total_out;

        ctx->lzma_strm->next_out = buffer + bytes_written;
        ctx->lzma_strm->avail_out = len - bytes_written;

        result = lzma_code(ctx->lzma_strm, LZMA_RUN);

        if (result != LZMA_OK && result != LZMA_STREAM_END) {
            error = -1;
            break;
        }

        bytes_written += ctx->lzma_strm->total_out - start_out;
        
        if (result == LZMA_STREAM_END)
            break;

        if (ctx->lzma_strm->avail_in == 0) {
            int bytes_read = 0;
            bytes_read = read(ctx->fd, ctx->strm_buffer, STREAM_BUFFER_SIZE);
            if (bytes_read < 0) {
                error = bytes_read;
                break;
            }
            if (bytes_read == 0)
                break;

            ctx->lzma_strm->next_in = ctx->strm_buffer;
            ctx->lzma_strm->avail_in = bytes_read;
        }
        if (bytes_written == len)
            break;
    }

    if (error != 0)
        return error;

    return bytes_written;
}

static int read_st(rdata_ctx_t *ctx, void *buffer, size_t len) {
    if (ctx->lzma_strm)
        return read_st_lzma(ctx, buffer, len);

    if (ctx->z_strm)
        return read_st_z(ctx, buffer, len);

    return read(ctx->fd, buffer, len);
}

static int lseek_st(rdata_ctx_t *ctx, size_t len) {
    if (ctx->lzma_strm || ctx->z_strm) {
        int retval = 0;
        char *buf = malloc(len);
        if (read_st(ctx, buf, len) != len)
            retval = -1;
        free(buf);
        return retval;
    }

    return lseek(ctx->fd, len, SEEK_CUR);
}

static readstat_error_t init_z_stream(rdata_ctx_t *ctx) {
    readstat_error_t retval = READSTAT_OK;
    ctx->z_strm = calloc(1, sizeof(z_stream));
    ctx->strm_buffer = malloc(STREAM_BUFFER_SIZE);
    int bytes_read = read(ctx->fd, ctx->strm_buffer, STREAM_BUFFER_SIZE);
    if (bytes_read <= 0) {
        retval = READSTAT_ERROR_READ;
        goto cleanup;
    }

    ctx->z_strm->next_in = ctx->strm_buffer;
    ctx->z_strm->avail_in = bytes_read;

    if (inflateInit(ctx->z_strm) != Z_OK) {
        retval = READSTAT_ERROR_MALLOC;
        goto cleanup;
    }
cleanup:
    return retval;
}

static readstat_error_t init_lzma_stream(rdata_ctx_t *ctx) {
    readstat_error_t retval = READSTAT_OK;
    ctx->lzma_strm = calloc(1, sizeof(lzma_stream));
    ctx->strm_buffer = malloc(STREAM_BUFFER_SIZE);
    if (lzma_stream_decoder(ctx->lzma_strm, UINT64_MAX, 0) != LZMA_OK) {
        retval = READSTAT_ERROR_MALLOC;
        goto cleanup;
    }
    int bytes_read = read(ctx->fd, ctx->strm_buffer, STREAM_BUFFER_SIZE);
    if (bytes_read <= 0) {
        retval = READSTAT_ERROR_READ;
        goto cleanup;
    }

    ctx->lzma_strm->next_in = ctx->strm_buffer;
    ctx->lzma_strm->avail_in = bytes_read;
cleanup:
    return retval;
}

static readstat_error_t init_stream(rdata_ctx_t *ctx) {
    readstat_error_t retval = READSTAT_OK;
    char header[5];
    
    if (read(ctx->fd, &header, sizeof(header)) != sizeof(header)) {
        retval = READSTAT_ERROR_READ;
        goto cleanup;
    }

    lseek(ctx->fd, 0, SEEK_SET);

    if (header[0] == '\x1f' && header[1] == '\x8b') {
        return init_z_stream(ctx);
    }
    if (strncmp("\xFD" "7zXZ", header, sizeof(header)) == 0) {
        return init_lzma_stream(ctx);
    }

cleanup:
    return retval;
}

rdata_ctx_t *init_rdata_ctx(const char *filename) {
    int fd = readstat_open(filename);
    if (fd == -1) {
        return NULL;
    }
    rdata_ctx_t *ctx = calloc(1, sizeof(rdata_ctx_t));
    rdata_atom_table_t *atom_table = malloc(sizeof(rdata_atom_table_t));
    
    atom_table->count = 0;
    atom_table->data = NULL;

    ctx->atom_table = atom_table;

    ctx->machine_needs_byteswap = 0;
    if (machine_is_little_endian()) {
        ctx->machine_needs_byteswap = 1;
    }

    ctx->fd = fd;
    
    return ctx;
}

void free_rdata_ctx(rdata_ctx_t *ctx) {
    readstat_close(ctx->fd);
    free(ctx->atom_table);
    if (ctx->lzma_strm) {
        lzma_end(ctx->lzma_strm);
        free(ctx->lzma_strm);
    }
    if (ctx->z_strm) {
        inflateEnd(ctx->z_strm);
        free(ctx->z_strm);
    }
    if (ctx->strm_buffer) {
        free(ctx->strm_buffer);
    }
    free(ctx);
}

readstat_error_t parse_internal(const char *filename, int is_rdata, void *user_ctx,
                readstat_handle_table_callback handle_table,
                readstat_handle_column_callback handle_column,
                readstat_handle_column_name_callback handle_column_name,
                readstat_handle_text_value_callback handle_text_value,
                readstat_handle_text_value_callback handle_value_label) {
    readstat_error_t retval = READSTAT_OK;
    rdata_v2_header_t v2_header;
    rdata_ctx_t *ctx = init_rdata_ctx(filename);

    if (ctx == NULL) {
        retval = READSTAT_ERROR_OPEN;
        goto cleanup;
    }

    ctx->user_ctx = user_ctx;
    ctx->handle_table = handle_table;
    ctx->handle_column = handle_column;
    ctx->handle_column_name = handle_column_name;
    ctx->handle_text_value = handle_text_value;
    ctx->handle_value_label = handle_value_label;
    
    if ((retval = init_stream(ctx)) != READSTAT_OK) {
        goto cleanup;
    }

    if (is_rdata) {
        char header_line[5];
        if (read_st(ctx, &header_line, sizeof(header_line)) != sizeof(header_line)) {
            retval = READSTAT_ERROR_READ;
            goto cleanup;
        }
        if (strncmp("RDX2\n", header_line, sizeof(header_line)) != 0) {
            retval = READSTAT_ERROR_PARSE;
            goto cleanup;
        }
    }
    
    if (read_st(ctx, &v2_header, sizeof(v2_header)) != sizeof(v2_header)) {
        retval = READSTAT_ERROR_READ;
        goto cleanup;
    }
    
    if (ctx->machine_needs_byteswap) {
        v2_header.format_version = byteswap4(v2_header.format_version);
        v2_header.writer_version = byteswap4(v2_header.writer_version);
        v2_header.reader_version = byteswap4(v2_header.reader_version);
    }
    
    if (is_rdata) {
        retval = read_environment(NULL, ctx);
    } else {
        retval = read_toplevel_object(NULL, NULL, ctx);
    }
    if (retval != READSTAT_OK)
        goto cleanup;
    
    char test;
    
    if (read_st(ctx, &test, 1) == 1) {
        retval = READSTAT_ERROR_PARSE;
        goto cleanup;
    }
    
cleanup:
    if (ctx) {
        free_rdata_ctx(ctx);
    }
    
    return retval;
}

readstat_error_t parse_rds(const char *filename, void *user_ctx,
        readstat_handle_column_callback handle_column,
        readstat_handle_column_name_callback handle_column_name,
        readstat_handle_text_value_callback handle_text_value,
        readstat_handle_text_value_callback handle_value_label) {
    return parse_internal(filename, 0, user_ctx, NULL, handle_column, handle_column_name,
            handle_text_value, handle_value_label);
}

readstat_error_t parse_rdata(const char *filename, void *user_ctx,
                readstat_handle_table_callback handle_table,
                readstat_handle_column_callback handle_column,
                readstat_handle_column_name_callback handle_column_name,
                readstat_handle_text_value_callback handle_text_value,
                readstat_handle_text_value_callback handle_value_label) {
    return parse_internal(filename, 1, user_ctx, handle_table, handle_column, handle_column_name,
            handle_text_value, handle_value_label);
}

static readstat_error_t read_toplevel_object(const char *table_name, const char *key, rdata_ctx_t *ctx) {
    rdata_sexptype_info_t sexptype_info;
    readstat_error_t retval = READSTAT_OK;
    
    if ((retval = read_sexptype_header(&sexptype_info, ctx)) != READSTAT_OK)
        goto cleanup;

    if (sexptype_info.header.type == RDATA_SEXPTYPE_REAL_VECTOR ||
            sexptype_info.header.type == RDATA_SEXPTYPE_INTEGER_VECTOR ||
            sexptype_info.header.type == RDATA_SEXPTYPE_LOGICAL_VECTOR) {
        if (table_name == NULL && ctx->handle_table) {
            if (ctx->handle_table(key, ctx->user_ctx)) {
                retval = READSTAT_ERROR_USER_ABORT;
                goto cleanup;
            }   
        }
        
        if ((retval = read_value_vector(sexptype_info.header, key, ctx)) != READSTAT_OK)
            goto cleanup;
    } else if (sexptype_info.header.type == RDATA_SEXPTYPE_CHARACTER_VECTOR) {
        if (table_name == NULL && ctx->handle_table) {
            if (ctx->handle_table(key, ctx->user_ctx)) {
                retval = READSTAT_ERROR_USER_ABORT;
                goto cleanup;
            }   
        }
        int32_t length;
        
        if ((retval = read_length(&length, ctx)) != READSTAT_OK)
            goto cleanup;

        if (ctx->handle_column) {
            if (ctx->handle_column(key, READSTAT_TYPE_STRING, NULL, NULL, length, ctx->user_ctx)) {
                retval = READSTAT_ERROR_USER_ABORT;
                goto cleanup;
            }
        }
        
        if ((retval = read_string_vector(length, ctx->handle_text_value, ctx->user_ctx, ctx)) != READSTAT_OK)
            goto cleanup;
    } else if (sexptype_info.header.type == RDATA_SEXPTYPE_GENERIC_VECTOR &&
            sexptype_info.header.object && sexptype_info.header.attributes) {
        if (table_name != NULL) {
            retval = recursive_discard(sexptype_info.header, ctx);
        } else {
            if (ctx->handle_table) {
                if (ctx->handle_table(key, ctx->user_ctx)) {
                    retval = READSTAT_ERROR_USER_ABORT;
                    goto cleanup;
                }
            }
            retval = read_generic_list(sexptype_info.header.attributes, ctx);
        }
        if (retval != READSTAT_OK)
            goto cleanup;
    } else {
        if ((retval = recursive_discard(sexptype_info.header, ctx)) != READSTAT_OK)
            goto cleanup;
    }

cleanup:

    return retval;
}

static readstat_error_t read_environment(const char *table_name, rdata_ctx_t *ctx) {
    readstat_error_t retval = READSTAT_OK;
    
    while (1) {
        rdata_sexptype_info_t sexptype_info;
        
        if ((retval = read_sexptype_header(&sexptype_info, ctx)) != READSTAT_OK)
            goto cleanup;
        
        if (sexptype_info.header.type == RDATA_PSEUDO_SXP_NIL)
            break;
        
        if (sexptype_info.header.type != RDATA_SEXPTYPE_PAIRLIST) {
            if ((retval = recursive_discard(sexptype_info.header, ctx)) != READSTAT_OK)
                goto cleanup;
            continue;
        }
        
        char *key = atom_table_lookup(ctx->atom_table, sexptype_info.ref);
        
        if ((retval = read_toplevel_object(table_name, key, ctx)) != READSTAT_OK)
            goto cleanup;
    }
    
cleanup:
    
    return retval;
}

static readstat_error_t read_sexptype_header(rdata_sexptype_info_t *header_info, rdata_ctx_t *ctx) {
    uint32_t sexptype;
    rdata_sexptype_header_t header;
    readstat_error_t retval = READSTAT_OK;
    if (read_st(ctx, &sexptype, sizeof(sexptype)) != sizeof(sexptype)) {
        retval = READSTAT_ERROR_READ;
        goto cleanup;
    }
    if (ctx->machine_needs_byteswap)
        sexptype = byteswap4(sexptype);
    
    memcpy(&header, &sexptype, sizeof(sexptype));
    uint32_t attributes = 0, tag = 0, ref = 0;

    if (header.type == RDATA_SEXPTYPE_PAIRLIST) {
        if (header.attributes) {
            if (read_st(ctx, &attributes, sizeof(attributes)) != sizeof(attributes)) {
                retval = READSTAT_ERROR_READ;
                goto cleanup;
            }
            if (ctx->machine_needs_byteswap)
                header_info->attributes = byteswap4(header_info->attributes);
        }
        if (header.tag) {
            if (read_st(ctx, &tag, sizeof(tag)) != sizeof(tag)) {
                retval = READSTAT_ERROR_READ;
                goto cleanup;
            }
            if (ctx->machine_needs_byteswap)
                tag = byteswap4(tag);
        }
        
        if (tag == 1) {
            rdata_sexptype_info_t key_info;
            
            if ((retval = read_sexptype_header(&key_info, ctx)) != READSTAT_OK)
                goto cleanup;
            
            if (key_info.header.type != RDATA_SEXPTYPE_CHARACTER_STRING) {
                retval = READSTAT_ERROR_PARSE;
                goto cleanup;
            }
                        
            char key[RDATA_ATOM_LEN];
            
            if ((retval = read_character_string(key, RDATA_ATOM_LEN, ctx)) != READSTAT_OK)
                goto cleanup;

            ref = atom_table_add(ctx->atom_table, key);
        } else if ((tag & 0xFF) == RDATA_PSEUDO_SXP_REF) {
            ref = (tag >> 8);
        }
    }
    
    header_info->header = header;
    header_info->attributes = attributes;
    header_info->tag = tag;
    header_info->ref = ref;

cleanup:
    
    return retval;
}

static int handle_class_name(const char *buf, int i, void *ctx) {
    int *class_is_posixct = (int *)ctx;
    *class_is_posixct |= (strcmp(buf, "POSIXct") == 0);
    return READSTAT_OK;
}

static int handle_vector_attribute(char *key, rdata_sexptype_info_t val_info, rdata_ctx_t *ctx) {
    readstat_error_t retval = READSTAT_OK;
    if (strcmp(key, "levels") == 0) {
        int32_t length;
        
        if ((retval = read_length(&length, ctx)) != READSTAT_OK)
            return retval;
        
        retval = read_string_vector(length, ctx->handle_value_label, ctx->user_ctx, ctx);
    } else if (strcmp(key, "class") == 0) {
        int32_t length;
        
        if ((retval = read_length(&length, ctx)) != READSTAT_OK)
            return retval;

        ctx->class_is_posixct = 0;
        retval = read_string_vector(length, &handle_class_name, &ctx->class_is_posixct, ctx);
    } else {
        retval = recursive_discard(val_info.header, ctx);
    }
    return retval;
}

static readstat_error_t read_character_string(char *key, size_t keylen, rdata_ctx_t *ctx) {
    uint32_t length;
    readstat_error_t retval = READSTAT_OK;
    
    if (read_st(ctx, &length, sizeof(length)) != sizeof(length)) {
        retval = READSTAT_ERROR_READ;
        goto cleanup;
    }
    
    if (ctx->machine_needs_byteswap)
        length = byteswap4(length);
    
    if (length == -1) {
        key[0] = '\0';
        return 0;
    }
    
    if (length + 1 > keylen) {
        retval = READSTAT_ERROR_PARSE;
        goto cleanup;
    }
    
    if (read_st(ctx, key, length) != length) {
        retval = READSTAT_ERROR_READ;
        goto cleanup;
    }
    
    key[length] = '\0';

cleanup:
    
    return retval;
}

static int handle_data_frame_attribute(char *key, rdata_sexptype_info_t val_info, rdata_ctx_t *ctx) {
    readstat_error_t retval = READSTAT_OK;
    
    if (strcmp(key, "names") == 0 && val_info.header.type == RDATA_SEXPTYPE_CHARACTER_VECTOR) {
        int32_t length;
        
        if ((retval = read_length(&length, ctx)) != READSTAT_OK)
            return retval;
        
        retval = read_string_vector(length, ctx->handle_column_name, ctx->user_ctx, ctx);
    } else if (strcmp(key, "label.table") == 0) {
        retval = recursive_discard(val_info.header, ctx);
    } else {
        retval = recursive_discard(val_info.header, ctx);
    }
    
    return retval;
}

static readstat_error_t read_attributes(int (*handle_attribute)(char *key, rdata_sexptype_info_t val_info, rdata_ctx_t *ctx),
                           rdata_ctx_t *ctx) {
    rdata_sexptype_info_t pairlist_info, val_info;
    readstat_error_t retval = READSTAT_OK;
    
    retval = read_sexptype_header(&pairlist_info, ctx);
    if (retval != READSTAT_OK)
        goto cleanup;
    
    while (pairlist_info.header.type == RDATA_SEXPTYPE_PAIRLIST) {
        /* value */
        if ((retval = read_sexptype_header(&val_info, ctx)) != READSTAT_OK)
            goto cleanup;
        
        if (handle_attribute) {
            char *key = atom_table_lookup(ctx->atom_table, pairlist_info.ref);
            if ((retval = handle_attribute(key, val_info, ctx)) != READSTAT_OK)
                goto cleanup;
        } else {
            if ((retval = recursive_discard(val_info.header, ctx)) != READSTAT_OK)
                goto cleanup;
        }
        
        /* next */
        if ((retval = read_sexptype_header(&pairlist_info, ctx)) != READSTAT_OK)
            goto cleanup;
    }

cleanup:
    
    return retval;
}

static readstat_error_t read_generic_list(int attributes, rdata_ctx_t *ctx) {
    readstat_error_t retval = READSTAT_OK;
    int32_t length;
    int i;
    rdata_sexptype_info_t sexptype_info;
    
    
    if ((retval = read_length(&length, ctx)) != READSTAT_OK)
        goto cleanup;
    
    for (i=0; i<length; i++) {        
        if ((retval = read_sexptype_header(&sexptype_info, ctx)) != READSTAT_OK)
            goto cleanup;
        
        if (sexptype_info.header.type == RDATA_SEXPTYPE_CHARACTER_VECTOR) {
            int32_t vec_length;
            
            if ((retval = read_length(&vec_length, ctx)) != READSTAT_OK)
                goto cleanup;
            if (ctx->handle_column) {
                if (ctx->handle_column(NULL, READSTAT_TYPE_STRING, NULL, NULL, vec_length, ctx->user_ctx)) {
                    retval = READSTAT_ERROR_USER_ABORT;
                    goto cleanup;
                }
            }
            retval = read_string_vector(vec_length, ctx->handle_text_value, ctx->user_ctx, ctx);
        } else {
            retval = read_value_vector(sexptype_info.header, NULL, ctx);
        }
        if (retval != READSTAT_OK)
            goto cleanup;
    }
    
    if (attributes) {
        if ((retval = read_attributes(&handle_data_frame_attribute, ctx)) != READSTAT_OK)
            goto cleanup;
    }
    
cleanup:
    
    return retval;
}

static readstat_error_t read_length(int32_t *outLength, rdata_ctx_t *ctx) {
    int32_t length;
    readstat_error_t retval = READSTAT_OK;
    
    if (read_st(ctx, &length, sizeof(length)) != sizeof(length)) {
        retval = READSTAT_ERROR_READ;
        goto cleanup;
    }
    
    if (ctx->machine_needs_byteswap)
        length = byteswap4(length);
    
    if (outLength)
        *outLength = length;
    
cleanup:
    
    return retval;
}

static readstat_error_t read_string_vector(int32_t length, readstat_handle_text_value_callback handle_text, 
        void *callback_ctx, rdata_ctx_t *ctx) {
    int32_t string_length;
    readstat_error_t retval = READSTAT_OK;
    rdata_sexptype_info_t info;
    size_t buffer_size = 4096;
    char *buffer = NULL;
    int i;

    buffer = malloc(buffer_size);
    
    for (i=0; i<length; i++) {
        if ((retval = read_sexptype_header(&info, ctx)) != READSTAT_OK)
            goto cleanup;
        
        if (info.header.type != RDATA_SEXPTYPE_CHARACTER_STRING) {
            retval = READSTAT_ERROR_PARSE;
            goto cleanup;
        }

        if ((retval = read_length(&string_length, ctx)) != READSTAT_OK)
            goto cleanup;
        
        if (string_length + 1 > buffer_size) {
            buffer = realloc(buffer, string_length + 1);
            if (buffer == NULL) {
                retval = READSTAT_ERROR_MALLOC;
                goto cleanup;
            }
        }
        
        if (read_st(ctx, buffer, string_length) != string_length) {
            retval = READSTAT_ERROR_READ;
            goto cleanup;
        }
        
        buffer[string_length] = '\0';
        
        if (handle_text) {
            if (handle_text(buffer, i, callback_ctx)) {
                retval = READSTAT_ERROR_USER_ABORT;
                goto cleanup;
            }
        }
    }
cleanup:
    
    if (buffer)
        free(buffer);

    return retval;
}


static readstat_error_t read_value_vector(rdata_sexptype_header_t header, const char *name, rdata_ctx_t *ctx) {
    readstat_error_t retval = READSTAT_OK;
    int32_t length;
    size_t input_elem_size = 0;
    void *vals = NULL;
    size_t buf_len = 0;
    int output_data_type;
    int i;
    
    switch (header.type) {
        case RDATA_SEXPTYPE_REAL_VECTOR:
            input_elem_size = sizeof(double);
            output_data_type = READSTAT_TYPE_DOUBLE;
            break;
        case RDATA_SEXPTYPE_INTEGER_VECTOR:
            input_elem_size = sizeof(int32_t);
            output_data_type = READSTAT_TYPE_DOUBLE;
            break;
        case RDATA_SEXPTYPE_LOGICAL_VECTOR:
            input_elem_size = sizeof(int32_t);
            output_data_type = READSTAT_TYPE_DOUBLE;
            break;
        default:
            retval = READSTAT_ERROR_PARSE;
            break;
    }
    if (retval != READSTAT_OK)
        goto cleanup;

    if ((retval = read_length(&length, ctx)) != READSTAT_OK)
        goto cleanup;

    buf_len = length * input_elem_size;
    
    vals = malloc(buf_len);
    if (vals == NULL) {
        retval = READSTAT_ERROR_MALLOC;
        goto cleanup;
    }
    
    if (read_st(ctx, vals, buf_len) != buf_len) {
        retval = READSTAT_ERROR_READ;
        goto cleanup;
    }
    
    if (ctx->machine_needs_byteswap) {
        if (input_elem_size == sizeof(double)) {
            double *d_vals = (double *)vals;
            for (i=0; i<buf_len/sizeof(double); i++) {
                d_vals[i] = byteswap_double(d_vals[i]);
            }
        } else {
            uint32_t *i_vals = (uint32_t *)vals;
            for (i=0; i<buf_len/sizeof(uint32_t); i++) {
                i_vals[i] = byteswap4(i_vals[i]);
            }
        }
    }
    
    ctx->class_is_posixct = 0;
    if (header.attributes) {
        if ((retval = read_attributes(&handle_vector_attribute, ctx)) != READSTAT_OK)
            goto cleanup;
    }
    
    if (ctx->handle_column) {
        if (header.type == RDATA_SEXPTYPE_LOGICAL_VECTOR ||
                header.type == RDATA_SEXPTYPE_INTEGER_VECTOR) {
            double *real_vals = malloc(length * sizeof(double));
            int32_t *i_vals = (int32_t *)vals;
            for (i=0; i<length; i++) {
                if (i_vals[i] == INT32_MIN) {
                    real_vals[i] = NAN;
                } else {
                    real_vals[i] = i_vals[i];
                }
            }
            if (ctx->handle_column(name, output_data_type, NULL, real_vals, length, ctx->user_ctx)) {
                retval = READSTAT_ERROR_USER_ABORT;
                goto cleanup;
            }
            free(real_vals);
        } else {
            if (ctx->handle_column(name, output_data_type, ctx->class_is_posixct ? "%ts" : NULL, vals, length, ctx->user_ctx)) {
                retval = READSTAT_ERROR_USER_ABORT;
                goto cleanup;
            }
        }
    }

cleanup:
    
    return retval;
}

static readstat_error_t discard_vector(rdata_sexptype_header_t sexptype_header, size_t element_size, rdata_ctx_t *ctx) {
    int32_t length;
    readstat_error_t retval = READSTAT_OK;
    
    if ((retval = read_length(&length, ctx)) != READSTAT_OK)
        goto cleanup;
    
    if (length > 0) {
        if (lseek_st(ctx, length * element_size) == -1) {
            return READSTAT_ERROR_READ;
        }
    } else {
        fprintf(stderr, "Vector with non-positive length: %d\n", length);
    }
    
    if (sexptype_header.attributes) {
        rdata_sexptype_info_t temp_info;
        if ((retval = read_sexptype_header(&temp_info, ctx)) != READSTAT_OK)
            goto cleanup;
        
        retval = recursive_discard(temp_info.header, ctx);
    }
    
cleanup:
    
    return retval;
}

static readstat_error_t discard_character_string(int add_to_table, rdata_ctx_t *ctx) {
    readstat_error_t retval = READSTAT_OK;
    char key[RDATA_ATOM_LEN];
    
    if ((retval = read_character_string(key, RDATA_ATOM_LEN, ctx)) != READSTAT_OK)
        goto cleanup;
    
    if (strlen(key) > 0 && add_to_table) {
        atom_table_add(ctx->atom_table, key);
    }
    
cleanup:
    
    return retval;
}

static readstat_error_t discard_pairlist(rdata_sexptype_header_t sexptype_header, rdata_ctx_t *ctx) {
    rdata_sexptype_info_t temp_info;
    readstat_error_t error = 0;
    while (1) {
        switch (sexptype_header.type) {
            case RDATA_SEXPTYPE_PAIRLIST:                
                /* value */
                if ((error = read_sexptype_header(&temp_info, ctx)) != READSTAT_OK)
                    return error;
                if ((error = recursive_discard(temp_info.header, ctx)) != READSTAT_OK)
                    return error;
                
                /* tail */
                if ((error = read_sexptype_header(&temp_info, ctx)) != READSTAT_OK)
                    return error;
                sexptype_header = temp_info.header;
                break;
            case RDATA_PSEUDO_SXP_NIL:
                goto done;
            default:
                return READSTAT_ERROR_PARSE;
        }
    }
done:
    
    return 0;
}

static readstat_error_t recursive_discard(rdata_sexptype_header_t sexptype_header, rdata_ctx_t *ctx) {
    uint32_t length;
    rdata_sexptype_info_t info;
    rdata_sexptype_info_t prot, tag;
    
    readstat_error_t error = 0;
    int i;

    switch (sexptype_header.type) {
        case RDATA_SEXPTYPE_SYMBOL:
            if ((error = read_sexptype_header(&info, ctx)) != READSTAT_OK)
                goto cleanup;
            
            if ((error = recursive_discard(info.header, ctx)) != READSTAT_OK)
                goto cleanup;
            break;
        case RDATA_PSEUDO_SXP_PERSIST:
        case RDATA_PSEUDO_SXP_NAMESPACE:
        case RDATA_PSEUDO_SXP_PACKAGE:
            if ((error = read_sexptype_header(&info, ctx)) != READSTAT_OK)
                goto cleanup;
            
            if ((error = recursive_discard(info.header, ctx)) != READSTAT_OK)
                goto cleanup;
            break;
        case RDATA_SEXPTYPE_BUILTIN_FUNCTION:
        case RDATA_SEXPTYPE_SPECIAL_FUNCTION:
            error = discard_character_string(0, ctx);
            break;
        case RDATA_SEXPTYPE_PAIRLIST:
            error = discard_pairlist(sexptype_header, ctx);
            break;
        case RDATA_SEXPTYPE_CHARACTER_STRING:
            error = discard_character_string(1, ctx);
            break;
        case RDATA_SEXPTYPE_RAW_VECTOR:
            error = discard_vector(sexptype_header, 1, ctx);
            break;
        case RDATA_SEXPTYPE_LOGICAL_VECTOR:
            error = discard_vector(sexptype_header, 4, ctx);
            break;
        case RDATA_SEXPTYPE_INTEGER_VECTOR:
            error = discard_vector(sexptype_header, 4, ctx);
            break;
        case RDATA_SEXPTYPE_REAL_VECTOR:
            error = discard_vector(sexptype_header, 8, ctx);
            break;
        case RDATA_SEXPTYPE_COMPLEX_VECTOR:
            error = discard_vector(sexptype_header, 16, ctx);
            break;
        case RDATA_SEXPTYPE_CHARACTER_VECTOR:
            if (read_st(ctx, &length, sizeof(length)) != sizeof(length)) {
                return READSTAT_ERROR_READ;
            }
            if (ctx->machine_needs_byteswap)
                length = byteswap4(length);
            
            for (i=0; i<length; i++) {
                error = read_sexptype_header(&info, ctx);
                if (error != READSTAT_OK)
                    goto cleanup;
                if (info.header.type != RDATA_SEXPTYPE_CHARACTER_STRING) {
                    error = READSTAT_ERROR_PARSE;
                    goto cleanup;
                }
                
                error = discard_character_string(0, ctx);
                if (error != READSTAT_OK)
                    goto cleanup;
            }
            break;
        case RDATA_SEXPTYPE_GENERIC_VECTOR:
        case RDATA_SEXPTYPE_EXPRESSION_VECTOR:
            if (read_st(ctx, &length, sizeof(length)) != sizeof(length)) {
                return READSTAT_ERROR_READ;
            }
            if (ctx->machine_needs_byteswap)
                length = byteswap4(length);
            
            for (i=0; i<length; i++) {
                if ((error = read_sexptype_header(&info, ctx)) != READSTAT_OK)
                    goto cleanup;
                if ((error = recursive_discard(info.header, ctx)) != READSTAT_OK)
                    goto cleanup;
            }
            if (sexptype_header.attributes) {
                if ((error = read_attributes(NULL, ctx)) != READSTAT_OK)
                    goto cleanup;
            }
            break;
        case RDATA_SEXPTYPE_DOT_DOT_DOT:
        case RDATA_SEXPTYPE_PROMISE:
        case RDATA_SEXPTYPE_LANGUAGE_OBJECT: 
        case RDATA_SEXPTYPE_CLOSURE:
            if (sexptype_header.attributes) {
                if ((error = read_sexptype_header(&info, ctx)) != READSTAT_OK)
                    goto cleanup;
                
                if ((error = recursive_discard(info.header, ctx)) != READSTAT_OK)
                    goto cleanup;
            }
            if (sexptype_header.tag) {
                if ((error = read_sexptype_header(&info, ctx)) != READSTAT_OK)
                    goto cleanup;
                
                if ((error = recursive_discard(info.header, ctx)) != READSTAT_OK)
                    goto cleanup;
            }
            /* CAR */
            if ((error = read_sexptype_header(&info, ctx)) != READSTAT_OK)
                goto cleanup;
            
            if ((error = recursive_discard(info.header, ctx)) != READSTAT_OK)
                goto cleanup;
            
            /* CDR */
            if ((error = read_sexptype_header(&info, ctx)) != READSTAT_OK)
                goto cleanup;
            
            if ((error = recursive_discard(info.header, ctx)) != READSTAT_OK)
                goto cleanup;
            break;
        case RDATA_SEXPTYPE_EXTERNAL_POINTER:
            read_sexptype_header(&prot, ctx);
            recursive_discard(prot.header, ctx);
            
            read_sexptype_header(&tag, ctx);
            recursive_discard(tag.header, ctx);
            break;
        case RDATA_SEXPTYPE_ENVIRONMENT:
            /* locked */
            if (lseek_st(ctx, sizeof(uint32_t)) == -1) {
                return READSTAT_ERROR_READ;
            }
            
            rdata_sexptype_info_t enclosure, frame, hash_table, attributes;
            read_sexptype_header(&enclosure, ctx);
            recursive_discard(enclosure.header, ctx);
            
            read_sexptype_header(&frame, ctx);
            recursive_discard(frame.header, ctx);
            
            read_sexptype_header(&hash_table, ctx);
            recursive_discard(hash_table.header, ctx);
            
            read_sexptype_header(&attributes, ctx);
            recursive_discard(attributes.header, ctx);
            /*
             if (sexptype_header.attributes) {
             if (lseek(ctx->fd, sizeof(uint32_t), SEEK_CUR) == -1) {
             return READSTAT_ERROR_READ;
             }
             } */
            break;
        case RDATA_PSEUDO_SXP_REF:
        case RDATA_PSEUDO_SXP_NIL:
        case RDATA_PSEUDO_SXP_GLOBAL_ENVIRONMENT:
        case RDATA_PSEUDO_SXP_UNBOUND_VALUE:
        case RDATA_PSEUDO_SXP_MISSING_ARGUMENT:
        case RDATA_PSEUDO_SXP_BASE_NAMESPACE:
        case RDATA_PSEUDO_SXP_EMPTY_ENVIRONMENT:
        case RDATA_PSEUDO_SXP_BASE_ENVIRONMENT:
            break;
        default:
            return READSTAT_ERROR_READ;
    }
cleanup:
    
    return error;
}

