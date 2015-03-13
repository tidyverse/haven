#include <stdlib.h>
#include <math.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include "readstat_dta.h"
#include "readstat_io.h"

dta_ctx_t *dta_ctx_init(int16_t nvar, int32_t nobs, unsigned char byteorder, unsigned char ds_format) {
    dta_ctx_t *ctx;
    if ((ctx = malloc(sizeof(dta_ctx_t))) == NULL) {
        return NULL;
    }
    memset(ctx, 0, sizeof(dta_ctx_t));

    int machine_byteorder = DTA_HILO;
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
    } else {
        ctx->fmtlist_entry_len = 49;
    }
    
    if (ds_format >= 117) {
        ctx->typlist_version = 117;
    } else if (ds_format >= 111) {
        ctx->typlist_version = 111;
    } else {
        ctx->typlist_version = 0;
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
    } else {
        ctx->lbllist_entry_len = 33;
        ctx->variable_name_len = 33;
    }

    if (ds_format < 108) {
        ctx->variable_labels_entry_len = 32;
        ctx->data_label_len = 32;
    } else {
        ctx->variable_labels_entry_len = 81;
        ctx->data_label_len = 81;
    }

    if (ds_format < 105) {
        ctx->time_stamp_len = 0;
        ctx->value_label_table_len_len = 2;
    } else {
        ctx->time_stamp_len = 18;
        ctx->value_label_table_len_len = 4;
    }

    if (ds_format < 117) {
        ctx->typlist_entry_len = 1;
        ctx->file_is_xmlish = 0;
    } else {
        ctx->typlist_entry_len = 2;
        ctx->file_is_xmlish = 1;
    }

    ctx->typlist_len = ctx->nvar * sizeof(uint16_t);
    ctx->varlist_len = ctx->variable_name_len * ctx->nvar * sizeof(char);
    ctx->srtlist_len = (ctx->nvar + 1) * sizeof(int16_t);
    ctx->fmtlist_len = ctx->fmtlist_entry_len * ctx->nvar * sizeof(char);
    ctx->lbllist_len = ctx->lbllist_entry_len * ctx->nvar * sizeof(char);
    ctx->variable_labels_len = ctx->variable_labels_entry_len * ctx->nvar * sizeof(char);

    if ((ctx->typlist = malloc(ctx->typlist_len)) == NULL) {
        dta_ctx_free(ctx);
        return NULL;
    }
    if ((ctx->varlist = malloc(ctx->varlist_len)) == NULL) {
        dta_ctx_free(ctx);
        return NULL;
    }
    if ((ctx->srtlist = malloc(ctx->srtlist_len)) == NULL) {
        dta_ctx_free(ctx);
        return NULL;
    }
    if ((ctx->fmtlist = malloc(ctx->fmtlist_len)) == NULL) {
        dta_ctx_free(ctx);
        return NULL;
    }
    if ((ctx->lbllist = malloc(ctx->lbllist_len)) == NULL) {
        dta_ctx_free(ctx);
        return NULL;
    }

    if ((ctx->variable_labels = malloc(ctx->variable_labels_len)) == NULL) {
        dta_ctx_free(ctx);
        return NULL;
    }
    
    return ctx;
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
    free(ctx);
}
