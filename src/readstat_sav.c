//
//  sav.c
//  Wizard
//
//  Created by Evan Miller on 12/16/11.
//  Copyright (c) 2011 __MyCompanyName__. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <stdint.h>
#include <math.h>
#include <float.h>
#include <time.h>

#include "readstat_sav.h"

#define SAV_VARINFO_INITIAL_CAPACITY  512

sav_ctx_t *sav_ctx_init(sav_file_header_record_t *header, readstat_io_t *io) {
    sav_ctx_t *ctx = NULL;
    if ((ctx = malloc(sizeof(sav_ctx_t))) == NULL) {
        return NULL;
    }
    memset(ctx, 0, sizeof(sav_ctx_t));
    
    if (header->layout_code == 2 || header->layout_code == 3) {
        ctx->machine_needs_byte_swap = 0;
    } else {
        ctx->machine_needs_byte_swap = 1;
    }
    
    ctx->data_is_compressed = (header->compressed != 0);
    ctx->record_count = ctx->machine_needs_byte_swap ? byteswap4(header->ncases) : header->ncases;
    ctx->fweight_index = ctx->machine_needs_byte_swap ? byteswap4(header->weight_index) : header->weight_index;
    
    double bias = ctx->machine_needs_byte_swap ? byteswap_double(header->bias) : header->bias;
    
    if (bias != 100.0) {
        sav_ctx_free(ctx);
        return NULL;
    }
    
    ctx->varinfo_capacity = SAV_VARINFO_INITIAL_CAPACITY;
    
    if ((ctx->varinfo = calloc(ctx->varinfo_capacity, sizeof(spss_varinfo_t))) == NULL) {
        sav_ctx_free(ctx);
        return NULL;
    }

    ctx->io = io;
    
    return ctx;
}

void sav_ctx_free(sav_ctx_t *ctx) {
    if (ctx->varinfo) {
        int i;
        for (i=0; i<ctx->var_count; i++) {
            if (ctx->varinfo[i].label)
                free(ctx->varinfo[i].label);
        }
        free(ctx->varinfo);
    }
    if (ctx->converter) {
        iconv_close(ctx->converter);
    }
    free(ctx);
}

