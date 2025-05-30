#include <limits.h>
#include <stdlib.h>
#include "../readstat.h"
#include "../readstat_malloc.h"
#include "../readstat_strings.h"

#include "readstat_sav.h"
#include "readstat_sav_parse.h"

%%{
    machine key_defs;

    action copy_key {
        memcpy(temp_key, str_start, str_len);
        temp_key[str_len] = '\0';
    }
        
    non_ascii_byte = (0x80 .. 0xFE); # multi-byte sequence might be incomplete

    key = ( ( non_ascii_byte | [A-Z@] ) ( non_ascii_byte | [A-Za-z0-9@#$_\.] ){0,7} ) >{ str_start = fpc; } %{ str_len = fpc - str_start; };
}%%

typedef struct varlookup {
    char      name[8*4+1];
    int       index;
} varlookup_t;

static int compare_key_varlookup(const void *elem1, const void *elem2) {
    const char *key = (const char *)elem1;
    const varlookup_t *v = (const varlookup_t *)elem2;
    return strcasecmp(key, v->name);
}

static int compare_varlookups(const void *elem1, const void *elem2) {
    const varlookup_t *v1 = (const varlookup_t *)elem1;
    const varlookup_t *v2 = (const varlookup_t *)elem2;
    return strcasecmp(v1->name, v2->name);
}

static int count_vars(sav_ctx_t *ctx) {
    int i;
    spss_varinfo_t *last_info = NULL;
    int var_count = 0;
    for (i=0; i<ctx->var_index; i++) {
        spss_varinfo_t *info = ctx->varinfo[i];
        if (last_info == NULL || strcmp(info->name, last_info->name) != 0) {
            var_count++;
        }
        last_info = info;
    }
    return var_count;
}

static varlookup_t *build_lookup_table(int var_count, sav_ctx_t *ctx) {
    varlookup_t *table = readstat_malloc(var_count * sizeof(varlookup_t));
    int offset = 0;
    int i;
    spss_varinfo_t *last_info = NULL;
    for (i=0; i<ctx->var_index; i++) {
        spss_varinfo_t *info = ctx->varinfo[i];

        if (last_info == NULL || strcmp(info->name, last_info->name) != 0) {
            varlookup_t *entry = &table[offset++];

            memcpy(entry->name, info->name, sizeof(info->name));
            entry->index = info->index;
        }
        last_info = info;
    }
    qsort(table, var_count, sizeof(varlookup_t), &compare_varlookups);
    return table;
}

%%{
    machine sav_long_variable_parse;
    include key_defs;
    write data nofinal noerror;
    alphtype unsigned char;
}%%

readstat_error_t sav_parse_long_variable_names_record(void *data, int count, sav_ctx_t *ctx) {
    unsigned char *c_data = (unsigned char *)data;
    int var_count = count_vars(ctx);
    readstat_error_t retval = READSTAT_OK;

    char temp_key[8+1];
    char temp_val[64+1];
    unsigned char *str_start = NULL;
    size_t str_len = 0;
    
    char error_buf[8192];
    unsigned char *p = c_data;
    unsigned char *pe = c_data + count;

    varlookup_t *table = build_lookup_table(var_count, ctx);

    unsigned char *eof = pe;

    int cs;

    %%{
        action set_long_name {
            varlookup_t *found = bsearch(temp_key, table, var_count, sizeof(varlookup_t), &compare_key_varlookup);
            if (!found) {
                snprintf(error_buf, sizeof(error_buf), "Failed to find %s", temp_key);
                if (ctx->handle.error)
                    ctx->handle.error(error_buf, ctx->user_ctx);
            } else {
                // Handle the edge case where a ghost variable name (from a multi-segment
                // variable) is identical to a real variable name. Normally we handle this
                // by incrementing the loop variable by n_segments, but n_segments hasn't
                // been set when this record is processed. So just set the longname to every
                // matching variable, ghost or real.
                varlookup_t *iter_match = found;
                while (iter_match >= table && strcmp(iter_match->name, temp_key) == 0) {
                    spss_varinfo_t *info = ctx->varinfo[iter_match->index];
                    snprintf(info->longname, sizeof(info->longname), "%*s", (int)str_len, temp_val);
                    iter_match--;
                }
                iter_match = found + 1;
                while (iter_match - table < var_count && strcmp(iter_match->name, temp_key) == 0) {
                    spss_varinfo_t *info = ctx->varinfo[iter_match->index];
                    snprintf(info->longname, sizeof(info->longname), "%*s", (int)str_len, temp_val);
                    iter_match++;
                }
            }
        }

        action copy_value {
            memcpy(temp_val, str_start, str_len);
            temp_val[str_len] = '\0';
        }

        value = ( non_ascii_byte | print ){1,64} >{ str_start = fpc; } %{ str_len = fpc - str_start; };
        
        keyval = ( key %copy_key "=" value %copy_value ) %set_long_name;
        
        main := keyval ("\t" keyval)*  "\t"?;
        
        write init;
        write exec;
    }%%

    if (cs < %%{ write first_final; }%%|| p != pe) {
        if (ctx->handle.error) {
            snprintf(error_buf, sizeof(error_buf), "Error parsing string \"%.*s\" around byte #%ld/%d, character %c", 
                    count, (char *)data, (long)(p - c_data), count, *p);
            ctx->handle.error(error_buf, ctx->user_ctx);
        }
        retval = READSTAT_ERROR_PARSE;
    }
    

    if (table)
        free(table);

    /* suppress warning */
    (void)sav_long_variable_parse_en_main;

    return retval;
}

%%{
    machine sav_very_long_string_parse;
    include key_defs;
    write data nofinal noerror;
    alphtype unsigned char;
}%%

readstat_error_t sav_parse_very_long_string_record(void *data, int count, sav_ctx_t *ctx) {
    unsigned char *c_data = (unsigned char *)data;
    int var_count = count_vars(ctx);
    readstat_error_t retval = READSTAT_OK;

    char temp_key[8*4+1];
    unsigned int temp_val = 0;
    unsigned char *str_start = NULL;
    size_t str_len = 0;

    size_t error_buf_len = 1024 + count;
    char *error_buf = NULL;
    unsigned char *p = c_data;
    unsigned char *pe = c_data + count;
    unsigned char *eof = pe;

    varlookup_t *table = NULL;
    int cs;

    error_buf = readstat_malloc(error_buf_len);
    table = build_lookup_table(var_count, ctx);
    
    %%{
        action set_width {
            varlookup_t *found = bsearch(temp_key, table, var_count, sizeof(varlookup_t), &compare_key_varlookup);
            if (found) {
                // See logic above; we need to apply this to all matching variables since ghost variable
                // names may conflict with real variable names.
                varlookup_t *first_match = found, *last_match = found;
                varlookup_t *iter_match = found - 1;
                while (iter_match >= table && strcmp(iter_match->name, temp_key) == 0) {
                    first_match = iter_match;
                    iter_match--;
                }
                iter_match = found + 1;
                while (iter_match - table < var_count && strcmp(iter_match->name, temp_key) == 0) {
                    last_match = iter_match;
                    iter_match++;
                }
                for (iter_match=first_match; iter_match<=last_match; iter_match++) {
                    ctx->varinfo[iter_match->index]->string_length = temp_val;
                    ctx->varinfo[iter_match->index]->write_format.width = temp_val;
                    ctx->varinfo[iter_match->index]->print_format.width = temp_val;
                }
            }
        }

        action incr_val {
            if (fc != '\0') {
                unsigned char digit = fc - '0';
                if (temp_val <= (UINT_MAX - digit) / 10) {
                    temp_val = 10 * temp_val + digit;
                } else {
                    fbreak;
                }
            }
        }
        
        value = [0-9]+ >{ temp_val = 0; } $incr_val;
        
        keyval = ( key %copy_key "=" value ) %set_width;
        
        main := keyval ("\0"* "\t" keyval)* "\0"* "\t"?;
        
        write init;
        write exec;
    }%%
    
    if (cs < %%{ write first_final; }%% || p != pe) {
        if (ctx->handle.error) {
            snprintf(error_buf, error_buf_len, "Parsed %ld of %ld bytes. Remaining bytes: %.*s",
                    (long)(p - c_data), (long)(pe - c_data), (int)(pe - p), p);
            ctx->handle.error(error_buf, ctx->user_ctx);
        }
        retval = READSTAT_ERROR_PARSE;
    }
    
    if (table)
        free(table);
    if (error_buf)
        free(error_buf);

    /* suppress warning */
    (void)sav_very_long_string_parse_en_main;

    return retval;
}
