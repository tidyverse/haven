
#define _XOPEN_SOURCE 700 /* for strnlen */

#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <stdint.h>
#include <string.h>
#include <inttypes.h>
#include <sys/types.h>

#include "../readstat.h"
#include "../readstat_bits.h"
#include "../readstat_iconv.h"
#include "../readstat_convert.h"
#include "../readstat_malloc.h"

#include "readstat_dta.h"
#include "readstat_dta_parse_timestamp.h"

#define MAX_VALUE_LABEL_LEN 32000

static readstat_error_t dta_update_progress(dta_ctx_t *ctx);
static readstat_error_t dta_read_descriptors(dta_ctx_t *ctx);
static readstat_error_t dta_read_tag(dta_ctx_t *ctx, const char *tag);
static readstat_error_t dta_read_expansion_fields(dta_ctx_t *ctx);


static readstat_error_t dta_update_progress(dta_ctx_t *ctx) {
    double progress = 0.0;
    if (ctx->row_limit > 0)
        progress = 1.0 * ctx->current_row / ctx->row_limit;
    if (ctx->handle.progress && ctx->handle.progress(progress, ctx->user_ctx) != READSTAT_HANDLER_OK)
        return READSTAT_ERROR_USER_ABORT;
    return READSTAT_OK;
}

static readstat_variable_t *dta_init_variable(dta_ctx_t *ctx, int i, int index_after_skipping,
        readstat_type_t type, size_t max_len, readstat_error_t *out_retval) {
    readstat_variable_t *variable = calloc(1, sizeof(readstat_variable_t));

    if (variable == NULL) {
        if (out_retval) *out_retval = READSTAT_ERROR_MALLOC;
        return NULL;
    }

    variable->type = type;
    variable->index = i;
    variable->index_after_skipping = index_after_skipping;
    variable->storage_width = max_len;

    readstat_convert(variable->name, sizeof(variable->name), 
            &ctx->varlist[ctx->variable_name_len*i],
            strnlen(&ctx->varlist[ctx->variable_name_len*i], ctx->variable_name_len),
            ctx->converter);

    if (ctx->variable_labels[ctx->variable_labels_entry_len*i]) {
        readstat_convert(variable->label, sizeof(variable->label),
                &ctx->variable_labels[ctx->variable_labels_entry_len*i],
                strnlen(&ctx->variable_labels[ctx->variable_labels_entry_len*i], ctx->variable_labels_entry_len),
                ctx->converter);
    }

    if (ctx->fmtlist[ctx->fmtlist_entry_len*i]) {
        readstat_convert(variable->format, sizeof(variable->format),
                &ctx->fmtlist[ctx->fmtlist_entry_len*i],
                strnlen(&ctx->fmtlist[ctx->fmtlist_entry_len*i], ctx->fmtlist_entry_len),
                ctx->converter);
        if (variable->format[0] == '%') {
            if (variable->format[1] == '-') {
                variable->alignment = READSTAT_ALIGNMENT_LEFT;
            } else if (variable->format[1] == '~') {
                variable->alignment = READSTAT_ALIGNMENT_CENTER;
            } else {
                variable->alignment = READSTAT_ALIGNMENT_RIGHT;
            }
        }
        int display_width;
        if (sscanf(variable->format, "%%%ds", &display_width) == 1 ||
                sscanf(variable->format, "%%-%ds", &display_width) == 1) {
            variable->display_width = display_width;
        }
    }

    return variable;
}

static readstat_error_t dta_read_chunk(
        dta_ctx_t *ctx,
        const char *start_tag, 
        void *dst, size_t dst_len,
        const char *end_tag) {
    char *dst_buffer = (char *)dst;
    readstat_io_t *io = ctx->io;
    readstat_error_t retval = READSTAT_OK;

    if ((retval = dta_read_tag(ctx, start_tag)) != READSTAT_OK)
        goto cleanup;

    if (io->read(dst_buffer, dst_len, io->io_ctx) != dst_len) {
        retval = READSTAT_ERROR_READ;
        goto cleanup;
    }

    if ((retval = dta_read_tag(ctx, end_tag)) != READSTAT_OK)
        goto cleanup;

cleanup:

    return retval;
}

static readstat_error_t dta_read_map(dta_ctx_t *ctx) {
    if (!ctx->file_is_xmlish)
        return READSTAT_OK;

    readstat_error_t retval = READSTAT_OK;
    uint64_t map_buffer[14];

    if ((retval = dta_read_chunk(ctx, "<map>", map_buffer, 
                    sizeof(map_buffer), "</map>")) != READSTAT_OK) {
        goto cleanup;
    }

    ctx->data_offset = ctx->bswap ? byteswap8(map_buffer[9]) : map_buffer[9];
    ctx->strls_offset = ctx->bswap ? byteswap8(map_buffer[10]) : map_buffer[10];
    ctx->value_labels_offset = ctx->bswap ? byteswap8(map_buffer[11]) : map_buffer[11];

cleanup:
    return retval;
}

static readstat_error_t dta_read_descriptors(dta_ctx_t *ctx) {
    readstat_error_t retval = READSTAT_OK;
    size_t buffer_len = ctx->nvar * ctx->typlist_entry_len;
    unsigned char *buffer = NULL;
    int i;

    if (ctx->nvar && (buffer = readstat_malloc(buffer_len)) == NULL) {
        retval = READSTAT_ERROR_MALLOC;
        goto cleanup;
    }

    if ((retval = dta_read_chunk(ctx, "<variable_types>", buffer, 
                    buffer_len, "</variable_types>")) != READSTAT_OK)
        goto cleanup;

    if (ctx->typlist_entry_len == 1) {
        for (i=0; i<ctx->nvar; i++) {
            ctx->typlist[i] = buffer[i];
        }
    } else if (ctx->typlist_entry_len == 2) {
        memcpy(ctx->typlist, buffer, buffer_len);
        if (ctx->bswap) {
            for (i=0; i<ctx->nvar; i++) {
                ctx->typlist[i] = byteswap2(ctx->typlist[i]);
            }
        }
    }

    if ((retval = dta_read_chunk(ctx, "<varnames>", ctx->varlist, 
                    ctx->varlist_len, "</varnames>")) != READSTAT_OK)
        goto cleanup;

    if ((retval = dta_read_chunk(ctx, "<sortlist>", ctx->srtlist, 
                    ctx->srtlist_len, "</sortlist>")) != READSTAT_OK)
        goto cleanup;

    if ((retval = dta_read_chunk(ctx, "<formats>", ctx->fmtlist, 
                    ctx->fmtlist_len, "</formats>")) != READSTAT_OK)
        goto cleanup;

    if ((retval = dta_read_chunk(ctx, "<value_label_names>", ctx->lbllist, 
                    ctx->lbllist_len, "</value_label_names>")) != READSTAT_OK)
        goto cleanup;

    if ((retval = dta_read_chunk(ctx, "<variable_labels>", ctx->variable_labels, 
                    ctx->variable_labels_len, "</variable_labels>")) != READSTAT_OK)
        goto cleanup;

cleanup:
    if (buffer)
        free(buffer);

    return retval;
}

static readstat_error_t dta_read_expansion_fields(dta_ctx_t *ctx) {
    readstat_error_t retval = READSTAT_OK;
    readstat_io_t *io = ctx->io;
    char *buffer = NULL;
    char *note_buffer = NULL;

    if (ctx->expansion_len_len == 0)
        return READSTAT_OK;

    if (ctx->file_is_xmlish && !ctx->handle.note) {
        if (io->seek(ctx->data_offset, READSTAT_SEEK_SET, io->io_ctx) == -1) {
            if (ctx->handle.error) {
                snprintf(ctx->error_buf, sizeof(ctx->error_buf),
                        "Failed to seek to data section (offset=%" PRId64 ")",
                        ctx->data_offset);
                ctx->handle.error(ctx->error_buf, ctx->user_ctx);
            }
            return READSTAT_ERROR_SEEK;
        }
        return READSTAT_OK;
    }

    retval = dta_read_tag(ctx, "<characteristics>");
    if (retval != READSTAT_OK)
        goto cleanup;

    while (1) {
        size_t len;
        char data_type;

        if (ctx->file_is_xmlish) {
            char start[4];
            if (io->read(start, sizeof(start), io->io_ctx) != sizeof(start)) {
                retval = READSTAT_ERROR_READ;
                goto cleanup;
            }
            if (memcmp(start, "</ch", sizeof(start)) == 0) {
                retval = dta_read_tag(ctx, "aracteristics>");
                if (retval != READSTAT_OK)
                    goto cleanup;

                break;
            } else if (memcmp(start, "<ch>", sizeof(start)) != 0) {
                retval = READSTAT_ERROR_PARSE;
                goto cleanup;
            }
            data_type = 1;
        } else {
            if (io->read(&data_type, 1, io->io_ctx) != 1) {
                retval = READSTAT_ERROR_READ;
                goto cleanup;
            }
        }

        if (ctx->expansion_len_len == 2) {
            uint16_t len16;
            if (io->read(&len16, sizeof(uint16_t), io->io_ctx) != sizeof(uint16_t)) {
                retval = READSTAT_ERROR_READ;
                goto cleanup;
            }
            len = ctx->bswap ? byteswap2(len16) : len16;
        } else {
            uint32_t len32;
            if (io->read(&len32, sizeof(uint32_t), io->io_ctx) != sizeof(uint32_t)) {
                retval = READSTAT_ERROR_READ;
                goto cleanup;
            }
            len = ctx->bswap ? byteswap4(len32) : len32;
        }

        if (data_type == 0 && len == 0)
            break;
        
        if (len > (1<<20)) {
            retval = READSTAT_ERROR_NOTE_IS_TOO_LONG;
            goto cleanup;
        }

        if (data_type == 1 && ctx->handle.note && len >= 2 * ctx->ch_metadata_len) {
            if ((buffer = readstat_realloc(buffer, len + 1)) == NULL) {
                retval = READSTAT_ERROR_MALLOC;
                goto cleanup;
            }
            buffer[len] = '\0';

            if (io->read(buffer, len, io->io_ctx) != len) {
                retval = READSTAT_ERROR_READ;
                goto cleanup;
            }
            int index = 0;
            /* _dta[note0] holds the number of notes, not a note */
            if (strcmp(&buffer[0], "_dta") == 0 &&
                    sscanf(&buffer[ctx->ch_metadata_len], "note%d", &index) == 1 && index > 0) {
                const char *note = &buffer[2*ctx->ch_metadata_len];
                size_t note_len = strlen(note);
                if ((note_buffer = readstat_realloc(note_buffer, 4*note_len+1)) == NULL) {
                    retval = READSTAT_ERROR_MALLOC;
                    goto cleanup;
                }
                retval = readstat_convert(note_buffer, 4*note_len+1, note, note_len, ctx->converter);
                if (retval != READSTAT_OK)
                    goto cleanup;

                if (ctx->handle.note(index, note_buffer, ctx->user_ctx) != READSTAT_HANDLER_OK) {
                    retval = READSTAT_ERROR_USER_ABORT;
                    goto cleanup;
                }
            }
        } else {
            if (io->seek(len, READSTAT_SEEK_CUR, io->io_ctx) == -1) {
                retval = READSTAT_ERROR_SEEK;
                goto cleanup;
            }
        }

        retval = dta_read_tag(ctx, "</ch>");
        if (retval != READSTAT_OK)
            goto cleanup;
    }

cleanup:
    if (buffer)
        free(buffer);
    if (note_buffer)
        free(note_buffer);

    return retval;
}

static readstat_error_t dta_read_tag(dta_ctx_t *ctx, const char *tag) {
    readstat_error_t retval = READSTAT_OK;
    if (ctx->initialized && !ctx->file_is_xmlish)
        return retval;

    char buffer[256];
    size_t len = strlen(tag);
    if (ctx->io->read(buffer, len, ctx->io->io_ctx) != len) {
        retval = READSTAT_ERROR_READ;
        goto cleanup;
    }
    if (strncmp(buffer, tag, len) != 0) {
        retval = READSTAT_ERROR_PARSE;
        goto cleanup;
    }
cleanup:
    return retval;
}

static int dta_compare_strls(const void *elem1, const void *elem2) {
    const dta_strl_t *key = (const dta_strl_t *)elem1;
    const dta_strl_t *target = *(const dta_strl_t **)elem2;
    if (key->o != target->o)
        return (key->o > target->o) - (key->o < target->o);
    return (key->v > target->v) - (key->v < target->v);
}

/* qsort comparator: both elements are pointers into the strls array */
static int dta_compare_strl_ptrs(const void *elem1, const void *elem2) {
    const dta_strl_t *key = *(const dta_strl_t **)elem1;
    return dta_compare_strls(key, elem2);
}

static dta_strl_t dta_interpret_strl_vo_bytes(dta_ctx_t *ctx, const unsigned char *vo_bytes) {
    dta_strl_t strl = {0};

    if (ctx->strl_v_len == 2) {
        if (ctx->endianness == READSTAT_ENDIAN_BIG) {
            strl.v = (vo_bytes[0] << 8) + vo_bytes[1];
            strl.o = (((uint64_t)vo_bytes[2] << 40)
                    + ((uint64_t)vo_bytes[3] << 32)
                    + ((uint64_t)vo_bytes[4] << 24)
                    + (vo_bytes[5] << 16)
                    + (vo_bytes[6] << 8)
                    + vo_bytes[7]);
        } else {
            strl.v = vo_bytes[0] + (vo_bytes[1] << 8);
            strl.o = (vo_bytes[2] + (vo_bytes[3] << 8)
                    + (vo_bytes[4] << 16)
                    + ((uint64_t)vo_bytes[5] << 24)
                    + ((uint64_t)vo_bytes[6] << 32)
                    + ((uint64_t)vo_bytes[7] << 40));
        }
    } else if (ctx->strl_v_len == 3) {
        /* Format 119 and later: 3-byte v followed by 5-byte o */
        int i;
        if (ctx->endianness == READSTAT_ENDIAN_BIG) {
            strl.v = ((uint32_t)vo_bytes[0] << 16) + ((uint32_t)vo_bytes[1] << 8) + vo_bytes[2];
            for (i=3; i<8; i++) {
                strl.o = (strl.o << 8) + vo_bytes[i];
            }
        } else {
            strl.v = vo_bytes[0] + ((uint32_t)vo_bytes[1] << 8) + ((uint32_t)vo_bytes[2] << 16);
            for (i=7; i>=3; i--) {
                strl.o = (strl.o << 8) + vo_bytes[i];
            }
        }
    } else if (ctx->strl_v_len == 4) {
        uint32_t v, o;

        memcpy(&v, &vo_bytes[0], sizeof(uint32_t));
        memcpy(&o, &vo_bytes[4], sizeof(uint32_t));

        strl.v = ctx->bswap ? byteswap4(v) : v;
        strl.o = ctx->bswap ? byteswap4(o) : o;
    }
    return strl;
}

static readstat_error_t dta_117_read_strl(dta_ctx_t *ctx, dta_strl_t *strl) {
    readstat_error_t retval = READSTAT_OK;
    readstat_io_t *io = ctx->io;
    dta_117_strl_header_t header;

    if (io->read(&header, SIZEOF_DTA_117_STRL_HEADER_T, io->io_ctx) != SIZEOF_DTA_117_STRL_HEADER_T) {
        retval = READSTAT_ERROR_READ;
        goto cleanup;
    }

    strl->v = ctx->bswap ? byteswap4(header.v) : header.v;
    strl->o = ctx->bswap ? byteswap4(header.o) : header.o;
    strl->type = header.type;
    strl->len = ctx->bswap ? byteswap4(header.len) : header.len;

cleanup:
    return retval;
}

static readstat_error_t dta_118_read_strl(dta_ctx_t *ctx, dta_strl_t *strl) {
    readstat_error_t retval = READSTAT_OK;
    readstat_io_t *io = ctx->io;
    dta_118_strl_header_t header;

    if (io->read(&header, SIZEOF_DTA_118_STRL_HEADER_T, io->io_ctx) != SIZEOF_DTA_118_STRL_HEADER_T) {
        retval = READSTAT_ERROR_READ;
        goto cleanup;
    }

    strl->v = ctx->bswap ? byteswap4(header.v) : header.v;
    strl->o = ctx->bswap ? byteswap8(header.o) : header.o;
    strl->type = header.type;
    strl->len = ctx->bswap ? byteswap4(header.len) : header.len;

cleanup:
    return retval;
}

static readstat_error_t dta_read_strl(dta_ctx_t *ctx, dta_strl_t *strl) {
    if (ctx->strl_o_len > 4) {
        return dta_118_read_strl(ctx, strl);
    }
    return dta_117_read_strl(ctx, strl);
}

static readstat_error_t dta_read_strls(dta_ctx_t *ctx) {
    if (!ctx->file_is_xmlish)
        return READSTAT_OK;

    readstat_error_t retval = READSTAT_OK;
    readstat_io_t *io = ctx->io;

    if (io->seek(ctx->strls_offset, READSTAT_SEEK_SET, io->io_ctx) == -1) {
        if (ctx->handle.error) {
            snprintf(ctx->error_buf, sizeof(ctx->error_buf), "Failed to seek to strls section (offset=%" PRId64 ")",
                    ctx->strls_offset);
            ctx->handle.error(ctx->error_buf, ctx->user_ctx);
        }
        retval = READSTAT_ERROR_SEEK;
        goto cleanup;
    }

    retval = dta_read_tag(ctx, "<strls>");
    if (retval != READSTAT_OK)
        goto cleanup;

    ctx->strls_capacity = 100;
    if ((ctx->strls = readstat_malloc(ctx->strls_capacity * sizeof(dta_strl_t *))) == NULL) {
        retval = READSTAT_ERROR_MALLOC;
        goto cleanup;
    }

    while (1) {
        char tag[3];
        if (io->read(tag, sizeof(tag), io->io_ctx) != sizeof(tag)) {
            retval = READSTAT_ERROR_READ;
            goto cleanup;
        }

        if (memcmp(tag, "GSO", sizeof(tag)) == 0) {
            dta_strl_t strl;
            retval = dta_read_strl(ctx, &strl);
            if (retval != READSTAT_OK)
                goto cleanup;

            readstat_off_t pos = io->seek(0, READSTAT_SEEK_CUR, io->io_ctx);
            if (pos == -1) {
                retval = READSTAT_ERROR_SEEK;
                goto cleanup;
            }
            if (strl.len > ctx->file_size - pos) {
                if (ctx->handle.error) {
                    snprintf(ctx->error_buf, sizeof(ctx->error_buf),
                            "strL length (%zu) exceeds remaining file size (%" PRId64 ")",
                            strl.len, (int64_t)(ctx->file_size - pos));
                    ctx->handle.error(ctx->error_buf, ctx->user_ctx);
                }
                retval = READSTAT_ERROR_PARSE;
                goto cleanup;
            }

            if (strl.type != DTA_GSO_TYPE_ASCII && strl.type != DTA_GSO_TYPE_BINARY) {
                /* Unknown record type: skip its contents */
                if (io->seek(strl.len, READSTAT_SEEK_CUR, io->io_ctx) == -1) {
                    retval = READSTAT_ERROR_SEEK;
                    goto cleanup;
                }
                continue;
            }

            if (ctx->strls_count == ctx->strls_capacity) {
                /* Every entry corresponds to a GSO record already read from the
                 * file, so the pointer array is bounded by the file size. Use
                 * plain realloc rather than the capped readstat_realloc, which
                 * fails at around 1.6 million strLs. */
                dta_strl_t **new_strls = realloc(ctx->strls, sizeof(dta_strl_t *) * ctx->strls_capacity * 2);
                if (new_strls == NULL) {
                    retval = READSTAT_ERROR_MALLOC;
                    goto cleanup;
                }
                ctx->strls = new_strls;
                ctx->strls_capacity *= 2;
            }

            /* The record's own length is bounded by the file size above, so
             * a plain malloc is safe; Stata allows strLs up to 2 GB. Binary
             * strLs (type 129) are kept too and delivered up to their first
             * NUL, since the value API only carries C strings. */
            dta_strl_t *strl_ptr = malloc(sizeof(dta_strl_t) + strl.len + 1);
            if (strl_ptr == NULL) {
                retval = READSTAT_ERROR_MALLOC;
                goto cleanup;
            }
            memcpy(strl_ptr, &strl, sizeof(dta_strl_t));

            ctx->strls[ctx->strls_count++] = strl_ptr;

            if (io->read(&strl_ptr->data[0], strl_ptr->len, io->io_ctx) != strl_ptr->len) {
                retval = READSTAT_ERROR_READ;
                goto cleanup;
            }
            /* Type 130 records must already end in NUL, but don't trust it */
            strl_ptr->data[strl_ptr->len] = '\0';
        } else if (memcmp(tag, "</s", sizeof(tag)) == 0) {
            retval = dta_read_tag(ctx, "trls>");
            if (retval != READSTAT_OK)
                goto cleanup;
            /* Stata writes the records in (o,v) order, which the lookup
             * relies on; sort in case another writer did not */
            qsort(ctx->strls, ctx->strls_count, sizeof(dta_strl_t *), &dta_compare_strl_ptrs);
            break;
        } else {
            retval = READSTAT_ERROR_PARSE;
            goto cleanup;
        }
    }

cleanup:
    return retval;
}

static readstat_value_t dta_interpret_int8_bytes(dta_ctx_t *ctx, const void *buf) {
    readstat_value_t value = { .type = READSTAT_TYPE_INT8 };
    int8_t byte = 0;
    memcpy(&byte, buf, sizeof(int8_t));
    if (ctx->machine_is_twos_complement) {
        byte = ones_to_twos_complement1(byte);
    }
    if (byte > ctx->max_int8) {
        if (ctx->supports_tagged_missing && byte > DTA_113_MISSING_INT8) {
            value.tag = 'a' + (byte - DTA_113_MISSING_INT8_A);
            value.is_tagged_missing = 1;
        } else {
            value.is_system_missing = 1;
        }
    }
    value.v.i8_value = byte;

    return value;
}

static readstat_value_t dta_interpret_int16_bytes(dta_ctx_t *ctx, const void *buf) {
    readstat_value_t value = { .type = READSTAT_TYPE_INT16 };
    int16_t num = 0;
    memcpy(&num, buf, sizeof(int16_t));
    if (ctx->bswap) {
        num = byteswap2(num);
    }
    if (ctx->machine_is_twos_complement) {
        num = ones_to_twos_complement2(num);
    }
    if (num > ctx->max_int16) {
        if (ctx->supports_tagged_missing && num > DTA_113_MISSING_INT16) {
            value.tag = 'a' + (num - DTA_113_MISSING_INT16_A);
            value.is_tagged_missing = 1;
        } else {
            value.is_system_missing = 1;
        }
    }
    value.v.i16_value = num;

    return value;
}

static readstat_value_t dta_interpret_int32_bytes(dta_ctx_t *ctx, const void *buf) {
    readstat_value_t value = { .type = READSTAT_TYPE_INT32 };
    int32_t num = 0;
    memcpy(&num, buf, sizeof(int32_t));
    if (ctx->bswap) {
        num = byteswap4(num);
    }
    if (ctx->machine_is_twos_complement) {
        num = ones_to_twos_complement4(num);
    }
    if (num > ctx->max_int32) {
        if (ctx->supports_tagged_missing && num > DTA_113_MISSING_INT32) {
            value.tag = 'a' + (num - DTA_113_MISSING_INT32_A);
            value.is_tagged_missing = 1;
        } else {
            value.is_system_missing = 1;
        }
    }
    value.v.i32_value = num;

    return value;
}

static readstat_value_t dta_interpret_float_bytes(dta_ctx_t *ctx, const void *buf) {
    readstat_value_t value = { .type = READSTAT_TYPE_FLOAT };
    float f_num = NAN;
    int32_t num = 0;
    memcpy(&num, buf, sizeof(int32_t));
    if (ctx->bswap) {
        num = byteswap4(num);
    }
    if (num > ctx->max_float) {
        int32_t tag_index = (num - DTA_113_MISSING_FLOAT_A) >> 11;
        if (ctx->supports_tagged_missing && num >= DTA_113_MISSING_FLOAT_A && tag_index <= 25) {
            value.tag = 'a' + tag_index;
            value.is_tagged_missing = 1;
        } else {
            value.is_system_missing = 1;
        }
    } else {
        memcpy(&f_num, &num, sizeof(int32_t));
        if (isnan(f_num)) /* negative NaN: not a Stata value */
            value.is_system_missing = 1;
    }
    value.v.float_value = f_num;

    return value;
}

static readstat_value_t dta_interpret_double_bytes(dta_ctx_t *ctx, const void *buf) {
    readstat_value_t value = { .type = READSTAT_TYPE_DOUBLE };
    double d_num = NAN;
    int64_t num = 0;
    memcpy(&num, buf, sizeof(int64_t));
    if (ctx->bswap) {
        num = byteswap8(num);
    }
    if (num > ctx->max_double) {
        int64_t tag_index = (num - DTA_113_MISSING_DOUBLE_A) >> 40;
        if (ctx->supports_tagged_missing && num >= DTA_113_MISSING_DOUBLE_A && tag_index <= 25) {
            value.tag = 'a' + tag_index;
            value.is_tagged_missing = 1;
        } else {
            value.is_system_missing = 1;
        }
    } else {
        memcpy(&d_num, &num, sizeof(int64_t));
        if (isnan(d_num)) /* negative NaN: not a Stata value */
            value.is_system_missing = 1;
    }
    value.v.double_value = d_num;

    return value;
}

static readstat_error_t dta_handle_row(const unsigned char *buf, dta_ctx_t *ctx) {
    char  str_buf[4*2045+1]; /* str2045 in a legacy code page, converted to UTF-8 */
    int j;
    readstat_off_t offset = 0;
    readstat_error_t retval = READSTAT_OK;
    for (j=0; j<ctx->nvar; j++) {
        size_t max_len;
        readstat_value_t value = { { 0 } };

        retval = dta_type_info(ctx->typlist[j], ctx, &max_len, &value.type);
        if (retval != READSTAT_OK)
            goto cleanup;

        if (ctx->variables[j]->skip) {
            offset += max_len;
            continue;
        }

        if (offset + max_len > ctx->record_len) {
            retval = READSTAT_ERROR_PARSE;
            goto cleanup;
        }

        if (value.type == READSTAT_TYPE_STRING) {
            if (max_len == 0) {
                retval = READSTAT_ERROR_PARSE;
                goto cleanup;
            }
            size_t str_len = strnlen((const char *)&buf[offset], max_len);
            /* Leading and trailing blanks are significant in Stata */
            retval = readstat_convert_notrim(str_buf, sizeof(str_buf),
                    (const char *)&buf[offset], str_len, ctx->converter);
            if (retval != READSTAT_OK)
                goto cleanup;
            value.v.string_value = str_buf;
        } else if (value.type == READSTAT_TYPE_STRING_REF) {
            dta_strl_t key = dta_interpret_strl_vo_bytes(ctx, &buf[offset]);
            dta_strl_t **found = bsearch(&key, ctx->strls, ctx->strls_count, sizeof(dta_strl_t *), &dta_compare_strls);

            if (found) {
                value.v.string_value = (*found)->data;
            } else if (key.v == 0 && key.o == 0) {
                value.v.string_value = ""; /* the predefined empty strL */
            }
            value.type = READSTAT_TYPE_STRING;
        } else if (value.type == READSTAT_TYPE_INT8) {
            value = dta_interpret_int8_bytes(ctx, &buf[offset]);
        } else if (value.type == READSTAT_TYPE_INT16) {
            value = dta_interpret_int16_bytes(ctx, &buf[offset]);
        } else if (value.type == READSTAT_TYPE_INT32) {
            value = dta_interpret_int32_bytes(ctx, &buf[offset]);
        } else if (value.type == READSTAT_TYPE_FLOAT) {
            value = dta_interpret_float_bytes(ctx, &buf[offset]);
        } else if (value.type == READSTAT_TYPE_DOUBLE) {
            value = dta_interpret_double_bytes(ctx, &buf[offset]);
        }

        if (ctx->handle.value(ctx->current_row, ctx->variables[j], value, ctx->user_ctx) != READSTAT_HANDLER_OK) {
            retval = READSTAT_ERROR_USER_ABORT;
            goto cleanup;
        }

        offset += max_len;
    }
cleanup:
    return retval;
}

static readstat_error_t dta_handle_rows(dta_ctx_t *ctx) {
    readstat_io_t *io = ctx->io;
    unsigned char *buf = NULL;
    int64_t i;
    readstat_error_t retval = READSTAT_OK;

    if (ctx->record_len && (buf = readstat_malloc(ctx->record_len)) == NULL) {
        retval = READSTAT_ERROR_MALLOC;
        goto cleanup;
    }

    if (ctx->row_offset) {
        if (io->seek(ctx->record_len * ctx->row_offset, READSTAT_SEEK_CUR, io->io_ctx) == -1) {
            retval = READSTAT_ERROR_SEEK;
            goto cleanup;
        }
    }

    for (i=0; i<ctx->row_limit; i++) {
        if (io->read(buf, ctx->record_len, io->io_ctx) != ctx->record_len) {
            retval = READSTAT_ERROR_READ;
            goto cleanup;
        }
        if ((retval = dta_handle_row(buf, ctx)) != READSTAT_OK) {
            goto cleanup;
        }
        ctx->current_row++;
        if ((retval = dta_update_progress(ctx)) != READSTAT_OK) {
            goto cleanup;
        }
    }

    if (ctx->row_limit < ctx->nobs - ctx->row_offset) {
        if (io->seek(ctx->record_len * (ctx->nobs - ctx->row_offset - ctx->row_limit), READSTAT_SEEK_CUR, io->io_ctx) == -1)
            retval = READSTAT_ERROR_SEEK;
    }

cleanup:
    if (buf)
        free(buf);

    return retval;
}

static readstat_error_t dta_read_data(dta_ctx_t *ctx) {
    readstat_error_t retval = READSTAT_OK;
    readstat_io_t *io = ctx->io;

    if (!ctx->handle.value) {
        return READSTAT_OK;
    }

    if (io->seek(ctx->data_offset, READSTAT_SEEK_SET, io->io_ctx) == -1) {
        if (ctx->handle.error) {
            snprintf(ctx->error_buf, sizeof(ctx->error_buf), "Failed to seek to data section (offset=%" PRId64 ")",
                    ctx->data_offset);
            ctx->handle.error(ctx->error_buf, ctx->user_ctx);
        }
        retval = READSTAT_ERROR_SEEK;
        goto cleanup;
    }

    if ((retval = dta_read_tag(ctx, "<data>")) != READSTAT_OK)
        goto cleanup;

    if ((retval = dta_update_progress(ctx)) != READSTAT_OK)
        goto cleanup;

    if ((retval = dta_handle_rows(ctx)) != READSTAT_OK)
        goto cleanup;

    if ((retval = dta_read_tag(ctx, "</data>")) != READSTAT_OK)
        goto cleanup;

cleanup:
    return retval;
}

static readstat_error_t dta_read_header(dta_ctx_t *ctx, dta_header_t *header) {
    readstat_error_t retval = READSTAT_OK;
    readstat_io_t *io = ctx->io;
    int bswap = 0;

    if (io->read(header, sizeof(dta_header_t), io->io_ctx) != sizeof(dta_header_t)) {
        retval = READSTAT_ERROR_READ;
        goto cleanup;
    }
    bswap = (header->byteorder == DTA_LOHI) ^ machine_is_little_endian();
    header->nvar = bswap ? byteswap2(header->nvar) : header->nvar;
    header->nobs = bswap ? byteswap4(header->nobs) : header->nobs;

cleanup:
    return retval;
}

static readstat_error_t dta_read_xmlish_header(dta_ctx_t *ctx, dta_header64_t *header) {
    readstat_error_t retval = READSTAT_OK;
    
    if ((retval = dta_read_tag(ctx, "<stata_dta>")) != READSTAT_OK) {
        goto cleanup;
    }
    if ((retval = dta_read_tag(ctx, "<header>")) != READSTAT_OK) {
        goto cleanup;
    }

    char ds_format[3];
    if ((retval = dta_read_chunk(ctx, "<release>", ds_format,
                    sizeof(ds_format), "</release>")) != READSTAT_OK) {
        goto cleanup;
    }

    header->ds_format = 100 * (ds_format[0] - '0') + 10 * (ds_format[1] - '0') + (ds_format[2] - '0');

    char byteorder[3];
    int byteswap = 0;
    if ((retval = dta_read_chunk(ctx, "<byteorder>", byteorder, 
                    sizeof(byteorder), "</byteorder>")) != READSTAT_OK) {
        goto cleanup;
    }
    if (strncmp(byteorder, "MSF", 3) == 0) {
        header->byteorder = DTA_HILO;
    } else if (strncmp(byteorder, "LSF", 3) == 0) {
        header->byteorder = DTA_LOHI;
    } else {
        retval = READSTAT_ERROR_PARSE;
        goto cleanup;
    }
    byteswap = (header->byteorder == DTA_LOHI) ^ machine_is_little_endian();

    if (header->ds_format >= 119) {
        uint32_t nvar;
        if ((retval = dta_read_chunk(ctx, "<K>", &nvar, 
                        sizeof(uint32_t), "</K>")) != READSTAT_OK) {
            goto cleanup;
        }
        header->nvar = byteswap ? byteswap4(nvar) : nvar;
    } else {
        uint16_t nvar;
        if ((retval = dta_read_chunk(ctx, "<K>", &nvar, 
                        sizeof(uint16_t), "</K>")) != READSTAT_OK) {
            goto cleanup;
        }
        header->nvar = byteswap ? byteswap2(nvar) : nvar;
    }

    if (header->ds_format >= 118) {
        uint64_t nobs;
        if ((retval = dta_read_chunk(ctx, "<N>", &nobs, 
                        sizeof(uint64_t), "</N>")) != READSTAT_OK) {
            goto cleanup;
        }
        header->nobs = byteswap ? byteswap8(nobs) : nobs;
    } else {
        uint32_t nobs;
        if ((retval = dta_read_chunk(ctx, "<N>", &nobs, 
                        sizeof(uint32_t), "</N>")) != READSTAT_OK) {
            goto cleanup;
        }
        header->nobs = byteswap ? byteswap4(nobs) : nobs;
    }

cleanup:
    return retval;
}

static readstat_error_t dta_read_label_and_timestamp(dta_ctx_t *ctx) {
    readstat_io_t *io = ctx->io;
    readstat_error_t retval = READSTAT_OK;
    char *data_label_buffer = NULL;
    char *timestamp_buffer = NULL;
    uint16_t label_len = 0;
    unsigned char timestamp_len = 0;
    char last_data_label_char = 0;
    struct tm timestamp = { .tm_isdst = -1 };

    if (ctx->file_is_xmlish) {
        if ((retval = dta_read_tag(ctx, "<label>")) != READSTAT_OK) {
            goto cleanup;
        }
        
        if (ctx->data_label_len_len == 2) {
            if (io->read(&label_len, sizeof(uint16_t), io->io_ctx) != sizeof(uint16_t)) {
                retval = READSTAT_ERROR_READ;
                goto cleanup;
            }
            label_len = ctx->bswap ? byteswap2(label_len) : label_len;
        } else if (ctx->data_label_len_len == 1) {
            unsigned char label_len_char;
            if (io->read(&label_len_char, sizeof(unsigned char), io->io_ctx) != sizeof(unsigned char)) {
                retval = READSTAT_ERROR_READ;
                goto cleanup;
            }
            label_len = label_len_char;
        }
    } else {
        label_len = ctx->data_label_len;
    }

    if ((data_label_buffer = readstat_malloc(label_len+1)) == NULL) {
        retval = READSTAT_ERROR_MALLOC;
        goto cleanup;
    }

    if (io->read(data_label_buffer, label_len, io->io_ctx) != label_len) {
        retval = READSTAT_ERROR_READ;
        goto cleanup;
    }

    if (!ctx->file_is_xmlish) {
        last_data_label_char = data_label_buffer[label_len-1];
        data_label_buffer[label_len] = '\0';
        label_len = strlen(data_label_buffer);
    }

    if ((ctx->data_label = readstat_malloc(4*label_len+1)) == NULL) {
        retval = READSTAT_ERROR_MALLOC;
        goto cleanup;
    }

    if ((retval = readstat_convert(ctx->data_label, 4*label_len+1,
                    data_label_buffer, label_len, ctx->converter)) != READSTAT_OK)
        goto cleanup;

    if (ctx->file_is_xmlish) {
        if ((retval = dta_read_tag(ctx, "</label>")) != READSTAT_OK) {
            goto cleanup;
        }
        
        if ((retval = dta_read_tag(ctx, "<timestamp>")) != READSTAT_OK) {
            goto cleanup;
        }
        
        if (io->read(&timestamp_len, 1, io->io_ctx) != 1) {
            retval = READSTAT_ERROR_READ;
            goto cleanup;
        }
    } else {
        timestamp_len = ctx->timestamp_len;
    }

    if (timestamp_len) {
        if ((timestamp_buffer = readstat_malloc(timestamp_len)) == NULL) {
            retval = READSTAT_ERROR_MALLOC;
            goto cleanup;
        }

        if (io->read(timestamp_buffer, timestamp_len, io->io_ctx) != timestamp_len) {
            retval = READSTAT_ERROR_READ;
            goto cleanup;
        }

        if (!ctx->file_is_xmlish)
            timestamp_len--;

        if (timestamp_buffer[0]) {
            if (timestamp_buffer[timestamp_len-1] == '\0' && last_data_label_char != '\0') {
                /* Stupid hack for miswritten files with off-by-one timestamp, DTA 114 era? */
                memmove(timestamp_buffer+1, timestamp_buffer, timestamp_len-1);
                timestamp_buffer[0] = last_data_label_char;
            }
            if (dta_parse_timestamp(timestamp_buffer, timestamp_len,
                        &timestamp, ctx->handle.error, ctx->user_ctx) == READSTAT_OK) {
                ctx->timestamp = mktime(&timestamp);
            }
        }
    }

    if ((retval = dta_read_tag(ctx, "</timestamp>")) != READSTAT_OK) {
        goto cleanup;
    }

cleanup:
    if (data_label_buffer)
        free(data_label_buffer);
    if (timestamp_buffer)
        free(timestamp_buffer);

    return retval;
}

static readstat_error_t dta_handle_variables(dta_ctx_t *ctx) {
    readstat_error_t retval = READSTAT_OK;
    int i;
    int index_after_skipping = 0;
    char label_set_name[4*129+1];

    for (i=0; i<ctx->nvar; i++) {
        size_t      max_len;
        readstat_type_t type;
        retval = dta_type_info(ctx->typlist[i], ctx, &max_len, &type);
        if (retval != READSTAT_OK)
            goto cleanup;

        if (type == READSTAT_TYPE_STRING)
            max_len++; /* might append NULL */
        if (type == READSTAT_TYPE_STRING_REF) {
            type = READSTAT_TYPE_STRING;
            max_len = 0;
        }

        ctx->variables[i] = dta_init_variable(ctx, i, index_after_skipping, type, max_len, &retval);
        if (ctx->variables[i] == NULL)
            goto cleanup;

        const char *value_labels = NULL;
        const char *lbllist_entry = &ctx->lbllist[ctx->lbllist_entry_len*i];

        if (lbllist_entry[0]) {
            retval = readstat_convert(label_set_name, sizeof(label_set_name),
                    lbllist_entry, strnlen(lbllist_entry, ctx->lbllist_entry_len), ctx->converter);
            if (retval != READSTAT_OK)
                goto cleanup;
            value_labels = label_set_name;
        }

        int cb_retval = READSTAT_HANDLER_OK;
        if (ctx->handle.variable)
            cb_retval = ctx->handle.variable(i, ctx->variables[i], value_labels, ctx->user_ctx);

        if (cb_retval == READSTAT_HANDLER_ABORT) {
            retval = READSTAT_ERROR_USER_ABORT;
            goto cleanup;
        }

        if (cb_retval == READSTAT_HANDLER_SKIP_VARIABLE) {
            ctx->variables[i]->skip = 1;
        } else {
            index_after_skipping++;
        }
    }
cleanup:
    return retval;
}

static readstat_error_t dta_handle_value_labels(dta_ctx_t *ctx) {
    readstat_io_t *io = ctx->io;
    readstat_error_t retval = READSTAT_OK;
    char *table_buffer = NULL;
    char *utf8_buffer = NULL;

    if (io->seek(ctx->value_labels_offset, READSTAT_SEEK_SET, io->io_ctx) == -1) {
        if (ctx->handle.error) {
            snprintf(ctx->error_buf, sizeof(ctx->error_buf), "Failed to seek to value labels section (offset=%" PRId64 ")",
                    ctx->value_labels_offset);
            ctx->handle.error(ctx->error_buf, ctx->user_ctx);
        }
        retval = READSTAT_ERROR_SEEK;
        goto cleanup;
    }
    if ((retval = dta_read_tag(ctx, "<value_labels>")) != READSTAT_OK) {
        goto cleanup;
    }

    if (!ctx->handle.value_label) {
        return READSTAT_OK;
    }

    while (1) {
        size_t len = 0;
        char raw_labname[129+1];
        char labname[4*129+1];
        uint32_t i = 0, n = 0;

        if (ctx->value_label_table_len_len == 2) {
            /* Formats 105 and earlier: n (int16), labname, 1 byte of padding,
             * n int16 codes, then n 8-byte labels */
            uint16_t n16;
            if (io->read(&n16, sizeof(uint16_t), io->io_ctx) < sizeof(uint16_t))
                break;

            n = ctx->bswap ? byteswap2(n16) : n16;
            len = 10 * n;
        } else {
            if (dta_read_tag(ctx, "<lbl>") != READSTAT_OK) {
                break;
            }

            int32_t table_header_len;
            if (io->read(&table_header_len, sizeof(int32_t), io->io_ctx) < sizeof(int32_t))
                break;

            len = table_header_len;

            if (ctx->bswap)
                len = byteswap4(table_header_len);
        }

        if (io->read(raw_labname, ctx->value_label_table_labname_len, io->io_ctx) < ctx->value_label_table_labname_len)
            break;
        retval = readstat_convert(labname, sizeof(labname), raw_labname,
                strnlen(raw_labname, ctx->value_label_table_labname_len), ctx->converter);
        if (retval != READSTAT_OK)
            goto cleanup;

        if (io->seek(ctx->value_label_table_padding_len, READSTAT_SEEK_CUR, io->io_ctx) == -1)
            break;

        if (len == 0)
            continue;

        if ((table_buffer = readstat_realloc(table_buffer, len)) == NULL) {
            retval = READSTAT_ERROR_MALLOC;
            goto cleanup;
        }

        if (io->read(table_buffer, len, io->io_ctx) < len) {
            break;
        }

        if (ctx->value_label_table_len_len == 2) {
            for (i=0; i<n; i++) {
                int16_t code = 0;
                char label_buf[4*8+1];
                const char *label_bytes = &table_buffer[2*n + 8*i];

                memcpy(&code, &table_buffer[2*i], sizeof(int16_t));
                if (ctx->bswap)
                    code = byteswap2(code);

                readstat_value_t value = { .v = { .i32_value = code }, .type = READSTAT_TYPE_INT32 };

                retval = readstat_convert(label_buf, sizeof(label_buf),
                        label_bytes, strnlen(label_bytes, 8), ctx->converter);
                if (retval != READSTAT_OK)
                    goto cleanup;

                if (label_buf[0] && ctx->handle.value_label(labname, value, label_buf, ctx->user_ctx) != READSTAT_HANDLER_OK) {
                    retval = READSTAT_ERROR_USER_ABORT;
                    goto cleanup;
                }
            }
        } else if (len >= 8) {
            if ((retval = dta_read_tag(ctx, "</lbl>")) != READSTAT_OK) {
                goto cleanup;
            }

            n = *(uint32_t *)table_buffer;

            uint32_t txtlen = *((uint32_t *)table_buffer+1);
            if (ctx->bswap) {
                n = byteswap4(n);
                txtlen = byteswap4(txtlen);
            }

            if (txtlen > len - 8 || n > (len - 8 - txtlen) / 8) {
                break;
            }

            uint32_t *off = (uint32_t *)table_buffer+2;
            uint32_t *val = (uint32_t *)table_buffer+2+n;
            char *txt = &table_buffer[8LL*n+8];
            size_t utf8_buffer_len = 4*txtlen+1;
            if (txtlen > MAX_VALUE_LABEL_LEN+1)
                utf8_buffer_len = 4*MAX_VALUE_LABEL_LEN+1;

            utf8_buffer = realloc(utf8_buffer, utf8_buffer_len);
            /* Much bigger than we need but whatever */
            if (utf8_buffer == NULL) {
                retval = READSTAT_ERROR_MALLOC;
                goto cleanup;
            }

            if (ctx->bswap) {
                for (i=0; i<n; i++) {
                    off[i] = byteswap4(off[i]);
                }
            }

            for (i=0; i<n; i++) {
                if (off[i] >= txtlen) {
                    retval = READSTAT_ERROR_PARSE;
                    goto cleanup;
                }

                readstat_value_t value = dta_interpret_int32_bytes(ctx, &val[i]);
                size_t max_label_len = txtlen - off[i];
                if (max_label_len > MAX_VALUE_LABEL_LEN)
                    max_label_len = MAX_VALUE_LABEL_LEN;
                size_t label_len = strnlen(&txt[off[i]], max_label_len);

                retval = readstat_convert(utf8_buffer, utf8_buffer_len, &txt[off[i]], label_len, ctx->converter);
                if (retval != READSTAT_OK)
                    goto cleanup;

                if (ctx->handle.value_label(labname, value, utf8_buffer, ctx->user_ctx) != READSTAT_HANDLER_OK) {
                    retval = READSTAT_ERROR_USER_ABORT;
                    goto cleanup;
                }
            }
        }
    }

cleanup:
    if (table_buffer)
        free(table_buffer);
    if (utf8_buffer)
        free(utf8_buffer);

    return retval;
}

readstat_error_t readstat_parse_dta(readstat_parser_t *parser, const char *path, void *user_ctx) {
    readstat_error_t retval = READSTAT_OK;
    readstat_io_t *io = parser->io;
    int i;
    dta_ctx_t    *ctx;
    size_t file_size = 0;

    ctx = dta_ctx_alloc(io);

    if (io->open(path, io->io_ctx) == -1) {
        retval = READSTAT_ERROR_OPEN;
        goto cleanup;
    }

    char magic[4];
    if (io->read(magic, 4, io->io_ctx) != 4) {
        retval = READSTAT_ERROR_READ;
        goto cleanup;
    }

    file_size = io->seek(0, READSTAT_SEEK_END, io->io_ctx);
    if (file_size == -1) {
        if (ctx->handle.error) {
            snprintf(ctx->error_buf, sizeof(ctx->error_buf), "Failed to seek to end of file");
            ctx->handle.error(ctx->error_buf, ctx->user_ctx);
        }
        retval = READSTAT_ERROR_SEEK;
        goto cleanup;
    }

    if (io->seek(0, READSTAT_SEEK_SET, io->io_ctx) == -1) {
        if (ctx->handle.error) {
            snprintf(ctx->error_buf, sizeof(ctx->error_buf), "Failed to seek to start of file");
            ctx->handle.error(ctx->error_buf, ctx->user_ctx);
        }
        retval = READSTAT_ERROR_SEEK;
        goto cleanup;
    }

    ctx->file_size = file_size;

    if (strncmp(magic, "<sta", 4) == 0) {
        dta_header64_t header;
        if ((retval = dta_read_xmlish_header(ctx, &header)) != READSTAT_OK) {
            goto cleanup;
        }
        retval = dta_ctx_init(ctx, header.nvar, header.nobs, header.byteorder, header.ds_format,
                parser->input_encoding, parser->output_encoding);
    } else {
        dta_header_t header;
        if ((retval = dta_read_header(ctx, &header)) != READSTAT_OK) {
            goto cleanup;
        }
        retval = dta_ctx_init(ctx, header.nvar, header.nobs, header.byteorder, header.ds_format,
                parser->input_encoding, parser->output_encoding);
    }
    if (retval != READSTAT_OK) {
        goto cleanup;
    }

    ctx->user_ctx = user_ctx;
    ctx->handle = parser->handlers;
    if (parser->row_offset > 0)
        ctx->row_offset = parser->row_offset;
    int64_t nobs_after_skipping = ctx->nobs - ctx->row_offset;
    if (nobs_after_skipping < 0) {
        nobs_after_skipping = 0;
        ctx->row_offset = ctx->nobs;
    }
    ctx->row_limit = nobs_after_skipping;
    if (parser->row_limit > 0 && parser->row_limit < nobs_after_skipping)
        ctx->row_limit = parser->row_limit;

    retval = dta_update_progress(ctx);
    if (retval != READSTAT_OK)
        goto cleanup;
    
    if ((retval = dta_read_label_and_timestamp(ctx)) != READSTAT_OK)
        goto cleanup;

    if ((retval = dta_read_tag(ctx, "</header>")) != READSTAT_OK) {
        goto cleanup;
    }

    if (ctx->handle.metadata) {
        readstat_metadata_t metadata = {
            .row_count = ctx->row_limit,
            .var_count = ctx->nvar,
            .file_label = ctx->data_label,
            .creation_time = ctx->timestamp,
            .modified_time = ctx->timestamp,
            .file_format_version = ctx->ds_format,
            .is64bit = ctx->ds_format >= 118,
            .endianness = ctx->endianness
        };
        if (ctx->handle.metadata(&metadata, user_ctx) != READSTAT_HANDLER_OK) {
            retval = READSTAT_ERROR_USER_ABORT;
            goto cleanup;
        }
    }

    if ((retval = dta_read_map(ctx)) != READSTAT_OK) {
        retval = READSTAT_ERROR_READ;
        goto cleanup;
    }

    if ((retval = dta_read_descriptors(ctx)) != READSTAT_OK) {
        goto cleanup;
    }

    for (i=0; i<ctx->nvar; i++) {
        size_t      max_len;
        if ((retval = dta_type_info(ctx->typlist[i], ctx, &max_len, NULL)) != READSTAT_OK)
            goto cleanup;

        ctx->record_len += max_len;
    }

    if (ctx->nvar > 0 && ctx->record_len == 0) {
        retval = READSTAT_ERROR_PARSE;
        goto cleanup;
    }

    if ((retval = dta_handle_variables(ctx)) != READSTAT_OK)
        goto cleanup;

    if ((retval = dta_read_expansion_fields(ctx)) != READSTAT_OK)
        goto cleanup;

    if (!ctx->file_is_xmlish) {
        ctx->data_offset = io->seek(0, READSTAT_SEEK_CUR, io->io_ctx);
        if (ctx->data_offset == -1) {
            retval = READSTAT_ERROR_SEEK;
            goto cleanup;
        }
        ctx->value_labels_offset = ctx->data_offset + ctx->record_len * ctx->nobs;
    }

    if ((retval = dta_read_strls(ctx)) != READSTAT_OK)
        goto cleanup;

    if ((retval = dta_read_data(ctx)) != READSTAT_OK)
        goto cleanup;

    if ((retval = dta_handle_value_labels(ctx)) != READSTAT_OK)
        goto cleanup;

cleanup:
    io->close(io->io_ctx);
    if (ctx)
        dta_ctx_free(ctx);

    return retval;
}
