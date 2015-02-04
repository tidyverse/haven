
#include <stdlib.h>
#include <unistd.h>
#include <iconv.h>
#include "readstat_sav.h"
#include "readstat_sav_parse.h"

typedef struct varlookup {
    char      name[9];
    int       index;
} varlookup_t;

static int compare_key_varlookup(const void *elem1, const void *elem2) {
    const char *key = (const char *)elem1;
    const varlookup_t *v = (const varlookup_t *)elem2;
    return strcmp(key, v->name);
}

static int compare_varlookups(const void *elem1, const void *elem2) {
    const varlookup_t *v1 = (const varlookup_t *)elem1;
    const varlookup_t *v2 = (const varlookup_t *)elem2;
    return strcmp(v1->name, v2->name);
}

%%{
    machine sav_long_variable_parse;
    write data;
    alphtype unsigned char;
}%%

readstat_error_t sav_parse_long_variable_names_record(void *data, int count, sav_ctx_t *ctx) {
    varlookup_t *table = malloc(ctx->var_index * sizeof(varlookup_t));
    int i;
    readstat_error_t retval = READSTAT_OK;
    int var_count = ctx->var_index;
    for (i=0; i<var_count; i++) {
        memcpy(table[i].name, ctx->varinfo[i].name, 9);
        table[i].index = ctx->varinfo[i].index;
    }
    qsort(table, var_count, sizeof(varlookup_t), &compare_varlookups);
        
    char temp_key[4*8+1];
    char temp_val[4*64+1];
    u_char *str_start = NULL;
    size_t str_len = 0;
    
    u_char *p = NULL;
    u_char *pe = NULL;
    u_char *output_buffer = NULL;
    if (ctx->converter) {
        size_t input_len = count;
        size_t output_len = input_len * 4;
        pe = p = output_buffer = malloc(output_len);
        size_t status = iconv(ctx->converter, (char **)&data, &input_len,
                (char **)&pe, &output_len);
        if (status == (size_t)-1) {
            free(output_buffer);
            return READSTAT_ERROR_PARSE;
        }
    } else {
        p = (u_char *)data;
        pe = (u_char *)data + count;
    }
    u_char *eof = pe;

    int cs;

    %%{
        action set_long_name {
            varlookup_t *found = bsearch(temp_key, table, var_count, sizeof(varlookup_t), &compare_key_varlookup);
            if (found) {
                memcpy(ctx->varinfo[found->index].longname, temp_val, str_len);
            } else {
                dprintf(STDERR_FILENO, "Failed to find %s\n", temp_key);
            }
        }

        action copy_key {
            memcpy(temp_key, str_start, str_len);
            temp_key[str_len] = '\0';
        }

        action copy_value {
            memcpy(temp_val, str_start, str_len);
            temp_val[str_len] = '\0';
        }

        non_ascii_character = ( # UTF-8 byte sequences
                0xC0..0xDF 0x80..0xBF | 
                0xE0..0xEF (0x80..0xBF){2} |
                0xF0..0xF7 (0x80..0xBF){3}
                );
        
        key = ( ( non_ascii_character | [A-Z@] ) ( non_ascii_character | [A-Z0-9@#$_\.] ){0,7} ) >{ str_start = fpc; } %{ str_len = fpc - str_start; };
        
        value = ( non_ascii_character | print ){1,64} >{ str_start = fpc; } %{ str_len = fpc - str_start; };
        
        keyval = ( key %copy_key "=" value %copy_value ) %set_long_name;
        
        main := keyval ("\t" keyval)*  "\t"?;
        
        write init;
        write exec;
    }%%

    if (cs < %%{ write first_final; }%%|| p != pe) {
        dprintf(STDERR_FILENO, "Error parsing string \"%s\" around byte #%ld/%d, character %c\n", 
                (char *)data, p - (u_char *)data, count, *p);
        retval = READSTAT_ERROR_PARSE;
    }
    
    if (table)
        free(table);
    if (output_buffer)
        free(output_buffer);
    return retval;
}

%%{
    machine sav_very_long_string_parse;
    write data;
    alphtype unsigned char;
}%%

readstat_error_t sav_parse_very_long_string_record(void *data, int count, sav_ctx_t *ctx) {
    varlookup_t *table = malloc(ctx->var_index * sizeof(varlookup_t));
    int i;
    readstat_error_t retval = READSTAT_OK;
    int var_count = ctx->var_index;
    for (i=0; i<var_count; i++) {
        memcpy(table[i].name, ctx->varinfo[i].name, 9);
        table[i].index = ctx->varinfo[i].index;
    }
    qsort(table, var_count, sizeof(varlookup_t), &compare_varlookups);
    
    char temp_key[8*4+1];
    int temp_val;
    u_char *str_start = NULL;
    size_t str_len = 0;

    u_char *p = NULL;
    u_char *pe = NULL;

    u_char *output_buffer = NULL;
    if (ctx->converter) {
        size_t input_len = count;
        size_t output_len = input_len * 4;

        pe = p = output_buffer = malloc(output_len);

        size_t status = iconv(ctx->converter, (char **)&data, &input_len,
                (char **)&pe, &output_len);
        if (status == (size_t)-1) {
            free(output_buffer);
            return READSTAT_ERROR_PARSE;
        }
    } else {
        p = (u_char *)data;
        pe = (u_char *)data + count;
    }
    
    int cs;
    
    %%{
        action set_width {
            varlookup_t *found = bsearch(temp_key, table, var_count, sizeof(varlookup_t), &compare_key_varlookup);
            if (found) {
                ctx->varinfo[found->index].string_length = temp_val;
            }
        }

        action copy_key {
            memcpy(temp_key, str_start, str_len);
            temp_key[str_len] = '\0';
        }
        
        action incr_val {
            if (fc != '\0') { 
                temp_val = 10 * temp_val + (fc - '0'); 
            }
        }
        
        non_ascii_character = ( # UTF-8 byte sequences
                0xC0..0xDF 0x80..0xBF | 
                0xE0..0xEF (0x80..0xBF){2} |
                0xF0..0xF7 (0x80..0xBF){3}
                );

        key = ( ( non_ascii_character | [A-Z@] ) ( non_ascii_character | [A-Z0-9@#$_\.] ){0,7} ) >{ str_start = fpc; } %{ str_len = fpc - str_start; };
        
        value = [0-9]+ >{ temp_val = 0; } $incr_val;
        
        keyval = ( key %copy_key "=" value ) %set_width;
        
        main := keyval ("\0"+ "\t" keyval)* "\0"+ "\t"?;
        
        write init;
        write exec;
    }%%
    
    if (cs < %%{ write first_final; }%% || p != pe) {
        dprintf(STDERR_FILENO, "Parsed %ld of %ld bytes\n", p - (u_char *)data, pe - (u_char *)data);
        dprintf(STDERR_FILENO, "Remaining bytes: %s\n", p);
        retval = READSTAT_ERROR_PARSE;
    }
    
    if (table)
        free(table);
    if (output_buffer)
        free(output_buffer);
    return retval;
}
