
#include <stdlib.h>
#include <math.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include "readstat_dta.h"
#include "readstat_io.h"
#include "readstat_convert.h"

static readstat_error_t dta_update_progress(int fd, dta_ctx_t *ctx);
static inline readstat_types_t dta_type_info(uint16_t typecode, size_t *max_len, dta_ctx_t *ctx);
static readstat_error_t dta_read_descriptors(int fd, dta_ctx_t *ctx);
static readstat_error_t dta_read_tag(int fd, dta_ctx_t *ctx, const char *tag);
static readstat_error_t dta_read_long_string(int fd, dta_ctx_t *ctx, int v, int o, char **long_string_out);
static readstat_error_t dta_skip_expansion_fields(int fd, dta_ctx_t *ctx);

static readstat_error_t dta_update_progress(int fd, dta_ctx_t *ctx) {
    if (!ctx->progress_handler)
        return READSTAT_OK;

    return readstat_update_progress(fd, ctx->file_size, ctx->progress_handler, ctx->user_ctx);
}

static inline readstat_types_t dta_type_info(uint16_t typecode, size_t *max_len, dta_ctx_t *ctx) {
    size_t len = 0;
    readstat_types_t type = READSTAT_TYPE_STRING;
    if (ctx->typlist_version == 111) {
        switch (typecode) {
            case DTA_111_TYPE_CODE_CHAR:
                len = 1; type = READSTAT_TYPE_CHAR; break;
            case DTA_111_TYPE_CODE_INT16:
                len = 2; type = READSTAT_TYPE_INT16; break;
            case DTA_111_TYPE_CODE_INT32:
                len = 4; type = READSTAT_TYPE_INT32; break;
            case DTA_111_TYPE_CODE_FLOAT:
                len = 4; type = READSTAT_TYPE_FLOAT; break;
            case DTA_111_TYPE_CODE_DOUBLE:
                len = 8; type = READSTAT_TYPE_DOUBLE; break;
            default:
                len = typecode; type = READSTAT_TYPE_STRING; break;
        }
    } else if (ctx->typlist_version == 117) {
        switch (typecode) {
            case DTA_117_TYPE_CODE_CHAR:
                len = 1; type = READSTAT_TYPE_CHAR; break;
            case DTA_117_TYPE_CODE_INT16:
                len = 2; type = READSTAT_TYPE_INT16; break;
            case DTA_117_TYPE_CODE_INT32:
                len = 4; type = READSTAT_TYPE_INT32; break;
            case DTA_117_TYPE_CODE_FLOAT:
                len = 4; type = READSTAT_TYPE_FLOAT; break;
            case DTA_117_TYPE_CODE_DOUBLE:
                len = 8; type = READSTAT_TYPE_DOUBLE; break;
            case DTA_117_TYPE_CODE_STRL:
                len = 8; type = READSTAT_TYPE_LONG_STRING; break;
            default:
                len = typecode; type = READSTAT_TYPE_STRING; break;
        }
    } else {
        switch (typecode) {
            case DTA_OLD_TYPE_CODE_CHAR:
                len = 1; type = READSTAT_TYPE_CHAR; break;
            case DTA_OLD_TYPE_CODE_INT16:
                len = 2; type = READSTAT_TYPE_INT16; break;
            case DTA_OLD_TYPE_CODE_INT32:
                len = 4; type = READSTAT_TYPE_INT32; break;
            case DTA_OLD_TYPE_CODE_FLOAT:
                len = 4; type = READSTAT_TYPE_FLOAT; break;
            case DTA_OLD_TYPE_CODE_DOUBLE:
                len = 8; type = READSTAT_TYPE_DOUBLE; break;
            default:
                len = typecode - 0x7F; type = READSTAT_TYPE_STRING; break;
        }
    }
    
    *max_len = len;
    return type;
}

static readstat_variable_t *dta_init_variable(dta_ctx_t *ctx, int i, readstat_types_t type) {
    readstat_variable_t *variable = calloc(1, sizeof(readstat_variable_t));

    variable->type = type;
    variable->index = i;

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
    }

    return variable;
}

static readstat_error_t dta_read_map(int fd, dta_ctx_t *ctx) {
    if (!ctx->file_is_xmlish)
        return 0;

    readstat_error_t retval = READSTAT_OK;
    if ((retval = dta_read_tag(fd, ctx, "<map>")) != READSTAT_OK) {
        goto cleanup;
    }

    uint64_t map_buffer[14];

    if (read(fd, map_buffer, sizeof(map_buffer)) != sizeof(map_buffer)) {
        retval = READSTAT_ERROR_READ;
        goto cleanup;
    }

    ctx->data_offset = ctx->machine_needs_byte_swap ? byteswap8(map_buffer[9]) : map_buffer[9];
    ctx->strls_offset = ctx->machine_needs_byte_swap ? byteswap8(map_buffer[10]) : map_buffer[10];
    ctx->value_labels_offset = ctx->machine_needs_byte_swap ? byteswap8(map_buffer[11]) : map_buffer[11];

    if ((retval = dta_read_tag(fd, ctx, "</map>")) != READSTAT_OK) {
        goto cleanup;
    }

cleanup:
    return retval;
}

static readstat_error_t dta_read_descriptors(int fd, dta_ctx_t *ctx) {
    if (dta_read_tag(fd, ctx, "<variable_types>") != READSTAT_OK)
        return -1;

    size_t buffer_len = ctx->nvar * ctx->typlist_entry_len;
    unsigned char *buffer = malloc(buffer_len);
    if (read(fd, buffer, buffer_len) != buffer_len) {
        free(buffer);
        return -1;
    }
    int i;
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
    free(buffer);

    if (dta_read_tag(fd, ctx, "</variable_types>") != READSTAT_OK)
        return -1;

    if (dta_read_tag(fd, ctx, "<varnames>") != READSTAT_OK ||
            read(fd, ctx->varlist, ctx->varlist_len) != ctx->varlist_len ||
            dta_read_tag(fd, ctx, "</varnames>") != READSTAT_OK)
        return -1;

    if (dta_read_tag(fd, ctx, "<sortlist>") != READSTAT_OK || 
            read(fd, ctx->srtlist, ctx->srtlist_len) != ctx->srtlist_len ||
            dta_read_tag(fd, ctx, "</sortlist>") != READSTAT_OK)
        return -1;

    if (dta_read_tag(fd, ctx, "<formats>") != READSTAT_OK ||
            read(fd, ctx->fmtlist, ctx->fmtlist_len) != ctx->fmtlist_len ||
            dta_read_tag(fd, ctx, "</formats>") != READSTAT_OK)
        return -1;

    if (dta_read_tag(fd, ctx, "<value_label_names>") != READSTAT_OK ||
            read(fd, ctx->lbllist, ctx->lbllist_len) != ctx->lbllist_len ||
            dta_read_tag(fd, ctx, "</value_label_names>") != READSTAT_OK)
        return -1;

    if (dta_read_tag(fd, ctx, "<variable_labels>") != READSTAT_OK ||
            read(fd, ctx->variable_labels, ctx->variable_labels_len) != ctx->variable_labels_len ||
            dta_read_tag(fd, ctx, "</variable_labels>") != READSTAT_OK)
        return -1;

    return 0;
}

static readstat_error_t dta_skip_expansion_fields(int fd, dta_ctx_t *ctx) {
    if (ctx->file_is_xmlish) {
        if (readstat_lseek(fd, ctx->data_offset, SEEK_SET) == -1)
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
            if (read(fd, &expansion_field, sizeof(expansion_field)) != sizeof(expansion_field))
                return READSTAT_ERROR_READ;

            if (ctx->machine_needs_byte_swap) {
                len = byteswap2(expansion_field.len);
            } else {
                len = expansion_field.len;
            }
            
            data_type = expansion_field.data_type;
        } else {
            dta_expansion_field_t  expansion_field;
            if (read(fd, &expansion_field, sizeof(expansion_field)) != sizeof(expansion_field))
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

        if (readstat_lseek(fd, len, SEEK_CUR) == -1)
            return READSTAT_ERROR_SEEK;
    }

    return READSTAT_ERROR_PARSE;
}

static readstat_error_t dta_read_tag(int fd, dta_ctx_t *ctx, const char *tag) {
    readstat_error_t retval = READSTAT_OK;
    if (ctx != NULL && !ctx->file_is_xmlish)
        return retval;

    char buffer[256];
    size_t len = strlen(tag);
    if (read(fd, buffer, len) != len) {
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

static readstat_error_t dta_read_long_string(int fd, dta_ctx_t *ctx, int v, int o, char **long_string_out) {
    readstat_error_t retval = READSTAT_OK;
    if (readstat_lseek(fd, ctx->strls_offset, SEEK_SET) != ctx->strls_offset) {
        retval = READSTAT_ERROR_SEEK;
        goto cleanup;
    }

    retval = dta_read_tag(fd, ctx, "<strls>");
    if (retval != READSTAT_OK)
        goto cleanup;

    dta_gso_header_t header;

    while (1) {
        if (read(fd, &header, sizeof(dta_gso_header_t)) != sizeof(dta_gso_header_t)) {
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
                if (read(fd, string_buf, header.len) != header.len) {
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
            if (readstat_lseek(fd, header.len, SEEK_CUR) == -1) {
                retval = READSTAT_ERROR_SEEK;
                goto cleanup;
            }
        }
    }

cleanup:
    return retval;
}

readstat_error_t dta_read_xmlish_preamble(int fd, dta_ctx_t *ctx, dta_header_t *header) {
    readstat_error_t retval = READSTAT_OK;
    
    if ((retval = dta_read_tag(fd, ctx, "<stata_dta>")) != READSTAT_OK) {
        goto cleanup;
    }
    if ((retval = dta_read_tag(fd, ctx, "<header>")) != READSTAT_OK) {
        goto cleanup;
    }

    char ds_format[3];
    if ((retval = dta_read_tag(fd, ctx, "<release>")) != READSTAT_OK) {
        goto cleanup;
    }
    if (read(fd, ds_format, sizeof(ds_format)) != sizeof(ds_format)) {
        retval = READSTAT_ERROR_READ;
        goto cleanup;
    }

    header->ds_format = 100 * (ds_format[0] - '0') + 10 * (ds_format[1] - '0') + (ds_format[2] - '0');

    if ((retval = dta_read_tag(fd, ctx, "</release>")) != READSTAT_OK) {
        goto cleanup;
    }

    char byteorder[3];
    if ((retval = dta_read_tag(fd, ctx, "<byteorder>")) != READSTAT_OK) {
        goto cleanup;
    }
    if (read(fd, byteorder, sizeof(byteorder)) != sizeof(byteorder)) {
        retval = READSTAT_ERROR_READ;
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
    if ((retval = dta_read_tag(fd, ctx, "</byteorder>")) != READSTAT_OK) {
        goto cleanup;
    }

    if ((retval = dta_read_tag(fd, ctx, "<K>")) != READSTAT_OK) {
        goto cleanup;
    }
    if (read(fd, &header->nvar, sizeof(int16_t)) != sizeof(int16_t)) {
        retval = READSTAT_ERROR_READ;
        goto cleanup;
    }
    if ((retval = dta_read_tag(fd, ctx, "</K>")) != READSTAT_OK) {
        goto cleanup;
    }

    if ((retval = dta_read_tag(fd, ctx, "<N>")) != READSTAT_OK) {
        goto cleanup;
    }
    if (read(fd, &header->nobs, sizeof(int32_t)) != sizeof(int32_t)) {
        retval = READSTAT_ERROR_READ;
        goto cleanup;
    }
    if (header->ds_format >= 118) {
        /* Only support files < 4 billion rows for now */
        if (header->byteorder == DTA_HILO) {
            if (read(fd, &header->nobs, sizeof(int32_t)) != sizeof(int32_t)) {
                retval = READSTAT_ERROR_READ;
                goto cleanup;
            }
        } else {
            if (readstat_lseek(fd, 4, SEEK_CUR) == -1) {
                retval = READSTAT_ERROR_SEEK;
                goto cleanup;
            }
        }
    }
    if ((retval = dta_read_tag(fd, ctx, "</N>")) != READSTAT_OK) {
        goto cleanup;
    }

cleanup:
    return retval;
}

readstat_error_t readstat_parse_dta(readstat_parser_t *parser, const char *filename, void *user_ctx) {
    readstat_error_t retval = READSTAT_OK;
    int i;
    size_t  record_len = 0;
    int fd = -1;
    char *buf = NULL;
    dta_header_t  header;
    dta_ctx_t    *ctx = NULL;
    char  str_buf[2048];
    char *long_string = NULL;
    size_t file_size = 0;

    if ((fd = readstat_open(filename)) == -1) {
        retval = READSTAT_ERROR_OPEN;
        goto cleanup;
    }

    char magic[4];
    if (read(fd, magic, 4) != 4) {
        retval = READSTAT_ERROR_READ;
        goto cleanup;
    }

    file_size = readstat_lseek(fd, 0, SEEK_END);
    if (file_size == -1) {
        retval = READSTAT_ERROR_SEEK;
        goto cleanup;
    }

    if (readstat_lseek(fd, 0, SEEK_SET) == -1) {
        retval = READSTAT_ERROR_SEEK;
        goto cleanup;
    }

    if (strncmp(magic, "<sta", 4) == 0) {
        retval = dta_read_xmlish_preamble(fd, ctx, &header);
    } else {
        if (read(fd, &header, sizeof(header)) != sizeof(header)) {
            retval = READSTAT_ERROR_READ;
            goto cleanup;
        }
    }

    if ((ctx = dta_ctx_init(header.nvar, header.nobs, header.byteorder, header.ds_format)) == NULL) {
        retval = READSTAT_ERROR_MALLOC;
        goto cleanup;
    }

    ctx->user_ctx = user_ctx;
    ctx->file_size = file_size;
    ctx->progress_handler = parser->progress_handler;

    retval = dta_update_progress(fd, ctx);
    if (retval != READSTAT_OK)
        goto cleanup;
    
    if (parser->info_handler) {
        if (parser->info_handler(ctx->nobs, ctx->nvar, user_ctx)) {
            retval = READSTAT_ERROR_USER_ABORT;
            goto cleanup;
        }
    }
    
    if (ctx->file_is_xmlish) {
        uint16_t label_len = 0;
        unsigned char timestamp_len;

        if ((retval = dta_read_tag(fd, ctx, "<label>")) != READSTAT_OK) {
            goto cleanup;
        }
        
        if (ctx->data_label_len_len == 2) {
            if (read(fd, &label_len, sizeof(uint16_t)) != sizeof(uint16_t)) {
                retval = READSTAT_ERROR_READ;
                goto cleanup;
            }
            label_len = ctx->machine_needs_byte_swap ? byteswap2(label_len) : label_len;
        } else if (ctx->data_label_len_len == 1) {
            unsigned char label_len_char;
            if (read(fd, &label_len_char, sizeof(unsigned char)) != sizeof(unsigned char)) {
                retval = READSTAT_ERROR_READ;
                goto cleanup;
            }
            label_len = label_len_char;
        }
        
        if (readstat_lseek(fd, label_len, SEEK_CUR) == -1) {
            retval = READSTAT_ERROR_SEEK;
            goto cleanup;
        }
        
        if ((retval = dta_read_tag(fd, ctx, "</label>")) != READSTAT_OK) {
            goto cleanup;
        }
        
        if ((retval = dta_read_tag(fd, ctx, "<timestamp>")) != READSTAT_OK) {
            goto cleanup;
        }
        
        if (read(fd, &timestamp_len, 1) != 1) {
            retval = READSTAT_ERROR_READ;
            goto cleanup;
        }
        
        if (readstat_lseek(fd, timestamp_len, SEEK_CUR) == -1) {
            retval = READSTAT_ERROR_SEEK;
            goto cleanup;
        }

        if ((retval = dta_read_tag(fd, ctx, "</timestamp>")) != READSTAT_OK) {
            goto cleanup;
        }
    } else {
        if (readstat_lseek(fd, ctx->data_label_len, SEEK_CUR) == -1) {
            retval = READSTAT_ERROR_SEEK;
            goto cleanup;
        }
        
        if (ctx->time_stamp_len) {
            if (readstat_lseek(fd, ctx->time_stamp_len, SEEK_CUR) == -1) {
                retval = READSTAT_ERROR_SEEK;
                goto cleanup;
            }
        }
    }
    
    if ((retval = dta_read_tag(fd, ctx, "</header>")) != READSTAT_OK) {
        goto cleanup;
    }

    if (dta_read_map(fd, ctx) != READSTAT_OK) {
        retval = READSTAT_ERROR_READ;
        goto cleanup;
    }

    if (dta_read_descriptors(fd, ctx) != READSTAT_OK) {
        retval = READSTAT_ERROR_READ;
        goto cleanup;
    }

    for (i=0; i<ctx->nvar; i++) {
        size_t      max_len;
        readstat_types_t type = dta_type_info(ctx->typlist[i], &max_len, ctx);

        record_len += max_len;

        if (type == READSTAT_TYPE_STRING)
            max_len++; /* might append NULL */

        if (parser->variable_handler) {
            readstat_variable_t *variable = dta_init_variable(ctx, i, type);

            const char *value_labels = NULL;

            if (ctx->lbllist[ctx->lbllist_entry_len*i])
                value_labels = &ctx->lbllist[ctx->lbllist_entry_len*i];

            int cb_retval = parser->variable_handler(i, variable, value_labels, user_ctx);

            free(variable);

            if (cb_retval) {
                retval = READSTAT_ERROR_USER_ABORT;
                goto cleanup;
            }
        }
    }

    if ((retval = dta_skip_expansion_fields(fd, ctx)) != READSTAT_OK) {
        goto cleanup;
    }
    
    if (record_len == 0) {
        retval = READSTAT_ERROR_PARSE;
        goto cleanup;
    }

    if ((retval = dta_read_tag(fd, ctx, "<data>")) != READSTAT_OK) {
        goto cleanup;
    }

    if ((retval = dta_update_progress(fd, ctx)) != READSTAT_OK) {
        goto cleanup;
    }

    if ((buf = malloc(record_len)) == NULL) {
        retval = READSTAT_ERROR_MALLOC;
        goto cleanup;
    }

    for (i=0; i<ctx->nobs; i++) {
        if (read(fd, buf, record_len) != record_len) {
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
                    off_t cur_pos = readstat_lseek(fd, 0, SEEK_CUR);
                    if (cur_pos == -1) {
                        retval = READSTAT_ERROR_SEEK;
                        goto cleanup;
                    }
                    retval = dta_read_long_string(fd, ctx, v, o, &long_string);
                    if (retval != READSTAT_OK) {
                        goto cleanup;
                    }
                    value.v.string_value = long_string;
                    if (readstat_lseek(fd, cur_pos, SEEK_SET) == -1) {
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
                    if (byte > DTA_MISSING_CHAR) {
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
                    if (num > DTA_MISSING_INT16) {
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
                    if (num > DTA_MISSING_INT32) {
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
                    if (num > DTA_MISSING_FLOAT) {
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
                    if (num > DTA_MISSING_DOUBLE) {
                        value.tag = 'a' + ((num - DTA_MISSING_DOUBLE_A) >> 40);
                    }
                } else {
                    memcpy(&d_num, &num, sizeof(int64_t));
                }
                value.v.double_value = d_num;
            }

            if (parser->value_handler) {
                if (parser->value_handler(i, j, value, user_ctx)) {
                    retval = READSTAT_ERROR_USER_ABORT;
                    goto cleanup;
                }
            }

            if (long_string) {
                free(long_string);
                long_string = NULL;
            }

            offset += max_len;
        }
        if ((retval = dta_update_progress(fd, ctx)) != READSTAT_OK) {
            goto cleanup;
        }
    }

    if ((retval = dta_read_tag(fd, ctx, "</data>")) != READSTAT_OK) {
        goto cleanup;
    }

    if (ctx->file_is_xmlish) {
        if (readstat_lseek(fd, ctx->value_labels_offset, SEEK_SET) == -1) {
            retval = READSTAT_ERROR_SEEK;
            goto cleanup;
        }
        if ((retval = dta_read_tag(fd, ctx, "<value_labels>")) != READSTAT_OK) {
            goto cleanup;
        }
    }
    
    if (parser->value_label_handler) {
        while (1) {
            size_t len = 0;
            char labname[129];
            char *table_buffer;
            int32_t i, n;

            if (ctx->value_label_table_len_len == 2) {
                int16_t table_header_len;
                if (read(fd, &table_header_len, sizeof(int16_t)) < sizeof(int16_t))
                    break;

                n = table_header_len;
            
                if (ctx->machine_needs_byte_swap)
                    n = byteswap2(table_header_len);

                len = 8 * n;
            } else {
                if (dta_read_tag(fd, ctx, "<lbl>") != READSTAT_OK) {
                    break;
                }

                int32_t table_header_len;
                if (read(fd, &table_header_len, sizeof(int32_t)) < sizeof(int32_t))
                    break;

                len = table_header_len;
            
                if (ctx->machine_needs_byte_swap)
                    len = byteswap4(table_header_len);
            }
                
            if (read(fd, labname, ctx->value_label_table_labname_len) < ctx->value_label_table_labname_len)
                break;

            if (readstat_lseek(fd, ctx->value_label_table_padding_len, SEEK_CUR) == -1)
                break;

            if ((table_buffer = malloc(len)) == NULL) {
                retval = READSTAT_ERROR_MALLOC;
                goto cleanup;
            }

            if (read(fd, table_buffer, len) < len) {
                free(table_buffer);
                break;
            }

            if (ctx->value_label_table_len_len == 2) {
                for (i=0; i<n; i++) {
                    readstat_value_t value = { .v = { .i32_value = i }, .type = READSTAT_TYPE_INT32 };
                    if (parser->value_label_handler(labname, value, table_buffer + 8 * i, user_ctx)) {
                        retval = READSTAT_ERROR_USER_ABORT;
                        free(table_buffer);
                        goto cleanup;
                    }
                }
            } else {
                if ((retval = dta_read_tag(fd, ctx, "</lbl>")) != READSTAT_OK) {
                    free(table_buffer);
                    goto cleanup;
                }
                
                n = *(int32_t *)table_buffer;

                int32_t txtlen = *((int32_t *)table_buffer+1);
                if (ctx->machine_needs_byte_swap) {
                    n = byteswap4(n);
                    txtlen = byteswap4(txtlen);
                }
                
                if (8*n + 8 > len || 8*n + 8 + txtlen > len || n < 0 || txtlen < 0) {
                    free(table_buffer);
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
                        if (parser->value_label_handler(labname, value, &txt[off[i]], user_ctx)) {
                            retval = READSTAT_ERROR_USER_ABORT;
                            free(table_buffer);
                            goto cleanup;
                        }
                    }
                }
            }
            
            free(table_buffer);
        }
    }

    if ((retval = dta_update_progress(fd, ctx)) != READSTAT_OK) {
        goto cleanup;
    }

cleanup:
    if (fd != -1)
        readstat_close(fd);
    if (ctx)
        dta_ctx_free(ctx);
    if (buf)
        free(buf);
    if (long_string)
        free(long_string);

    return retval;
}

