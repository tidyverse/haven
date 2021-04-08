
#line 1 "src/spss/readstat_sav_parse.rl"
#include <limits.h>
#include <stdlib.h>
#include "../readstat.h"
#include "../readstat_malloc.h"
#include "../readstat_strings.h"

#include "readstat_sav.h"
#include "readstat_sav_parse.h"


#line 21 "src/spss/readstat_sav_parse.rl"


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


#line 69 "src/spss/readstat_sav_parse.c"
static const char _sav_long_variable_parse_actions[] = {
	0, 1, 1, 1, 5, 2, 2, 0, 
	3, 6, 4, 3
};

static const short _sav_long_variable_parse_key_offsets[] = {
	0, 0, 5, 19, 33, 47, 61, 75, 
	89, 103, 104, 108, 113, 118, 123, 128, 
	133, 138, 143, 148, 153, 158, 163, 168, 
	173, 178, 183, 188, 193, 198, 203, 208, 
	213, 218, 223, 228, 233, 238, 243, 248, 
	253, 258, 263, 268, 273, 278, 283, 288, 
	293, 298, 303, 308, 313, 318, 323, 328, 
	333, 338, 343, 348, 353, 358, 363, 368, 
	373, 378, 383, 388, 393, 398, 403, 408, 
	413, 418, 423, 428
};

static const unsigned char _sav_long_variable_parse_trans_keys[] = {
	255u, 0u, 63u, 91u, 127u, 47u, 61u, 96u, 
	255u, 0u, 34u, 37u, 45u, 58u, 63u, 91u, 
	94u, 123u, 127u, 47u, 61u, 96u, 255u, 0u, 
	34u, 37u, 45u, 58u, 63u, 91u, 94u, 123u, 
	127u, 47u, 61u, 96u, 255u, 0u, 34u, 37u, 
	45u, 58u, 63u, 91u, 94u, 123u, 127u, 47u, 
	61u, 96u, 255u, 0u, 34u, 37u, 45u, 58u, 
	63u, 91u, 94u, 123u, 127u, 47u, 61u, 96u, 
	255u, 0u, 34u, 37u, 45u, 58u, 63u, 91u, 
	94u, 123u, 127u, 47u, 61u, 96u, 255u, 0u, 
	34u, 37u, 45u, 58u, 63u, 91u, 94u, 123u, 
	127u, 47u, 61u, 96u, 255u, 0u, 34u, 37u, 
	45u, 58u, 63u, 91u, 94u, 123u, 127u, 61u, 
	127u, 255u, 0u, 31u, 9u, 127u, 255u, 0u, 
	31u, 255u, 0u, 63u, 91u, 127u, 9u, 127u, 
	255u, 0u, 31u, 9u, 127u, 255u, 0u, 31u, 
	9u, 127u, 255u, 0u, 31u, 9u, 127u, 255u, 
	0u, 31u, 9u, 127u, 255u, 0u, 31u, 9u, 
	127u, 255u, 0u, 31u, 9u, 127u, 255u, 0u, 
	31u, 9u, 127u, 255u, 0u, 31u, 9u, 127u, 
	255u, 0u, 31u, 9u, 127u, 255u, 0u, 31u, 
	9u, 127u, 255u, 0u, 31u, 9u, 127u, 255u, 
	0u, 31u, 9u, 127u, 255u, 0u, 31u, 9u, 
	127u, 255u, 0u, 31u, 9u, 127u, 255u, 0u, 
	31u, 9u, 127u, 255u, 0u, 31u, 9u, 127u, 
	255u, 0u, 31u, 9u, 127u, 255u, 0u, 31u, 
	9u, 127u, 255u, 0u, 31u, 9u, 127u, 255u, 
	0u, 31u, 9u, 127u, 255u, 0u, 31u, 9u, 
	127u, 255u, 0u, 31u, 9u, 127u, 255u, 0u, 
	31u, 9u, 127u, 255u, 0u, 31u, 9u, 127u, 
	255u, 0u, 31u, 9u, 127u, 255u, 0u, 31u, 
	9u, 127u, 255u, 0u, 31u, 9u, 127u, 255u, 
	0u, 31u, 9u, 127u, 255u, 0u, 31u, 9u, 
	127u, 255u, 0u, 31u, 9u, 127u, 255u, 0u, 
	31u, 9u, 127u, 255u, 0u, 31u, 9u, 127u, 
	255u, 0u, 31u, 9u, 127u, 255u, 0u, 31u, 
	9u, 127u, 255u, 0u, 31u, 9u, 127u, 255u, 
	0u, 31u, 9u, 127u, 255u, 0u, 31u, 9u, 
	127u, 255u, 0u, 31u, 9u, 127u, 255u, 0u, 
	31u, 9u, 127u, 255u, 0u, 31u, 9u, 127u, 
	255u, 0u, 31u, 9u, 127u, 255u, 0u, 31u, 
	9u, 127u, 255u, 0u, 31u, 9u, 127u, 255u, 
	0u, 31u, 9u, 127u, 255u, 0u, 31u, 9u, 
	127u, 255u, 0u, 31u, 9u, 127u, 255u, 0u, 
	31u, 9u, 127u, 255u, 0u, 31u, 9u, 127u, 
	255u, 0u, 31u, 9u, 127u, 255u, 0u, 31u, 
	9u, 127u, 255u, 0u, 31u, 9u, 127u, 255u, 
	0u, 31u, 9u, 127u, 255u, 0u, 31u, 9u, 
	127u, 255u, 0u, 31u, 9u, 127u, 255u, 0u, 
	31u, 9u, 127u, 255u, 0u, 31u, 9u, 127u, 
	255u, 0u, 31u, 9u, 127u, 255u, 0u, 31u, 
	9u, 127u, 255u, 0u, 31u, 9u, 127u, 255u, 
	0u, 31u, 9u, 127u, 255u, 0u, 31u, 9u, 
	127u, 255u, 0u, 31u, 9u, 0
};

static const char _sav_long_variable_parse_single_lengths[] = {
	0, 1, 4, 4, 4, 4, 4, 4, 
	4, 1, 2, 3, 1, 3, 3, 3, 
	3, 3, 3, 3, 3, 3, 3, 3, 
	3, 3, 3, 3, 3, 3, 3, 3, 
	3, 3, 3, 3, 3, 3, 3, 3, 
	3, 3, 3, 3, 3, 3, 3, 3, 
	3, 3, 3, 3, 3, 3, 3, 3, 
	3, 3, 3, 3, 3, 3, 3, 3, 
	3, 3, 3, 3, 3, 3, 3, 3, 
	3, 3, 3, 1
};

static const char _sav_long_variable_parse_range_lengths[] = {
	0, 2, 5, 5, 5, 5, 5, 5, 
	5, 0, 1, 1, 2, 1, 1, 1, 
	1, 1, 1, 1, 1, 1, 1, 1, 
	1, 1, 1, 1, 1, 1, 1, 1, 
	1, 1, 1, 1, 1, 1, 1, 1, 
	1, 1, 1, 1, 1, 1, 1, 1, 
	1, 1, 1, 1, 1, 1, 1, 1, 
	1, 1, 1, 1, 1, 1, 1, 1, 
	1, 1, 1, 1, 1, 1, 1, 1, 
	1, 1, 1, 0
};

static const short _sav_long_variable_parse_index_offsets[] = {
	0, 0, 4, 14, 24, 34, 44, 54, 
	64, 74, 76, 80, 85, 89, 94, 99, 
	104, 109, 114, 119, 124, 129, 134, 139, 
	144, 149, 154, 159, 164, 169, 174, 179, 
	184, 189, 194, 199, 204, 209, 214, 219, 
	224, 229, 234, 239, 244, 249, 254, 259, 
	264, 269, 274, 279, 284, 289, 294, 299, 
	304, 309, 314, 319, 324, 329, 334, 339, 
	344, 349, 354, 359, 364, 369, 374, 379, 
	384, 389, 394, 399
};

static const char _sav_long_variable_parse_indicies[] = {
	1, 1, 1, 0, 1, 3, 1, 1, 
	1, 1, 1, 1, 1, 2, 1, 3, 
	1, 1, 1, 1, 1, 1, 1, 4, 
	1, 3, 1, 1, 1, 1, 1, 1, 
	1, 5, 1, 3, 1, 1, 1, 1, 
	1, 1, 1, 6, 1, 3, 1, 1, 
	1, 1, 1, 1, 1, 7, 1, 3, 
	1, 1, 1, 1, 1, 1, 1, 8, 
	1, 3, 1, 1, 1, 1, 1, 1, 
	1, 9, 3, 1, 1, 1, 1, 10, 
	11, 1, 1, 1, 12, 1, 1, 1, 
	0, 11, 1, 1, 1, 13, 11, 1, 
	1, 1, 14, 11, 1, 1, 1, 15, 
	11, 1, 1, 1, 16, 11, 1, 1, 
	1, 17, 11, 1, 1, 1, 18, 11, 
	1, 1, 1, 19, 11, 1, 1, 1, 
	20, 11, 1, 1, 1, 21, 11, 1, 
	1, 1, 22, 11, 1, 1, 1, 23, 
	11, 1, 1, 1, 24, 11, 1, 1, 
	1, 25, 11, 1, 1, 1, 26, 11, 
	1, 1, 1, 27, 11, 1, 1, 1, 
	28, 11, 1, 1, 1, 29, 11, 1, 
	1, 1, 30, 11, 1, 1, 1, 31, 
	11, 1, 1, 1, 32, 11, 1, 1, 
	1, 33, 11, 1, 1, 1, 34, 11, 
	1, 1, 1, 35, 11, 1, 1, 1, 
	36, 11, 1, 1, 1, 37, 11, 1, 
	1, 1, 38, 11, 1, 1, 1, 39, 
	11, 1, 1, 1, 40, 11, 1, 1, 
	1, 41, 11, 1, 1, 1, 42, 11, 
	1, 1, 1, 43, 11, 1, 1, 1, 
	44, 11, 1, 1, 1, 45, 11, 1, 
	1, 1, 46, 11, 1, 1, 1, 47, 
	11, 1, 1, 1, 48, 11, 1, 1, 
	1, 49, 11, 1, 1, 1, 50, 11, 
	1, 1, 1, 51, 11, 1, 1, 1, 
	52, 11, 1, 1, 1, 53, 11, 1, 
	1, 1, 54, 11, 1, 1, 1, 55, 
	11, 1, 1, 1, 56, 11, 1, 1, 
	1, 57, 11, 1, 1, 1, 58, 11, 
	1, 1, 1, 59, 11, 1, 1, 1, 
	60, 11, 1, 1, 1, 61, 11, 1, 
	1, 1, 62, 11, 1, 1, 1, 63, 
	11, 1, 1, 1, 64, 11, 1, 1, 
	1, 65, 11, 1, 1, 1, 66, 11, 
	1, 1, 1, 67, 11, 1, 1, 1, 
	68, 11, 1, 1, 1, 69, 11, 1, 
	1, 1, 70, 11, 1, 1, 1, 71, 
	11, 1, 1, 1, 72, 11, 1, 1, 
	1, 73, 11, 1, 1, 1, 74, 11, 
	1, 0
};

static const char _sav_long_variable_parse_trans_targs[] = {
	2, 0, 3, 10, 4, 5, 6, 7, 
	8, 9, 11, 12, 13, 14, 15, 16, 
	17, 18, 19, 20, 21, 22, 23, 24, 
	25, 26, 27, 28, 29, 30, 31, 32, 
	33, 34, 35, 36, 37, 38, 39, 40, 
	41, 42, 43, 44, 45, 46, 47, 48, 
	49, 50, 51, 52, 53, 54, 55, 56, 
	57, 58, 59, 60, 61, 62, 63, 64, 
	65, 66, 67, 68, 69, 70, 71, 72, 
	73, 74, 75
};

static const char _sav_long_variable_parse_trans_actions[] = {
	1, 0, 0, 5, 0, 0, 0, 0, 
	0, 0, 3, 8, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0
};

static const char _sav_long_variable_parse_eof_actions[] = {
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 8, 0, 8, 8, 8, 
	8, 8, 8, 8, 8, 8, 8, 8, 
	8, 8, 8, 8, 8, 8, 8, 8, 
	8, 8, 8, 8, 8, 8, 8, 8, 
	8, 8, 8, 8, 8, 8, 8, 8, 
	8, 8, 8, 8, 8, 8, 8, 8, 
	8, 8, 8, 8, 8, 8, 8, 8, 
	8, 8, 8, 8, 8, 8, 8, 8, 
	8, 8, 8, 8
};

static const int sav_long_variable_parse_start = 1;

static const int sav_long_variable_parse_en_main = 1;


#line 79 "src/spss/readstat_sav_parse.rl"


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

    
#line 306 "src/spss/readstat_sav_parse.c"
	{
	cs = sav_long_variable_parse_start;
	}

#line 311 "src/spss/readstat_sav_parse.c"
	{
	int _klen;
	unsigned int _trans;
	const char *_acts;
	unsigned int _nacts;
	const unsigned char *_keys;

	if ( p == pe )
		goto _test_eof;
	if ( cs == 0 )
		goto _out;
_resume:
	_keys = _sav_long_variable_parse_trans_keys + _sav_long_variable_parse_key_offsets[cs];
	_trans = _sav_long_variable_parse_index_offsets[cs];

	_klen = _sav_long_variable_parse_single_lengths[cs];
	if ( _klen > 0 ) {
		const unsigned char *_lower = _keys;
		const unsigned char *_mid;
		const unsigned char *_upper = _keys + _klen - 1;
		while (1) {
			if ( _upper < _lower )
				break;

			_mid = _lower + ((_upper-_lower) >> 1);
			if ( (*p) < *_mid )
				_upper = _mid - 1;
			else if ( (*p) > *_mid )
				_lower = _mid + 1;
			else {
				_trans += (unsigned int)(_mid - _keys);
				goto _match;
			}
		}
		_keys += _klen;
		_trans += _klen;
	}

	_klen = _sav_long_variable_parse_range_lengths[cs];
	if ( _klen > 0 ) {
		const unsigned char *_lower = _keys;
		const unsigned char *_mid;
		const unsigned char *_upper = _keys + (_klen<<1) - 2;
		while (1) {
			if ( _upper < _lower )
				break;

			_mid = _lower + (((_upper-_lower) >> 1) & ~1);
			if ( (*p) < _mid[0] )
				_upper = _mid - 2;
			else if ( (*p) > _mid[1] )
				_lower = _mid + 2;
			else {
				_trans += (unsigned int)((_mid - _keys)>>1);
				goto _match;
			}
		}
		_trans += _klen;
	}

_match:
	_trans = _sav_long_variable_parse_indicies[_trans];
	cs = _sav_long_variable_parse_trans_targs[_trans];

	if ( _sav_long_variable_parse_trans_actions[_trans] == 0 )
		goto _again;

	_acts = _sav_long_variable_parse_actions + _sav_long_variable_parse_trans_actions[_trans];
	_nacts = (unsigned int) *_acts++;
	while ( _nacts-- > 0 )
	{
		switch ( *_acts++ )
		{
	case 0:
#line 13 "src/spss/readstat_sav_parse.rl"
	{
        memcpy(temp_key, str_start, str_len);
        temp_key[str_len] = '\0';
    }
	break;
	case 1:
#line 20 "src/spss/readstat_sav_parse.rl"
	{ str_start = p; }
	break;
	case 2:
#line 20 "src/spss/readstat_sav_parse.rl"
	{ str_len = p - str_start; }
	break;
	case 3:
#line 102 "src/spss/readstat_sav_parse.rl"
	{
            varlookup_t *found = bsearch(temp_key, table, var_count, sizeof(varlookup_t), &compare_key_varlookup);
            if (found) {
                spss_varinfo_t *info = ctx->varinfo[found->index];
                memcpy(info->longname, temp_val, str_len);
                info->longname[str_len] = '\0';
            } else if (ctx->handle.error) {
                snprintf(error_buf, sizeof(error_buf), "Failed to find %s", temp_key);
                ctx->handle.error(error_buf, ctx->user_ctx);
            }
        }
	break;
	case 4:
#line 114 "src/spss/readstat_sav_parse.rl"
	{
            memcpy(temp_val, str_start, str_len);
            temp_val[str_len] = '\0';
        }
	break;
	case 5:
#line 119 "src/spss/readstat_sav_parse.rl"
	{ str_start = p; }
	break;
	case 6:
#line 119 "src/spss/readstat_sav_parse.rl"
	{ str_len = p - str_start; }
	break;
#line 429 "src/spss/readstat_sav_parse.c"
		}
	}

_again:
	if ( cs == 0 )
		goto _out;
	if ( ++p != pe )
		goto _resume;
	_test_eof: {}
	if ( p == eof )
	{
	const char *__acts = _sav_long_variable_parse_actions + _sav_long_variable_parse_eof_actions[cs];
	unsigned int __nacts = (unsigned int) *__acts++;
	while ( __nacts-- > 0 ) {
		switch ( *__acts++ ) {
	case 3:
#line 102 "src/spss/readstat_sav_parse.rl"
	{
            varlookup_t *found = bsearch(temp_key, table, var_count, sizeof(varlookup_t), &compare_key_varlookup);
            if (found) {
                spss_varinfo_t *info = ctx->varinfo[found->index];
                memcpy(info->longname, temp_val, str_len);
                info->longname[str_len] = '\0';
            } else if (ctx->handle.error) {
                snprintf(error_buf, sizeof(error_buf), "Failed to find %s", temp_key);
                ctx->handle.error(error_buf, ctx->user_ctx);
            }
        }
	break;
	case 4:
#line 114 "src/spss/readstat_sav_parse.rl"
	{
            memcpy(temp_val, str_start, str_len);
            temp_val[str_len] = '\0';
        }
	break;
	case 6:
#line 119 "src/spss/readstat_sav_parse.rl"
	{ str_len = p - str_start; }
	break;
#line 470 "src/spss/readstat_sav_parse.c"
		}
	}
	}

	_out: {}
	}

#line 127 "src/spss/readstat_sav_parse.rl"


    if (cs < 11|| p != pe) {
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


#line 501 "src/spss/readstat_sav_parse.c"
static const char _sav_very_long_string_parse_actions[] = {
	0, 1, 1, 1, 3, 1, 4, 2, 
	2, 0, 2, 5, 4
};

static const char _sav_very_long_string_parse_key_offsets[] = {
	0, 0, 5, 19, 33, 47, 61, 75, 
	89, 103, 104, 106, 109, 111
};

static const unsigned char _sav_very_long_string_parse_trans_keys[] = {
	255u, 0u, 63u, 91u, 127u, 47u, 61u, 96u, 
	255u, 0u, 34u, 37u, 45u, 58u, 63u, 91u, 
	94u, 123u, 127u, 47u, 61u, 96u, 255u, 0u, 
	34u, 37u, 45u, 58u, 63u, 91u, 94u, 123u, 
	127u, 47u, 61u, 96u, 255u, 0u, 34u, 37u, 
	45u, 58u, 63u, 91u, 94u, 123u, 127u, 47u, 
	61u, 96u, 255u, 0u, 34u, 37u, 45u, 58u, 
	63u, 91u, 94u, 123u, 127u, 47u, 61u, 96u, 
	255u, 0u, 34u, 37u, 45u, 58u, 63u, 91u, 
	94u, 123u, 127u, 47u, 61u, 96u, 255u, 0u, 
	34u, 37u, 45u, 58u, 63u, 91u, 94u, 123u, 
	127u, 47u, 61u, 96u, 255u, 0u, 34u, 37u, 
	45u, 58u, 63u, 91u, 94u, 123u, 127u, 61u, 
	48u, 57u, 0u, 48u, 57u, 0u, 9u, 255u, 
	0u, 63u, 91u, 127u, 0
};

static const char _sav_very_long_string_parse_single_lengths[] = {
	0, 1, 4, 4, 4, 4, 4, 4, 
	4, 1, 0, 1, 2, 1
};

static const char _sav_very_long_string_parse_range_lengths[] = {
	0, 2, 5, 5, 5, 5, 5, 5, 
	5, 0, 1, 1, 0, 2
};

static const char _sav_very_long_string_parse_index_offsets[] = {
	0, 0, 4, 14, 24, 34, 44, 54, 
	64, 74, 76, 78, 81, 84
};

static const char _sav_very_long_string_parse_indicies[] = {
	1, 1, 1, 0, 1, 3, 1, 1, 
	1, 1, 1, 1, 1, 2, 1, 3, 
	1, 1, 1, 1, 1, 1, 1, 4, 
	1, 3, 1, 1, 1, 1, 1, 1, 
	1, 5, 1, 3, 1, 1, 1, 1, 
	1, 1, 1, 6, 1, 3, 1, 1, 
	1, 1, 1, 1, 1, 7, 1, 3, 
	1, 1, 1, 1, 1, 1, 1, 8, 
	1, 3, 1, 1, 1, 1, 1, 1, 
	1, 9, 3, 1, 10, 1, 11, 12, 
	1, 13, 14, 1, 1, 1, 1, 0, 
	0
};

static const char _sav_very_long_string_parse_trans_targs[] = {
	2, 0, 3, 10, 4, 5, 6, 7, 
	8, 9, 11, 12, 11, 12, 13
};

static const char _sav_very_long_string_parse_trans_actions[] = {
	1, 0, 0, 7, 0, 0, 0, 0, 
	0, 0, 10, 3, 5, 0, 0
};

static const int sav_very_long_string_parse_start = 1;

static const int sav_very_long_string_parse_en_main = 1;


#line 153 "src/spss/readstat_sav_parse.rl"


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

    varlookup_t *table = NULL;
    int cs;

    error_buf = readstat_malloc(error_buf_len);
    table = build_lookup_table(var_count, ctx);
    
    
#line 600 "src/spss/readstat_sav_parse.c"
	{
	cs = sav_very_long_string_parse_start;
	}

#line 605 "src/spss/readstat_sav_parse.c"
	{
	int _klen;
	unsigned int _trans;
	const char *_acts;
	unsigned int _nacts;
	const unsigned char *_keys;

	if ( p == pe )
		goto _test_eof;
	if ( cs == 0 )
		goto _out;
_resume:
	_keys = _sav_very_long_string_parse_trans_keys + _sav_very_long_string_parse_key_offsets[cs];
	_trans = _sav_very_long_string_parse_index_offsets[cs];

	_klen = _sav_very_long_string_parse_single_lengths[cs];
	if ( _klen > 0 ) {
		const unsigned char *_lower = _keys;
		const unsigned char *_mid;
		const unsigned char *_upper = _keys + _klen - 1;
		while (1) {
			if ( _upper < _lower )
				break;

			_mid = _lower + ((_upper-_lower) >> 1);
			if ( (*p) < *_mid )
				_upper = _mid - 1;
			else if ( (*p) > *_mid )
				_lower = _mid + 1;
			else {
				_trans += (unsigned int)(_mid - _keys);
				goto _match;
			}
		}
		_keys += _klen;
		_trans += _klen;
	}

	_klen = _sav_very_long_string_parse_range_lengths[cs];
	if ( _klen > 0 ) {
		const unsigned char *_lower = _keys;
		const unsigned char *_mid;
		const unsigned char *_upper = _keys + (_klen<<1) - 2;
		while (1) {
			if ( _upper < _lower )
				break;

			_mid = _lower + (((_upper-_lower) >> 1) & ~1);
			if ( (*p) < _mid[0] )
				_upper = _mid - 2;
			else if ( (*p) > _mid[1] )
				_lower = _mid + 2;
			else {
				_trans += (unsigned int)((_mid - _keys)>>1);
				goto _match;
			}
		}
		_trans += _klen;
	}

_match:
	_trans = _sav_very_long_string_parse_indicies[_trans];
	cs = _sav_very_long_string_parse_trans_targs[_trans];

	if ( _sav_very_long_string_parse_trans_actions[_trans] == 0 )
		goto _again;

	_acts = _sav_very_long_string_parse_actions + _sav_very_long_string_parse_trans_actions[_trans];
	_nacts = (unsigned int) *_acts++;
	while ( _nacts-- > 0 )
	{
		switch ( *_acts++ )
		{
	case 0:
#line 13 "src/spss/readstat_sav_parse.rl"
	{
        memcpy(temp_key, str_start, str_len);
        temp_key[str_len] = '\0';
    }
	break;
	case 1:
#line 20 "src/spss/readstat_sav_parse.rl"
	{ str_start = p; }
	break;
	case 2:
#line 20 "src/spss/readstat_sav_parse.rl"
	{ str_len = p - str_start; }
	break;
	case 3:
#line 177 "src/spss/readstat_sav_parse.rl"
	{
            varlookup_t *found = bsearch(temp_key, table, var_count, sizeof(varlookup_t), &compare_key_varlookup);
            if (found) {
                ctx->varinfo[found->index]->string_length = temp_val;
                ctx->varinfo[found->index]->write_format.width = temp_val;
                ctx->varinfo[found->index]->print_format.width = temp_val;
            }
        }
	break;
	case 4:
#line 186 "src/spss/readstat_sav_parse.rl"
	{
            if ((*p) != '\0') {
                unsigned char digit = (*p) - '0';
                if (temp_val <= (UINT_MAX - digit) / 10) {
                    temp_val = 10 * temp_val + digit;
                } else {
                    {p++; goto _out; }
                }
            }
        }
	break;
	case 5:
#line 197 "src/spss/readstat_sav_parse.rl"
	{ temp_val = 0; }
	break;
#line 722 "src/spss/readstat_sav_parse.c"
		}
	}

_again:
	if ( cs == 0 )
		goto _out;
	if ( ++p != pe )
		goto _resume;
	_test_eof: {}
	_out: {}
	}

#line 205 "src/spss/readstat_sav_parse.rl"

    
    if (cs < 12 || p != pe) {
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
