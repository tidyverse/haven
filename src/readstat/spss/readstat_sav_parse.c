
#line 1 "src/spss/readstat_sav_parse.rl"
#include <limits.h>
#include <stdlib.h>
#include "../readstat.h"
#include "../readstat_malloc.h"

#include "readstat_sav.h"
#include "readstat_sav_parse.h"

typedef struct varlookup {
    char      name[8*4+1];
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


#line 64 "src/spss/readstat_sav_parse.c"
static const char _sav_long_variable_parse_actions[] = {
	0, 1, 3, 1, 5, 2, 4, 1, 
	3, 6, 2, 0
};

static const short _sav_long_variable_parse_key_offsets[] = {
	0, 0, 6, 20, 34, 48, 62, 76, 
	90, 104, 105, 110, 116, 122, 128, 134, 
	140, 146, 152, 158, 164, 170, 176, 182, 
	188, 194, 200, 206, 212, 218, 224, 230, 
	236, 242, 248, 254, 260, 266, 272, 278, 
	284, 290, 296, 302, 308, 314, 320, 326, 
	332, 338, 344, 350, 356, 362, 368, 374, 
	380, 386, 392, 398, 404, 410, 416, 422, 
	428, 434, 440, 446, 452, 458, 464, 470, 
	476, 482, 488, 494
};

static const unsigned char _sav_long_variable_parse_trans_keys[] = {
	0u, 63u, 91u, 127u, 248u, 255u, 47u, 61u, 
	0u, 34u, 37u, 45u, 58u, 63u, 91u, 94u, 
	96u, 127u, 248u, 255u, 47u, 61u, 0u, 34u, 
	37u, 45u, 58u, 63u, 91u, 94u, 96u, 127u, 
	248u, 255u, 47u, 61u, 0u, 34u, 37u, 45u, 
	58u, 63u, 91u, 94u, 96u, 127u, 248u, 255u, 
	47u, 61u, 0u, 34u, 37u, 45u, 58u, 63u, 
	91u, 94u, 96u, 127u, 248u, 255u, 47u, 61u, 
	0u, 34u, 37u, 45u, 58u, 63u, 91u, 94u, 
	96u, 127u, 248u, 255u, 47u, 61u, 0u, 34u, 
	37u, 45u, 58u, 63u, 91u, 94u, 96u, 127u, 
	248u, 255u, 47u, 61u, 0u, 34u, 37u, 45u, 
	58u, 63u, 91u, 94u, 96u, 127u, 248u, 255u, 
	61u, 127u, 0u, 31u, 248u, 255u, 9u, 127u, 
	0u, 31u, 248u, 255u, 0u, 63u, 91u, 127u, 
	248u, 255u, 9u, 127u, 0u, 31u, 248u, 255u, 
	9u, 127u, 0u, 31u, 248u, 255u, 9u, 127u, 
	0u, 31u, 248u, 255u, 9u, 127u, 0u, 31u, 
	248u, 255u, 9u, 127u, 0u, 31u, 248u, 255u, 
	9u, 127u, 0u, 31u, 248u, 255u, 9u, 127u, 
	0u, 31u, 248u, 255u, 9u, 127u, 0u, 31u, 
	248u, 255u, 9u, 127u, 0u, 31u, 248u, 255u, 
	9u, 127u, 0u, 31u, 248u, 255u, 9u, 127u, 
	0u, 31u, 248u, 255u, 9u, 127u, 0u, 31u, 
	248u, 255u, 9u, 127u, 0u, 31u, 248u, 255u, 
	9u, 127u, 0u, 31u, 248u, 255u, 9u, 127u, 
	0u, 31u, 248u, 255u, 9u, 127u, 0u, 31u, 
	248u, 255u, 9u, 127u, 0u, 31u, 248u, 255u, 
	9u, 127u, 0u, 31u, 248u, 255u, 9u, 127u, 
	0u, 31u, 248u, 255u, 9u, 127u, 0u, 31u, 
	248u, 255u, 9u, 127u, 0u, 31u, 248u, 255u, 
	9u, 127u, 0u, 31u, 248u, 255u, 9u, 127u, 
	0u, 31u, 248u, 255u, 9u, 127u, 0u, 31u, 
	248u, 255u, 9u, 127u, 0u, 31u, 248u, 255u, 
	9u, 127u, 0u, 31u, 248u, 255u, 9u, 127u, 
	0u, 31u, 248u, 255u, 9u, 127u, 0u, 31u, 
	248u, 255u, 9u, 127u, 0u, 31u, 248u, 255u, 
	9u, 127u, 0u, 31u, 248u, 255u, 9u, 127u, 
	0u, 31u, 248u, 255u, 9u, 127u, 0u, 31u, 
	248u, 255u, 9u, 127u, 0u, 31u, 248u, 255u, 
	9u, 127u, 0u, 31u, 248u, 255u, 9u, 127u, 
	0u, 31u, 248u, 255u, 9u, 127u, 0u, 31u, 
	248u, 255u, 9u, 127u, 0u, 31u, 248u, 255u, 
	9u, 127u, 0u, 31u, 248u, 255u, 9u, 127u, 
	0u, 31u, 248u, 255u, 9u, 127u, 0u, 31u, 
	248u, 255u, 9u, 127u, 0u, 31u, 248u, 255u, 
	9u, 127u, 0u, 31u, 248u, 255u, 9u, 127u, 
	0u, 31u, 248u, 255u, 9u, 127u, 0u, 31u, 
	248u, 255u, 9u, 127u, 0u, 31u, 248u, 255u, 
	9u, 127u, 0u, 31u, 248u, 255u, 9u, 127u, 
	0u, 31u, 248u, 255u, 9u, 127u, 0u, 31u, 
	248u, 255u, 9u, 127u, 0u, 31u, 248u, 255u, 
	9u, 127u, 0u, 31u, 248u, 255u, 9u, 127u, 
	0u, 31u, 248u, 255u, 9u, 127u, 0u, 31u, 
	248u, 255u, 9u, 127u, 0u, 31u, 248u, 255u, 
	9u, 127u, 0u, 31u, 248u, 255u, 9u, 127u, 
	0u, 31u, 248u, 255u, 9u, 127u, 0u, 31u, 
	248u, 255u, 9u, 127u, 0u, 31u, 248u, 255u, 
	9u, 127u, 0u, 31u, 248u, 255u, 9u, 127u, 
	0u, 31u, 248u, 255u, 9u, 127u, 0u, 31u, 
	248u, 255u, 9u, 127u, 0u, 31u, 248u, 255u, 
	9u, 127u, 0u, 31u, 248u, 255u, 9u, 0
};

static const char _sav_long_variable_parse_single_lengths[] = {
	0, 0, 2, 2, 2, 2, 2, 2, 
	2, 1, 1, 2, 0, 2, 2, 2, 
	2, 2, 2, 2, 2, 2, 2, 2, 
	2, 2, 2, 2, 2, 2, 2, 2, 
	2, 2, 2, 2, 2, 2, 2, 2, 
	2, 2, 2, 2, 2, 2, 2, 2, 
	2, 2, 2, 2, 2, 2, 2, 2, 
	2, 2, 2, 2, 2, 2, 2, 2, 
	2, 2, 2, 2, 2, 2, 2, 2, 
	2, 2, 2, 1
};

static const char _sav_long_variable_parse_range_lengths[] = {
	0, 3, 6, 6, 6, 6, 6, 6, 
	6, 0, 2, 2, 3, 2, 2, 2, 
	2, 2, 2, 2, 2, 2, 2, 2, 
	2, 2, 2, 2, 2, 2, 2, 2, 
	2, 2, 2, 2, 2, 2, 2, 2, 
	2, 2, 2, 2, 2, 2, 2, 2, 
	2, 2, 2, 2, 2, 2, 2, 2, 
	2, 2, 2, 2, 2, 2, 2, 2, 
	2, 2, 2, 2, 2, 2, 2, 2, 
	2, 2, 2, 0
};

static const short _sav_long_variable_parse_index_offsets[] = {
	0, 0, 4, 13, 22, 31, 40, 49, 
	58, 67, 69, 73, 78, 82, 87, 92, 
	97, 102, 107, 112, 117, 122, 127, 132, 
	137, 142, 147, 152, 157, 162, 167, 172, 
	177, 182, 187, 192, 197, 202, 207, 212, 
	217, 222, 227, 232, 237, 242, 247, 252, 
	257, 262, 267, 272, 277, 282, 287, 292, 
	297, 302, 307, 312, 317, 322, 327, 332, 
	337, 342, 347, 352, 357, 362, 367, 372, 
	377, 382, 387, 392
};

static const char _sav_long_variable_parse_indicies[] = {
	1, 1, 1, 0, 1, 3, 1, 1, 
	1, 1, 1, 1, 2, 1, 3, 1, 
	1, 1, 1, 1, 1, 4, 1, 3, 
	1, 1, 1, 1, 1, 1, 5, 1, 
	3, 1, 1, 1, 1, 1, 1, 6, 
	1, 3, 1, 1, 1, 1, 1, 1, 
	7, 1, 3, 1, 1, 1, 1, 1, 
	1, 8, 1, 3, 1, 1, 1, 1, 
	1, 1, 9, 3, 1, 1, 1, 1, 
	10, 11, 1, 1, 1, 12, 1, 1, 
	1, 0, 11, 1, 1, 1, 13, 11, 
	1, 1, 1, 14, 11, 1, 1, 1, 
	15, 11, 1, 1, 1, 16, 11, 1, 
	1, 1, 17, 11, 1, 1, 1, 18, 
	11, 1, 1, 1, 19, 11, 1, 1, 
	1, 20, 11, 1, 1, 1, 21, 11, 
	1, 1, 1, 22, 11, 1, 1, 1, 
	23, 11, 1, 1, 1, 24, 11, 1, 
	1, 1, 25, 11, 1, 1, 1, 26, 
	11, 1, 1, 1, 27, 11, 1, 1, 
	1, 28, 11, 1, 1, 1, 29, 11, 
	1, 1, 1, 30, 11, 1, 1, 1, 
	31, 11, 1, 1, 1, 32, 11, 1, 
	1, 1, 33, 11, 1, 1, 1, 34, 
	11, 1, 1, 1, 35, 11, 1, 1, 
	1, 36, 11, 1, 1, 1, 37, 11, 
	1, 1, 1, 38, 11, 1, 1, 1, 
	39, 11, 1, 1, 1, 40, 11, 1, 
	1, 1, 41, 11, 1, 1, 1, 42, 
	11, 1, 1, 1, 43, 11, 1, 1, 
	1, 44, 11, 1, 1, 1, 45, 11, 
	1, 1, 1, 46, 11, 1, 1, 1, 
	47, 11, 1, 1, 1, 48, 11, 1, 
	1, 1, 49, 11, 1, 1, 1, 50, 
	11, 1, 1, 1, 51, 11, 1, 1, 
	1, 52, 11, 1, 1, 1, 53, 11, 
	1, 1, 1, 54, 11, 1, 1, 1, 
	55, 11, 1, 1, 1, 56, 11, 1, 
	1, 1, 57, 11, 1, 1, 1, 58, 
	11, 1, 1, 1, 59, 11, 1, 1, 
	1, 60, 11, 1, 1, 1, 61, 11, 
	1, 1, 1, 62, 11, 1, 1, 1, 
	63, 11, 1, 1, 1, 64, 11, 1, 
	1, 1, 65, 11, 1, 1, 1, 66, 
	11, 1, 1, 1, 67, 11, 1, 1, 
	1, 68, 11, 1, 1, 1, 69, 11, 
	1, 1, 1, 70, 11, 1, 1, 1, 
	71, 11, 1, 1, 1, 72, 11, 1, 
	1, 1, 73, 11, 1, 1, 1, 74, 
	11, 1, 0
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


#line 64 "src/spss/readstat_sav_parse.rl"


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

    
#line 308 "src/spss/readstat_sav_parse.c"
	{
	cs = sav_long_variable_parse_start;
	}

#line 313 "src/spss/readstat_sav_parse.c"
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
#line 87 "src/spss/readstat_sav_parse.rl"
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
	case 1:
#line 99 "src/spss/readstat_sav_parse.rl"
	{
            memcpy(temp_key, str_start, str_len);
            temp_key[str_len] = '\0';
        }
	break;
	case 2:
#line 104 "src/spss/readstat_sav_parse.rl"
	{
            memcpy(temp_val, str_start, str_len);
            temp_val[str_len] = '\0';
        }
	break;
	case 3:
#line 111 "src/spss/readstat_sav_parse.rl"
	{ str_start = p; }
	break;
	case 4:
#line 111 "src/spss/readstat_sav_parse.rl"
	{ str_len = p - str_start; }
	break;
	case 5:
#line 113 "src/spss/readstat_sav_parse.rl"
	{ str_start = p; }
	break;
	case 6:
#line 113 "src/spss/readstat_sav_parse.rl"
	{ str_len = p - str_start; }
	break;
#line 431 "src/spss/readstat_sav_parse.c"
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
	case 0:
#line 87 "src/spss/readstat_sav_parse.rl"
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
	case 2:
#line 104 "src/spss/readstat_sav_parse.rl"
	{
            memcpy(temp_val, str_start, str_len);
            temp_val[str_len] = '\0';
        }
	break;
	case 6:
#line 113 "src/spss/readstat_sav_parse.rl"
	{ str_len = p - str_start; }
	break;
#line 472 "src/spss/readstat_sav_parse.c"
		}
	}
	}

	_out: {}
	}

#line 121 "src/spss/readstat_sav_parse.rl"


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


#line 503 "src/spss/readstat_sav_parse.c"
static const char _sav_very_long_string_parse_actions[] = {
	0, 1, 0, 1, 2, 1, 3, 2, 
	4, 1, 2, 5, 2
};

static const char _sav_very_long_string_parse_key_offsets[] = {
	0, 0, 6, 20, 34, 48, 62, 76, 
	90, 104, 105, 107, 110, 112
};

static const unsigned char _sav_very_long_string_parse_trans_keys[] = {
	0u, 63u, 91u, 127u, 248u, 255u, 47u, 61u, 
	0u, 34u, 37u, 45u, 58u, 63u, 91u, 94u, 
	96u, 127u, 248u, 255u, 47u, 61u, 0u, 34u, 
	37u, 45u, 58u, 63u, 91u, 94u, 96u, 127u, 
	248u, 255u, 47u, 61u, 0u, 34u, 37u, 45u, 
	58u, 63u, 91u, 94u, 96u, 127u, 248u, 255u, 
	47u, 61u, 0u, 34u, 37u, 45u, 58u, 63u, 
	91u, 94u, 96u, 127u, 248u, 255u, 47u, 61u, 
	0u, 34u, 37u, 45u, 58u, 63u, 91u, 94u, 
	96u, 127u, 248u, 255u, 47u, 61u, 0u, 34u, 
	37u, 45u, 58u, 63u, 91u, 94u, 96u, 127u, 
	248u, 255u, 47u, 61u, 0u, 34u, 37u, 45u, 
	58u, 63u, 91u, 94u, 96u, 127u, 248u, 255u, 
	61u, 48u, 57u, 0u, 48u, 57u, 0u, 9u, 
	0u, 63u, 91u, 127u, 248u, 255u, 0
};

static const char _sav_very_long_string_parse_single_lengths[] = {
	0, 0, 2, 2, 2, 2, 2, 2, 
	2, 1, 0, 1, 2, 0
};

static const char _sav_very_long_string_parse_range_lengths[] = {
	0, 3, 6, 6, 6, 6, 6, 6, 
	6, 0, 1, 1, 0, 3
};

static const char _sav_very_long_string_parse_index_offsets[] = {
	0, 0, 4, 13, 22, 31, 40, 49, 
	58, 67, 69, 71, 74, 77
};

static const char _sav_very_long_string_parse_indicies[] = {
	1, 1, 1, 0, 1, 3, 1, 1, 
	1, 1, 1, 1, 2, 1, 3, 1, 
	1, 1, 1, 1, 1, 4, 1, 3, 
	1, 1, 1, 1, 1, 1, 5, 1, 
	3, 1, 1, 1, 1, 1, 1, 6, 
	1, 3, 1, 1, 1, 1, 1, 1, 
	7, 1, 3, 1, 1, 1, 1, 1, 
	1, 8, 1, 3, 1, 1, 1, 1, 
	1, 1, 9, 3, 1, 10, 1, 11, 
	12, 1, 13, 14, 1, 1, 1, 1, 
	0, 0
};

static const char _sav_very_long_string_parse_trans_targs[] = {
	2, 0, 3, 10, 4, 5, 6, 7, 
	8, 9, 11, 12, 11, 12, 13
};

static const char _sav_very_long_string_parse_trans_actions[] = {
	5, 0, 0, 7, 0, 0, 0, 0, 
	0, 0, 10, 1, 3, 0, 0
};

static const int sav_very_long_string_parse_start = 1;

static const int sav_very_long_string_parse_en_main = 1;


#line 146 "src/spss/readstat_sav_parse.rl"


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
    
    
#line 601 "src/spss/readstat_sav_parse.c"
	{
	cs = sav_very_long_string_parse_start;
	}

#line 606 "src/spss/readstat_sav_parse.c"
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
#line 170 "src/spss/readstat_sav_parse.rl"
	{
            varlookup_t *found = bsearch(temp_key, table, var_count, sizeof(varlookup_t), &compare_key_varlookup);
            if (found) {
                ctx->varinfo[found->index]->string_length = temp_val;
            }
        }
	break;
	case 1:
#line 177 "src/spss/readstat_sav_parse.rl"
	{
            memcpy(temp_key, str_start, str_len);
            temp_key[str_len] = '\0';
        }
	break;
	case 2:
#line 182 "src/spss/readstat_sav_parse.rl"
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
	case 3:
#line 195 "src/spss/readstat_sav_parse.rl"
	{ str_start = p; }
	break;
	case 4:
#line 195 "src/spss/readstat_sav_parse.rl"
	{ str_len = p - str_start; }
	break;
	case 5:
#line 197 "src/spss/readstat_sav_parse.rl"
	{ temp_val = 0; }
	break;
#line 721 "src/spss/readstat_sav_parse.c"
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
