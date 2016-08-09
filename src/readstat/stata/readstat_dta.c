#include <stdlib.h>
#include <math.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>

#include "../readstat.h"
#include "../readstat_iconv.h"
#include "../readstat_bits.h"

#include "readstat_dta.h"

#define DTA_MIN_VERSION 104
#define DTA_MAX_VERSION 118

dta_ctx_t *dta_ctx_alloc(readstat_io_t *io) {
    dta_ctx_t *ctx;
    if ((ctx = malloc(sizeof(dta_ctx_t))) == NULL) {
        return NULL;
    }
    memset(ctx, 0, sizeof(dta_ctx_t));

    ctx->io = io;
    ctx->initialized = 0;

    return ctx;
}

readstat_error_t dta_ctx_init(dta_ctx_t *ctx, int16_t nvar, int32_t nobs,
        unsigned char byteorder, unsigned char ds_format,
        const char *input_encoding, const char *output_encoding) {
    readstat_error_t retval = READSTAT_OK;
    int machine_byteorder = DTA_HILO;
    if (ds_format < DTA_MIN_VERSION || ds_format > DTA_MAX_VERSION)
        return READSTAT_ERROR_UNSUPPORTED_FILE_FORMAT_VERSION;

    if (machine_is_little_endian()) {
        machine_byteorder = DTA_LOHI;
    }

    if (byteorder != machine_byteorder) {
        ctx->machine_needs_byte_swap = 1;
    }

    ctx->nvar = ctx->machine_needs_byte_swap ? byteswap2(nvar) : nvar;
    ctx->nobs = ctx->machine_needs_byte_swap ? byteswap4(nobs) : nobs;
    
    ctx->machine_is_twos_complement = READSTAT_MACHINE_IS_TWOS_COMPLEMENT;

    if (ds_format < 105) {
        ctx->fmtlist_entry_len = 7;
    } else if (ds_format < 114) {
        ctx->fmtlist_entry_len = 12;
    } else if (ds_format < 118) {
        ctx->fmtlist_entry_len = 49;
    } else {
        ctx->fmtlist_entry_len = 57;
    }
    
    if (ds_format >= 117) {
        ctx->typlist_version = 117;
    } else if (ds_format >= 111) {
        ctx->typlist_version = 111;
    } else {
        ctx->typlist_version = 0;
    }

    if (ds_format >= 118) {
        ctx->data_label_len_len = 2;
        ctx->strl_v_len = 2;
        ctx->strl_o_len = 6;
    } else if (ds_format >= 117) {
        ctx->data_label_len_len = 1;
        ctx->strl_v_len = 4;
        ctx->strl_o_len = 4;
    }

    if (ds_format < 105) {
        ctx->expansion_len_len = 0;
    } else if (ds_format < 110) {
        ctx->expansion_len_len = 2;
    } else {
        ctx->expansion_len_len = 4;
    }
    
    if (ds_format < 110) {
        ctx->lbllist_entry_len = 9;
        ctx->variable_name_len = 9;
        ctx->ch_metadata_len = 9;
    } else if (ds_format < 118) {
        ctx->lbllist_entry_len = 33;
        ctx->variable_name_len = 33;
        ctx->ch_metadata_len = 33;
    } else {
        ctx->lbllist_entry_len = 129;
        ctx->variable_name_len = 129;
        ctx->ch_metadata_len = 129;
    }

    if (ds_format < 108) {
        ctx->variable_labels_entry_len = 32;
        ctx->data_label_len = 32;
    } else if (ds_format < 118) {
        ctx->variable_labels_entry_len = 81;
        ctx->data_label_len = 81;
    } else {
        ctx->variable_labels_entry_len = 321;
        ctx->data_label_len = 321;
    }

    if (ds_format < 105) {
        ctx->timestamp_len = 0;
        ctx->value_label_table_len_len = 2;
        ctx->value_label_table_labname_len = 12;
        ctx->value_label_table_padding_len = 2;
    } else {
        ctx->timestamp_len = 18;
        ctx->value_label_table_len_len = 4;
        if (ds_format < 118) {
            ctx->value_label_table_labname_len = 33;
        } else {
            ctx->value_label_table_labname_len = 129;
        }
        ctx->value_label_table_padding_len = 3;
    }

    if (ds_format < 117) {
        ctx->typlist_entry_len = 1;
        ctx->file_is_xmlish = 0;
    } else {
        ctx->typlist_entry_len = 2;
        ctx->file_is_xmlish = 1;
    }

    if (ds_format < 113) {
        ctx->max_int8 = DTA_OLD_MAX_INT8;
        ctx->max_int16 = DTA_OLD_MAX_INT16;
        ctx->max_int32 = DTA_OLD_MAX_INT32;
        ctx->max_float = DTA_OLD_MAX_FLOAT;
        ctx->max_double = DTA_OLD_MAX_DOUBLE;
    } else {
        ctx->max_int8 = DTA_113_MAX_INT8;
        ctx->max_int16 = DTA_113_MAX_INT16;
        ctx->max_int32 = DTA_113_MAX_INT32;
        ctx->max_float = DTA_113_MAX_FLOAT;
        ctx->max_double = DTA_113_MAX_DOUBLE;

        ctx->supports_tagged_missing = 1;
    }

    if (output_encoding) {
        if (input_encoding) {
            ctx->converter = iconv_open(output_encoding, input_encoding);
        } else if (ds_format < 118) {
            ctx->converter = iconv_open(output_encoding, "WINDOWS-1252");
        } else if (strcmp(output_encoding, "UTF-8") != 0) {
            ctx->converter = iconv_open(output_encoding, "UTF-8");
        }
        if (ctx->converter == (iconv_t)-1) {
            ctx->converter = NULL;
            retval = READSTAT_ERROR_UNSUPPORTED_CHARSET;
            goto cleanup;
        }
    }

    ctx->typlist_len = ctx->nvar * sizeof(uint16_t);
    ctx->varlist_len = ctx->variable_name_len * ctx->nvar * sizeof(char);
    ctx->srtlist_len = (ctx->nvar + 1) * sizeof(int16_t);
    ctx->fmtlist_len = ctx->fmtlist_entry_len * ctx->nvar * sizeof(char);
    ctx->lbllist_len = ctx->lbllist_entry_len * ctx->nvar * sizeof(char);
    ctx->variable_labels_len = ctx->variable_labels_entry_len * ctx->nvar * sizeof(char);

    if ((ctx->typlist = malloc(ctx->typlist_len)) == NULL) {
        retval = READSTAT_ERROR_MALLOC;
        goto cleanup;
    }
    if ((ctx->varlist = malloc(ctx->varlist_len)) == NULL) {
        retval = READSTAT_ERROR_MALLOC;
        goto cleanup;
    }
    if ((ctx->srtlist = malloc(ctx->srtlist_len)) == NULL) {
        retval = READSTAT_ERROR_MALLOC;
        goto cleanup;
    }
    if ((ctx->fmtlist = malloc(ctx->fmtlist_len)) == NULL) {
        retval = READSTAT_ERROR_MALLOC;
        goto cleanup;
    }
    if ((ctx->lbllist = malloc(ctx->lbllist_len)) == NULL) {
        retval = READSTAT_ERROR_MALLOC;
        goto cleanup;
    }

    if ((ctx->variable_labels = malloc(ctx->variable_labels_len)) == NULL) {
        retval = READSTAT_ERROR_MALLOC;
        goto cleanup;
    }

    ctx->initialized = 1;

cleanup:
    return retval;
}

void dta_ctx_free(dta_ctx_t *ctx) {
    if (ctx->typlist)
        free(ctx->typlist);
    if (ctx->varlist)
        free(ctx->varlist);
    if (ctx->srtlist)
        free(ctx->srtlist);
    if (ctx->fmtlist)
        free(ctx->fmtlist);
    if (ctx->lbllist)
        free(ctx->lbllist);
    if (ctx->variable_labels)
        free(ctx->variable_labels);
    if (ctx->converter)
        iconv_close(ctx->converter);
    if (ctx->data_label)
        free(ctx->data_label);
    if (ctx->strls) {
        int i;
        for (i=0; i<ctx->strls_count; i++) {
            free(ctx->strls[i]);
        }
        free(ctx->strls);
    }
    free(ctx);
}

readstat_type_t dta_type_info(uint16_t typecode, size_t *max_len, dta_ctx_t *ctx) {
    size_t len = 0;
    readstat_type_t type = READSTAT_TYPE_STRING;
    if (ctx->typlist_version == 111) {
        switch (typecode) {
            case DTA_111_TYPE_CODE_INT8:
                len = 1; type = READSTAT_TYPE_INT8; break;
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
            case DTA_117_TYPE_CODE_INT8:
                len = 1; type = READSTAT_TYPE_INT8; break;
            case DTA_117_TYPE_CODE_INT16:
                len = 2; type = READSTAT_TYPE_INT16; break;
            case DTA_117_TYPE_CODE_INT32:
                len = 4; type = READSTAT_TYPE_INT32; break;
            case DTA_117_TYPE_CODE_FLOAT:
                len = 4; type = READSTAT_TYPE_FLOAT; break;
            case DTA_117_TYPE_CODE_DOUBLE:
                len = 8; type = READSTAT_TYPE_DOUBLE; break;
            case DTA_117_TYPE_CODE_STRL:
                len = 8; type = READSTAT_TYPE_STRING_REF; break;
            default:
                len = typecode; type = READSTAT_TYPE_STRING; break;
        }
    } else {
        switch (typecode) {
            case DTA_OLD_TYPE_CODE_INT8:
                len = 1; type = READSTAT_TYPE_INT8; break;
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
