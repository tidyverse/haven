
#include <stdlib.h>
#include <math.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include "readstat_dta.h"
#include "readstat_convert.h"

static readstat_error_t dta_update_progress(dta_ctx_t *ctx);
static readstat_error_t dta_read_descriptors(dta_ctx_t *ctx);
static readstat_error_t dta_read_tag(dta_ctx_t *ctx, const char *tag);
static readstat_error_t dta_read_long_string(dta_ctx_t *ctx, int v, int o, char **long_string_out);
static readstat_error_t dta_skip_expansion_fields(dta_ctx_t *ctx);


static readstat_error_t dta_update_progress(dta_ctx_t *ctx) {
    readstat_io_t *io = ctx->io;
    return io->update(ctx->file_size, ctx->progress_handler, ctx->user_ctx, io->io_ctx);
}

static readstat_variable_t *dta_init_variable(dta_ctx_t *ctx, int i, readstat_types_t type, size_t max_len) {
    readstat_variable_t *variable = calloc(1, sizeof(readstat_variable_t));

    variable->type = type;
    variable->index = i;
    variable->storage_width = max_len;

    readstat_convert(variable->name, sizeof(variable->name), 
            &ctx->varlist[ctx->variable_name_len*i],
            ctx->variable_name_len, ctx->converter);

    if (ctx->variable_labels[ctx->variable_labels_entry_len*i]) {
        readstat_convert(variable->label, sizeof(variable->label),
                &ctx->variable_labels[ctx->variable_labels_entry_len*i],
                ctx->variable_labels_entry_len, ctx->converter);
    }

    if (ctx->fmtlist[ctx->fmtlist_entry_len*i]) {
        readstat_convert(variable->format, sizeof(variable->format),
                &ctx->fmtlist[ctx->fmtlist_entry_len*i],
                ctx->fmtlist_entry_len, ctx->converter);
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

    ctx->data_offset = ctx->machine_needs_byte_swap ? byteswap8(map_buffer[9]) : map_buffer[9];
    ctx->strls_offset = ctx->machine_needs_byte_swap ? byteswap8(map_buffer[10]) : map_buffer[10];
    ctx->value_labels_offset = ctx->machine_needs_byte_swap ? byteswap8(map_buffer[11]) : map_buffer[11];

cleanup:
    return retval;
}

static readstat_error_t dta_read_descriptors(dta_ctx_t *ctx) {
    readstat_error_t retval = READSTAT_OK;
    size_t buffer_len = ctx->nvar * ctx->typlist_entry_len;
    unsigned char *buffer = NULL;
    int i;

    buffer = malloc(buffer_len);

    if ((retval = dta_read_chunk(ctx, "<variable_types>", buffer, 
                    buffer_len, "</variable_types>")) != READSTAT_OK)
        goto cleanup;

    if (ctx->typlist_entry_len == 1) {
        for (i=0; i<ctx->nvar; i++) {
            ctx->typlist[i] = buffer[i];
        }
    } else if (ctx->typlist_entry_len == 2) {
        memcpy(ctx->typlist, buffer, buffer_len);
        if (ctx->machine_needs_byte_swap) {
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

static readstat_error_t dta_skip_expansion_fields(dta_ctx_t *ctx) {
    readstat_io_t *io = ctx->io;
    if (ctx->file_is_xmlish) {
        if (io->seek(ctx->data_offset, READSTAT_SEEK_SET, io->io_ctx) == -1)
            return READSTAT_ERROR_SEEK;

        return READSTAT_OK;
    }
    if (ctx->expansion_len_len == 0)
        return READSTAT_OK;
    
    while (1) {
        size_t len;
        char data_type;
        if (ctx->expansion_len_len == 2) {
            dta_short_expansion_field_t  expansion_field;
            if (io->read(&expansion_field, sizeof(expansion_field), io->io_ctx) != sizeof(expansion_field))
                return READSTAT_ERROR_READ;

            if (ctx->machine_needs_byte_swap) {
                len = byteswap2(expansion_field.len);
            } else {
                len = expansion_field.len;
            }
            
            data_type = expansion_field.data_type;
        } else {
            dta_expansion_field_t  expansion_field;
            if (io->read(&expansion_field, sizeof(expansion_field), io->io_ctx) != sizeof(expansion_field))
                return READSTAT_ERROR_READ;
            
            if (ctx->machine_needs_byte_swap) {
                len = byteswap4(expansion_field.len);
            } else {
                len = expansion_field.len;
            }
            
            data_type = expansion_field.data_type;
        }

        if (data_type == 0 && len == 0)
            return READSTAT_OK;
        
        if (data_type != 1)
            return READSTAT_ERROR_PARSE;

        if (io->seek(len, READSTAT_SEEK_CUR, io->io_ctx) == -1)
            return READSTAT_ERROR_SEEK;
    }

    return READSTAT_ERROR_PARSE;
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

static readstat_error_t dta_read_long_string(dta_ctx_t *ctx, int v, int o, char **long_string_out) {
    readstat_error_t retval = READSTAT_OK;
    readstat_io_t *io = ctx->io;
    if (io->seek(ctx->strls_offset, READSTAT_SEEK_SET, io->io_ctx) != ctx->strls_offset) {
        retval = READSTAT_ERROR_SEEK;
        goto cleanup;
    }

    retval = dta_read_tag(ctx, "<strls>");
    if (retval != READSTAT_OK)
        goto cleanup;

    dta_gso_header_t header;

    while (1) {
        if (io->read(&header, sizeof(dta_gso_header_t), io->io_ctx) != sizeof(dta_gso_header_t)) {
            retval = READSTAT_ERROR_READ;
            goto cleanup;
        }

        if (strncmp(header.gso, "GSO", sizeof("GSO")-1) != 0) {
            retval = READSTAT_ERROR_PARSE;
            goto cleanup;
        }
        
        if (header.len <= 0) {
            retval = READSTAT_ERROR_PARSE;
            goto cleanup;
        }

        if (header.v == v && header.o == o) {
            if (header.t == DTA_GSO_TYPE_BINARY) {
                *long_string_out = NULL;
            } else if (header.t == DTA_GSO_TYPE_ASCII) {
                char *string_buf = malloc(header.len);
                if (io->read(string_buf, header.len, io->io_ctx) != header.len) {
                    free(string_buf);
                    retval = READSTAT_ERROR_READ;
                    goto cleanup;
                }
                if (string_buf[header.len-1] != '\0') {
                    free(string_buf);
                    retval = READSTAT_ERROR_PARSE;
                    goto cleanup;
                }
                *long_string_out = string_buf;
            } else {
                retval = READSTAT_ERROR_PARSE;
                goto cleanup;
            }
            break;
        } else {
            if (io->seek(header.len, READSTAT_SEEK_CUR, io->io_ctx) == -1) {
                retval = READSTAT_ERROR_SEEK;
                goto cleanup;
            }
        }
    }

cleanup:
    return retval;
}

static readstat_error_t dta_read_xmlish_preamble(dta_ctx_t *ctx, dta_header_t *header) {
    readstat_error_t retval = READSTAT_OK;
    readstat_io_t *io = ctx->io;
    
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

    if ((retval = dta_read_chunk(ctx, "<K>", &header->nvar, 
                    sizeof(int16_t), "</K>")) != READSTAT_OK) {
        goto cleanup;
    }

    if ((retval = dta_read_tag(ctx, "<N>")) != READSTAT_OK) {
        goto cleanup;
    }
    if (io->read(&header->nobs, sizeof(int32_t), io->io_ctx) != sizeof(int32_t)) {
        retval = READSTAT_ERROR_READ;
        goto cleanup;
    }
    if (header->ds_format >= 118) {
        /* Only support files < 4 billion rows for now */
        if (header->byteorder == DTA_HILO) {
            if (io->read(&header->nobs, sizeof(int32_t), io->io_ctx) != sizeof(int32_t)) {
                retval = READSTAT_ERROR_READ;
                goto cleanup;
            }
        } else {
            if (io->seek(4, READSTAT_SEEK_CUR, io->io_ctx) == -1) {
                retval = READSTAT_ERROR_SEEK;
                goto cleanup;
            }
        }
    }
    if ((retval = dta_read_tag(ctx, "</N>")) != READSTAT_OK) {
        goto cleanup;
    }

cleanup:
    return retval;
}

static readstat_error_t dta_skip_label_and_timestamp(dta_ctx_t *ctx) {
    readstat_io_t *io = ctx->io;
    readstat_error_t retval = READSTAT_OK;
    if (ctx->file_is_xmlish) {
        uint16_t label_len = 0;
        unsigned char timestamp_len;

        if ((retval = dta_read_tag(ctx, "<label>")) != READSTAT_OK) {
            goto cleanup;
        }
        
        if (ctx->data_label_len_len == 2) {
            if (io->read(&label_len, sizeof(uint16_t), io->io_ctx) != sizeof(uint16_t)) {
                retval = READSTAT_ERROR_READ;
                goto cleanup;
            }
            label_len = ctx->machine_needs_byte_swap ? byteswap2(label_len) : label_len;
        } else if (ctx->data_label_len_len == 1) {
            unsigned char label_len_char;
            if (io->read(&label_len_char, sizeof(unsigned char), io->io_ctx) != sizeof(unsigned char)) {
                retval = READSTAT_ERROR_READ;
                goto cleanup;
            }
            label_len = label_len_char;
        }
        
        if (io->seek(label_len, READSTAT_SEEK_CUR, io->io_ctx) == -1) {
            retval = READSTAT_ERROR_SEEK;
            goto cleanup;
        }
        
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
        
        if (io->seek(timestamp_len, READSTAT_SEEK_CUR, io->io_ctx) == -1) {
            retval = READSTAT_ERROR_SEEK;
            goto cleanup;
        }

        if ((retval = dta_read_tag(ctx, "</timestamp>")) != READSTAT_OK) {
            goto cleanup;
        }
    } else {
        if (io->seek(ctx->data_label_len, READSTAT_SEEK_CUR, io->io_ctx) == -1) {
            retval = READSTAT_ERROR_SEEK;
            goto cleanup;
        }
        
        if (ctx->time_stamp_len) {
            if (io->seek(ctx->time_stamp_len, READSTAT_SEEK_CUR, io->io_ctx) == -1) {
                retval = READSTAT_ERROR_SEEK;
                goto cleanup;
            }
        }
    }

cleanup:
    return retval;
}

static readstat_error_t dta_handle_variables(dta_ctx_t *ctx) {
    if (!ctx->variable_handler)
        return READSTAT_OK;

    readstat_error_t retval = READSTAT_OK;
    int i;

    for (i=0; i<ctx->nvar; i++) {
        size_t      max_len;
        readstat_types_t type = dta_type_info(ctx->typlist[i], &max_len, ctx);

        if (type == READSTAT_TYPE_STRING)
            max_len++; /* might append NULL */

        readstat_variable_t *variable = dta_init_variable(ctx, i, type, max_len);

        const char *value_labels = NULL;

        if (ctx->lbllist[ctx->lbllist_entry_len*i])
            value_labels = &ctx->lbllist[ctx->lbllist_entry_len*i];

        int cb_retval = ctx->variable_handler(i, variable, value_labels, ctx->user_ctx);

        free(variable);

        if (cb_retval) {
            retval = READSTAT_ERROR_USER_ABORT;
            goto cleanup;
        }
    }
cleanup:
    return retval;
}

static readstat_error_t dta_handle_rows(dta_ctx_t *ctx) {
    readstat_io_t *io = ctx->io;
    char *buf = NULL;
    char  str_buf[2048];
    int i;
    readstat_error_t retval = READSTAT_OK;
    char *long_string = NULL;

    if (!ctx->value_handler) {
        if (io->seek(ctx->record_len * ctx->nobs, READSTAT_SEEK_CUR, io->io_ctx) == -1)
            retval = READSTAT_ERROR_SEEK;

        return retval;
    }

    if ((buf = malloc(ctx->record_len)) == NULL) {
        retval = READSTAT_ERROR_MALLOC;
        goto cleanup;
    }

    for (i=0; i<ctx->row_limit; i++) {
        if (io->read(buf, ctx->record_len, io->io_ctx) != ctx->record_len) {
            retval = READSTAT_ERROR_READ;
            goto cleanup;
        }
        int j;
        off_t offset = 0;
        for (j=0; j<ctx->nvar; j++) {
            size_t max_len;
            readstat_value_t value;
            memset(&value, 0, sizeof(readstat_value_t));

            value.type = dta_type_info(ctx->typlist[j], &max_len, ctx);

            if (value.type == READSTAT_TYPE_STRING) {
                readstat_convert(str_buf, sizeof(str_buf), &buf[offset], max_len, ctx->converter);
                value.v.string_value = str_buf;
            } else if (value.type == READSTAT_TYPE_LONG_STRING) {
                uint32_t v, o;
                v = *((uint32_t *)&buf[offset]);
                o = *((uint32_t *)&buf[offset+4]);
                if (ctx->machine_needs_byte_swap) {
                    v = byteswap4(v);
                    o = byteswap4(o);
                }
                if (v > 0 && o > 0) {
                    off_t cur_pos = io->seek(0, READSTAT_SEEK_CUR, io->io_ctx);
                    if (cur_pos == -1) {
                        retval = READSTAT_ERROR_SEEK;
                        goto cleanup;
                    }
                    retval = dta_read_long_string(ctx, v, o, &long_string);
                    if (retval != READSTAT_OK) {
                        goto cleanup;
                    }
                    value.v.string_value = long_string;
                    if (io->seek(cur_pos, READSTAT_SEEK_SET, io->io_ctx) == -1) {
                        retval = READSTAT_ERROR_SEEK;
                        goto cleanup;
                    }
                }
            } else if (value.type == READSTAT_TYPE_CHAR) {
                char byte = buf[offset];
                if (ctx->machine_is_twos_complement) {
                    byte = ones_to_twos_complement1(byte);
                }
                if (byte > DTA_MAX_CHAR) {
                    value.is_system_missing = 1;
                    if (byte > DTA_MISSING_CHAR && ctx->supports_tagged_missing) {
                        value.tag = 'a' + (byte - DTA_MISSING_CHAR_A);
                    }
                }
                value.v.char_value = byte;
            } else if (value.type == READSTAT_TYPE_INT16) {
                int16_t num = *((int16_t *)&buf[offset]);
                if (ctx->machine_needs_byte_swap) {
                    num = byteswap2(num);
                }
                if (ctx->machine_is_twos_complement) {
                    num = ones_to_twos_complement2(num);
                }
                if (num > DTA_MAX_INT16) {
                    value.is_system_missing = 1;
                    if (num > DTA_MISSING_INT16 && ctx->supports_tagged_missing) {
                        value.tag = 'a' + (num - DTA_MISSING_INT16_A);
                    }
                }
                value.v.i16_value = num;
            } else if (value.type == READSTAT_TYPE_INT32) {
                int32_t num = *((int32_t *)&buf[offset]);
                if (ctx->machine_needs_byte_swap) {
                    num = byteswap4(num);
                }
                if (ctx->machine_is_twos_complement) {
                    num = ones_to_twos_complement4(num);
                }
                if (num > DTA_MAX_INT32) {
                    value.is_system_missing = 1;
                    if (num > DTA_MISSING_INT32 && ctx->supports_tagged_missing) {
                        value.tag = 'a' + (num - DTA_MISSING_INT32_A);
                    }
                }
                value.v.i32_value = num;
            } else if (value.type == READSTAT_TYPE_FLOAT) {
                int32_t num = *((int32_t *)&buf[offset]);
                float f_num = NAN;
                if (ctx->machine_needs_byte_swap) {
                    num = byteswap4(num);
                }
                if (num > DTA_MAX_FLOAT) {
                    value.is_system_missing = 1;
                    if (num > DTA_MISSING_FLOAT && ctx->supports_tagged_missing) {
                        value.tag = 'a' + ((num - DTA_MISSING_FLOAT_A) >> 11);
                    }
                } else {
                    memcpy(&f_num, &num, sizeof(int32_t));
                }
                value.v.float_value = f_num;
            } else if (value.type == READSTAT_TYPE_DOUBLE) {
                int64_t num = *((int64_t *)&buf[offset]);
                double d_num = NAN;
                if (ctx->machine_needs_byte_swap) {
                    num = byteswap8(num);
                }
                if (num > DTA_MAX_DOUBLE) {
                    value.is_system_missing = 1;
                    if (num > DTA_MISSING_DOUBLE && ctx->supports_tagged_missing) {
                        value.tag = 'a' + ((num - DTA_MISSING_DOUBLE_A) >> 40);
                    }
                } else {
                    memcpy(&d_num, &num, sizeof(int64_t));
                }
                value.v.double_value = d_num;
            }

            if (ctx->value_handler(i, j, value, ctx->user_ctx)) {
                retval = READSTAT_ERROR_USER_ABORT;
                goto cleanup;
            }

            if (long_string) {
                free(long_string);
                long_string = NULL;
            }

            offset += max_len;
        }
        if ((retval = dta_update_progress(ctx)) != READSTAT_OK) {
            goto cleanup;
        }
    }

    if (ctx->row_limit < ctx->nobs) {
        if (io->seek(ctx->record_len * (ctx->nobs - ctx->row_limit), READSTAT_SEEK_CUR, io->io_ctx) == -1)
            retval = READSTAT_ERROR_SEEK;
    }

cleanup:
    if (buf)
        free(buf);
    if (long_string)
        free(long_string);

    return retval;
}

static readstat_error_t dta_handle_value_labels(dta_ctx_t *ctx) {
    readstat_io_t *io = ctx->io;
    readstat_error_t retval = READSTAT_OK;
    if (!ctx->value_label_handler) {
        if (io->seek(0, READSTAT_SEEK_END, io->io_ctx) == -1)
            return READSTAT_ERROR_SEEK;

        return READSTAT_OK;
    }

    char *table_buffer = NULL;

    while (1) {
        size_t len = 0;
        char labname[129];
        int32_t i = 0, n = 0;

        if (ctx->value_label_table_len_len == 2) {
            int16_t table_header_len;
            if (io->read(&table_header_len, sizeof(int16_t), io->io_ctx) < sizeof(int16_t))
                break;

            n = table_header_len;

            if (ctx->machine_needs_byte_swap)
                n = byteswap2(table_header_len);

            len = 8 * n;
        } else {
            if (dta_read_tag(ctx, "<lbl>") != READSTAT_OK) {
                break;
            }

            int32_t table_header_len;
            if (io->read(&table_header_len, sizeof(int32_t), io->io_ctx) < sizeof(int32_t))
                break;

            len = table_header_len;

            if (ctx->machine_needs_byte_swap)
                len = byteswap4(table_header_len);
        }

        if (io->read(labname, ctx->value_label_table_labname_len, io->io_ctx) < ctx->value_label_table_labname_len)
            break;

        if (io->seek(ctx->value_label_table_padding_len, READSTAT_SEEK_CUR, io->io_ctx) == -1)
            break;

        if ((table_buffer = realloc(table_buffer, len)) == NULL) {
            retval = READSTAT_ERROR_MALLOC;
            goto cleanup;
        }

        if (io->read(table_buffer, len, io->io_ctx) < len) {
            break;
        }

        if (ctx->value_label_table_len_len == 2) {
            for (i=0; i<n; i++) {
                readstat_value_t value = { .v = { .i32_value = i }, .type = READSTAT_TYPE_INT32 };
                if (ctx->value_label_handler(labname, value, table_buffer + 8 * i, ctx->user_ctx)) {
                    retval = READSTAT_ERROR_USER_ABORT;
                    goto cleanup;
                }
            }
        } else {
            if ((retval = dta_read_tag(ctx, "</lbl>")) != READSTAT_OK) {
                goto cleanup;
            }

            n = *(int32_t *)table_buffer;

            int32_t txtlen = *((int32_t *)table_buffer+1);
            if (ctx->machine_needs_byte_swap) {
                n = byteswap4(n);
                txtlen = byteswap4(txtlen);
            }

            if (8*n + 8 > len || 8*n + 8 + txtlen > len || n < 0 || txtlen < 0) {
                break;
            }

            int32_t *off = (int32_t *)table_buffer+2;
            int32_t *val = (int32_t *)table_buffer+2+n;
            char *txt = &table_buffer[8*n+8];

            if (ctx->machine_needs_byte_swap) {
                for (i=0; i<n; i++) {
                    off[i] = byteswap4(off[i]);
                    val[i] = byteswap4(val[i]);
                }
            }
            if (ctx->machine_is_twos_complement) {
                for (i=0; i<n; i++) {
                    val[i] = ones_to_twos_complement4(val[i]);
                }
            }

            for (i=0; i<n; i++) {
                if (off[i] < txtlen) {
                    readstat_value_t value = { .v = { .i32_value = val[i] }, .type = READSTAT_TYPE_INT32 };
                    if (val[i] > DTA_MAX_INT32) {
                        value.is_system_missing = 1;
                        if (val[i] > DTA_MISSING_INT32 && ctx->supports_tagged_missing) {
                            value.tag = 'a' + (val[i] - DTA_MISSING_INT32_A);
                        }
                    }
                    if (ctx->value_label_handler(labname, value, &txt[off[i]], ctx->user_ctx)) {
                        retval = READSTAT_ERROR_USER_ABORT;
                        goto cleanup;
                    }
                }
            }
        }

    }

cleanup:
    if (table_buffer)
        free(table_buffer);

    return retval;
}

readstat_error_t readstat_parse_dta(readstat_parser_t *parser, const char *path, void *user_ctx) {
    readstat_error_t retval = READSTAT_OK;
    readstat_io_t *io = parser->io;
    int i;
    dta_header_t  header;
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
        retval = READSTAT_ERROR_SEEK;
        goto cleanup;
    }

    if (io->seek(0, READSTAT_SEEK_SET, io->io_ctx) == -1) {
        retval = READSTAT_ERROR_SEEK;
        goto cleanup;
    }

    if (strncmp(magic, "<sta", 4) == 0) {
        retval = dta_read_xmlish_preamble(ctx, &header);
    } else {
        if (io->read(&header, sizeof(header), io->io_ctx) != sizeof(header)) {
            retval = READSTAT_ERROR_READ;
            goto cleanup;
        }
    }

    retval = dta_ctx_init(ctx, header.nvar, header.nobs, header.byteorder, header.ds_format,
            parser->input_encoding, parser->output_encoding);
    if (retval != READSTAT_OK) {
        goto cleanup;
    }

    ctx->user_ctx = user_ctx;
    ctx->file_size = file_size;
    ctx->progress_handler = parser->progress_handler;
    ctx->variable_handler = parser->variable_handler;
    ctx->value_handler = parser->value_handler;
    ctx->value_label_handler = parser->value_label_handler;
    ctx->row_limit = ctx->nobs;
    if (parser->row_limit > 0 && parser->row_limit < ctx->nobs)
        ctx->row_limit = parser->row_limit;

    retval = dta_update_progress(ctx);
    if (retval != READSTAT_OK)
        goto cleanup;
    
    if (parser->info_handler) {
        if (parser->info_handler(ctx->row_limit, ctx->nvar, user_ctx)) {
            retval = READSTAT_ERROR_USER_ABORT;
            goto cleanup;
        }
    }
    
    if ((retval = dta_skip_label_and_timestamp(ctx)) != READSTAT_OK)
        goto cleanup;

    if ((retval = dta_read_tag(ctx, "</header>")) != READSTAT_OK) {
        goto cleanup;
    }

    if (dta_read_map(ctx) != READSTAT_OK) {
        retval = READSTAT_ERROR_READ;
        goto cleanup;
    }

    if (dta_read_descriptors(ctx) != READSTAT_OK) {
        retval = READSTAT_ERROR_READ;
        goto cleanup;
    }

    for (i=0; i<ctx->nvar; i++) {
        size_t      max_len;
        dta_type_info(ctx->typlist[i], &max_len, ctx);
        ctx->record_len += max_len;
    }

    if (ctx->record_len == 0) {
        retval = READSTAT_ERROR_PARSE;
        goto cleanup;
    }

    if ((retval = dta_handle_variables(ctx)) != READSTAT_OK)
        goto cleanup;

    if ((retval = dta_skip_expansion_fields(ctx)) != READSTAT_OK)
        goto cleanup;
    
    if ((retval = dta_read_tag(ctx, "<data>")) != READSTAT_OK)
        goto cleanup;

    if ((retval = dta_update_progress(ctx)) != READSTAT_OK)
        goto cleanup;

    if ((retval = dta_handle_rows(ctx)) != READSTAT_OK)
        goto cleanup;

    if ((retval = dta_read_tag(ctx, "</data>")) != READSTAT_OK)
        goto cleanup;

    if (ctx->file_is_xmlish) {
        if (io->seek(ctx->value_labels_offset, READSTAT_SEEK_SET, io->io_ctx) == -1) {
            retval = READSTAT_ERROR_SEEK;
            goto cleanup;
        }
        if ((retval = dta_read_tag(ctx, "<value_labels>")) != READSTAT_OK) {
            goto cleanup;
        }
    }

    if ((retval = dta_handle_value_labels(ctx)) != READSTAT_OK)
        goto cleanup;
    
    if ((retval = dta_update_progress(ctx)) != READSTAT_OK)
        goto cleanup;

cleanup:
    io->close(io->io_ctx);
    if (ctx)
        dta_ctx_free(ctx);

    return retval;
}
