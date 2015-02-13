
#include <stdlib.h>
#include <math.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include "readstat_io.h"
#include "readstat_dta.h"

static inline readstat_types_t dta_type_info(uint16_t typecode, size_t *max_len, dta_ctx_t *ctx);
static readstat_error_t dta_read_descriptors(int fd, dta_ctx_t *ctx);
static readstat_error_t dta_read_tag(int fd, dta_ctx_t *ctx, const char *tag);
static readstat_error_t dta_read_long_string(int fd, dta_ctx_t *ctx, int v, int o, char **long_string_out);
static int dta_skip_expansion_fields(int fd, dta_ctx_t *ctx);

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

static int dta_skip_expansion_fields(int fd, dta_ctx_t *ctx) {
    if (ctx->file_is_xmlish) {
        if (lseek(fd, ctx->data_offset, SEEK_SET) == -1)
            return -1;

        return 0;
    }
    if (ctx->expansion_len_len == 0)
        return 0;
    
    while (1) {
        size_t len;
        char data_type;
        if (ctx->expansion_len_len == 2) {
            dta_short_expansion_field_t  expansion_field;
            if (read(fd, &expansion_field, sizeof(expansion_field)) != sizeof(expansion_field))
                return -1;

            if (ctx->machine_needs_byte_swap) {
                len = byteswap2(expansion_field.len);
            } else {
                len = expansion_field.len;
            }
            
            data_type = expansion_field.data_type;
        } else {
            dta_expansion_field_t  expansion_field;
            if (read(fd, &expansion_field, sizeof(expansion_field)) != sizeof(expansion_field))
                return -1;
            
            if (ctx->machine_needs_byte_swap) {
                len = byteswap4(expansion_field.len);
            } else {
                len = expansion_field.len;
            }
            
            data_type = expansion_field.data_type;
        }

        if (data_type == 0 && len == 0)
            return 0;
        
        if (data_type != 1)
            return -1;

        if (lseek(fd, len, SEEK_CUR) == -1)
            return -1;
    }

    return -1;
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
    if (lseek(fd, ctx->strls_offset, SEEK_SET) != ctx->strls_offset) {
        retval = READSTAT_ERROR_READ;
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
            if (lseek(fd, header.len, SEEK_CUR) == -1) {
                retval = READSTAT_ERROR_READ;
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

    if ((fd = readstat_open(filename)) == -1) {
        retval = READSTAT_ERROR_OPEN;
        goto cleanup;
    }

    char magic[4];
    if (read(fd, magic, 4) != 4) {
        retval = READSTAT_ERROR_READ;
        goto cleanup;
    }

    lseek(fd, 0, SEEK_SET);

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
    
    if (parser->info_handler) {
        if (parser->info_handler(ctx->nobs, ctx->nvar, user_ctx)) {
            retval = READSTAT_ERROR_USER_ABORT;
            goto cleanup;
        }
    }
    
    if (ctx->file_is_xmlish) {
        unsigned char label_len;

        if ((retval = dta_read_tag(fd, ctx, "<label>")) != READSTAT_OK) {
            goto cleanup;
        }
        
        if (read(fd, &label_len, 1) != 1) {
            retval = READSTAT_ERROR_READ;
            goto cleanup;
        }
        
        if (lseek(fd, label_len, SEEK_CUR) == -1) {
            retval = READSTAT_ERROR_READ;
            goto cleanup;
        }
        
        if ((retval = dta_read_tag(fd, ctx, "</label>")) != READSTAT_OK) {
            goto cleanup;
        }
        
        if ((retval = dta_read_tag(fd, ctx, "<timestamp>")) != READSTAT_OK) {
            goto cleanup;
        }
        
        if (read(fd, &label_len, 1) != 1) {
            retval = READSTAT_ERROR_READ;
            goto cleanup;
        }
        
        if (lseek(fd, label_len, SEEK_CUR) == -1) {
            retval = READSTAT_ERROR_READ;
            goto cleanup;
        }

        if ((retval = dta_read_tag(fd, ctx, "</timestamp>")) != READSTAT_OK) {
            goto cleanup;
        }
    } else {
        if (lseek(fd, ctx->data_label_len, SEEK_CUR) == -1) {
            retval = READSTAT_ERROR_READ;
            goto cleanup;
        }
        
        if (ctx->time_stamp_len) {
            if (lseek(fd, ctx->time_stamp_len, SEEK_CUR) == -1) {
                retval = READSTAT_ERROR_READ;
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
        
        const char *variable_name = &ctx->varlist[ctx->variable_name_len*i];
        const char *variable_label = NULL;
        const char *value_labels = NULL;
        const char *variable_format = NULL;
        
        if (ctx->variable_labels[ctx->variable_labels_entry_len*i])
            variable_label = &ctx->variable_labels[ctx->variable_labels_entry_len*i];
        if (ctx->lbllist[ctx->lbllist_entry_len*i])
            value_labels = &ctx->lbllist[ctx->lbllist_entry_len*i];
        if (ctx->fmtlist[ctx->fmtlist_entry_len*i])
            variable_format = &ctx->fmtlist[ctx->fmtlist_entry_len*i];

        if (parser->variable_handler) {
            if (parser->variable_handler(i, variable_name, variable_format, variable_label, 
                        value_labels, type, user_ctx)) {
                retval = READSTAT_ERROR_USER_ABORT;
                goto cleanup;
            }
        }
    }

    if (dta_skip_expansion_fields(fd, ctx) != 0) {
        retval = READSTAT_ERROR_READ;
        goto cleanup;
    }
    
    if (record_len == 0) {
        retval = READSTAT_ERROR_PARSE;
        goto cleanup;
    }

    if ((retval = dta_read_tag(fd, ctx, "<data>")) != READSTAT_OK) {
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
            readstat_types_t type = dta_type_info(ctx->typlist[j], &max_len, ctx);
            void *value = NULL;

            if (type == READSTAT_TYPE_STRING) {
                int needs_null = 1;
                int k;
                for (k=0; k<max_len; k++) {
                    if (buf[offset+k] == '\0') {
                        needs_null = 0;
                        break;
                    }
                }
                if (needs_null) {
                    memcpy(str_buf, &buf[offset], max_len);
                    str_buf[max_len] = '\0';
                    value = str_buf;
                } else {
                    value = &buf[offset];
                }
            } else if (type == READSTAT_TYPE_LONG_STRING) {
                uint32_t v, o;
                v = *((uint32_t *)&buf[offset]);
                o = *((uint32_t *)&buf[offset+4]);
                if (ctx->machine_needs_byte_swap) {
                    v = byteswap4(v);
                    o = byteswap4(o);
                }
                if (v > 0 && o > 0) {
                    off_t cur_pos = lseek(fd, 0, SEEK_CUR);
                    if (cur_pos == -1) {
                        retval = READSTAT_ERROR_READ;
                        goto cleanup;
                    }
                    retval = dta_read_long_string(fd, ctx, v, o, &long_string);
                    if (retval != READSTAT_OK) {
                        goto cleanup;
                    }
                    value = long_string;
                    if (lseek(fd, cur_pos, SEEK_SET) == -1) {
                        retval = READSTAT_ERROR_READ;
                        goto cleanup;
                    }
                }
            } else if (type == READSTAT_TYPE_CHAR) {
                char byte = buf[offset];
                if (ctx->machine_is_twos_complement) {
                    byte = ones_to_twos_complement1(byte);
                }
                value = byte <= DTA_MAX_CHAR ? &byte : NULL;
            } else if (type == READSTAT_TYPE_INT16) {
                int16_t num = *((int16_t *)&buf[offset]);
                if (ctx->machine_needs_byte_swap) {
                    num = byteswap2(num);
                }
                if (ctx->machine_is_twos_complement) {
                    num = ones_to_twos_complement2(num);
                }
                value = num <= DTA_MAX_INT16 ? &num : NULL;
            } else if (type == READSTAT_TYPE_INT32) {
                int32_t num = *((int32_t *)&buf[offset]);
                if (ctx->machine_needs_byte_swap) {
                    num = byteswap4(num);
                }
                if (ctx->machine_is_twos_complement) {
                    num = ones_to_twos_complement4(num);
                }
                value = num <= DTA_MAX_INT32 ? &num : NULL;
            } else if (type == READSTAT_TYPE_FLOAT) {
                float num = *((float *)&buf[offset]);
                if (ctx->machine_needs_byte_swap) {
                    num = byteswap_float(num);
                }
                value = num <= DTA_MAX_FLOAT ? &num : NULL;
            } else if (type == READSTAT_TYPE_DOUBLE) {
                double num = *((double *)&buf[offset]);
                if (ctx->machine_needs_byte_swap) {
                    num = byteswap_double(num);
                }
                value = num <= DTA_MAX_DOUBLE ? &num : NULL;
            }

            if (parser->value_handler) {
                if (parser->value_handler(i, j, value, type, user_ctx)) {
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
    }

    if ((retval = dta_read_tag(fd, ctx, "</data>")) != READSTAT_OK) {
        goto cleanup;
    }

    if (ctx->file_is_xmlish) {
        if (lseek(fd, ctx->value_labels_offset, SEEK_SET) == -1) {
            retval = READSTAT_ERROR_READ;
            goto cleanup;
        }
        if ((retval = dta_read_tag(fd, ctx, "<value_labels>")) != READSTAT_OK) {
            goto cleanup;
        }
    }
    
    if (parser->value_label_handler) {
        while (1) {
            size_t len = 0;
            char *table_buffer;

            if (ctx->value_label_table_len_len == 2) {
                dta_short_value_label_table_header_t table_header;
                if (read(fd, &table_header, sizeof(dta_short_value_label_table_header_t)) < 
                        sizeof(dta_short_value_label_table_header_t))
                    break;

                len = table_header.len;
            
                if (ctx->machine_needs_byte_swap)
                    len = byteswap2(table_header.len);
                
                if ((table_buffer = malloc(8 * len)) == NULL) {
                    retval = READSTAT_ERROR_MALLOC;
                    goto cleanup;
                }
                
                if (read(fd, table_buffer, 8 * len) < 8 * len) {
                    free(table_buffer);
                    break;
                }
                
                int32_t l;
                for (l=0; l<len; l++) {
                    if (parser->value_label_handler(table_header.labname, &l, READSTAT_TYPE_INT32, table_buffer + 8 * l, user_ctx)) {
                        retval = READSTAT_ERROR_USER_ABORT;
                        free(table_buffer);
                        goto cleanup;
                    }
                }
            } else {
                if (dta_read_tag(fd, ctx, "<lbl>") != READSTAT_OK) {
                    break;
                }

                dta_value_label_table_header_t table_header;
                if (read(fd, &table_header, sizeof(dta_value_label_table_header_t)) < 
                        sizeof(dta_value_label_table_header_t))
                    break;
            
                len = table_header.len;
            
                if (ctx->machine_needs_byte_swap)
                    len = byteswap4(table_header.len);
                        
                if ((table_buffer = malloc(len)) == NULL) {
                    retval = READSTAT_ERROR_MALLOC;
                    goto cleanup;
                }
                
                if (read(fd, table_buffer, len) < len) {
                    free(table_buffer);
                    break;
                }

                if ((retval = dta_read_tag(fd, ctx, "</lbl>")) != READSTAT_OK) {
                    free(table_buffer);
                    goto cleanup;
                }
                
                int32_t n = *(int32_t *)table_buffer;
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
                int i;
                
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
                        if (parser->value_label_handler(table_header.labname, &val[i], READSTAT_TYPE_INT32, &txt[off[i]], user_ctx)) {
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

