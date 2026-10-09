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


#line 68 "src/spss/readstat_sav_parse.c"
static const signed char _sav_long_variable_parse_actions[] = {
	0, 1, 1, 1, 5, 2, 2, 0,
	3, 6, 4, 3, 0
};

static const signed char _sav_long_variable_parse_key_offsets[] = {
	0, 0, 7, 21, 35, 49, 63, 77,
	91, 105, 106, 110, 116, 117, 0
};

static const unsigned char _sav_long_variable_parse_trans_keys[] = {
	255u, 0u, 63u, 91u, 96u, 123u, 127u, 47u,
	61u, 96u, 255u, 0u, 34u, 37u, 45u, 58u,
	63u, 91u, 94u, 123u, 127u, 47u, 61u, 96u,
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
	127u, 61u, 127u, 255u, 0u, 31u, 0u, 9u,
	127u, 255u, 1u, 31u, 0u, 0u, 255u, 1u,
	63u, 91u, 96u, 123u, 127u, 0u
};

static const signed char _sav_long_variable_parse_single_lengths[] = {
	0, 1, 4, 4, 4, 4, 4, 4,
	4, 1, 2, 4, 1, 2, 0
};

static const signed char _sav_long_variable_parse_range_lengths[] = {
	0, 3, 5, 5, 5, 5, 5, 5,
	5, 0, 1, 1, 0, 3, 0
};

static const signed char _sav_long_variable_parse_index_offsets[] = {
	0, 0, 5, 15, 25, 35, 45, 55,
	65, 75, 77, 81, 87, 89, 0
};

static const signed char _sav_long_variable_parse_cond_targs[] = {
	0, 0, 0, 0, 2, 0, 10, 0,
	0, 0, 0, 0, 0, 0, 3, 0,
	10, 0, 0, 0, 0, 0, 0, 0,
	4, 0, 10, 0, 0, 0, 0, 0,
	0, 0, 5, 0, 10, 0, 0, 0,
	0, 0, 0, 0, 6, 0, 10, 0,
	0, 0, 0, 0, 0, 0, 7, 0,
	10, 0, 0, 0, 0, 0, 0, 0,
	8, 0, 10, 0, 0, 0, 0, 0,
	0, 0, 9, 10, 0, 0, 0, 0,
	11, 12, 13, 0, 0, 0, 11, 12,
	0, 12, 0, 0, 0, 0, 2, 0,
	1, 2, 3, 4, 5, 6, 7, 8,
	9, 10, 11, 12, 13, 0
};

static const signed char _sav_long_variable_parse_cond_actions[] = {
	0, 0, 0, 0, 1, 0, 5, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	5, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 5, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 5, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 5, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	5, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 5, 0, 0, 0, 0, 0,
	0, 0, 0, 5, 0, 0, 0, 0,
	3, 8, 8, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 1, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 8, 0, 0, 0
};

static const signed char _sav_long_variable_parse_eof_trans[] = {
	96, 97, 98, 99, 100, 101, 102, 103,
	104, 105, 106, 107, 108, 109, 0
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
	
	
#line 181 "src/spss/readstat_sav_parse.c"
	{
		cs = (int)sav_long_variable_parse_start;
	}
	
#line 186 "src/spss/readstat_sav_parse.c"
	{
		int _klen;
		unsigned int _trans = 0;
		const unsigned char * _keys;
		const signed char * _acts;
		unsigned int _nacts;
		_resume: {}
		if ( p == pe && p != eof )
			goto _out;
		if ( p == eof ) {
			if ( _sav_long_variable_parse_eof_trans[cs] > 0 ) {
				_trans = (unsigned int)_sav_long_variable_parse_eof_trans[cs] - 1;
			}
		}
		else {
			_keys = ( _sav_long_variable_parse_trans_keys + (_sav_long_variable_parse_key_offsets[cs]));
			_trans = (unsigned int)_sav_long_variable_parse_index_offsets[cs];
			
			_klen = (int)_sav_long_variable_parse_single_lengths[cs];
			if ( _klen > 0 ) {
				const unsigned char *_lower = _keys;
				const unsigned char *_upper = _keys + _klen - 1;
				const unsigned char *_mid;
				while ( 1 ) {
					if ( _upper < _lower ) {
						_keys += _klen;
						_trans += (unsigned int)_klen;
						break;
					}
					
					_mid = _lower + ((_upper-_lower) >> 1);
					if ( ( (*( p))) < (*( _mid)) )
						_upper = _mid - 1;
					else if ( ( (*( p))) > (*( _mid)) )
						_lower = _mid + 1;
					else {
						_trans += (unsigned int)(_mid - _keys);
						goto _match;
					}
				}
			}
			
			_klen = (int)_sav_long_variable_parse_range_lengths[cs];
			if ( _klen > 0 ) {
				const unsigned char *_lower = _keys;
				const unsigned char *_upper = _keys + (_klen<<1) - 2;
				const unsigned char *_mid;
				while ( 1 ) {
					if ( _upper < _lower ) {
						_trans += (unsigned int)_klen;
						break;
					}
					
					_mid = _lower + (((_upper-_lower) >> 1) & ~1);
					if ( ( (*( p))) < (*( _mid)) )
						_upper = _mid - 2;
					else if ( ( (*( p))) > (*( _mid + 1)) )
						_lower = _mid + 2;
					else {
						_trans += (unsigned int)((_mid - _keys)>>1);
						break;
					}
				}
			}
			
			_match: {}
		}
		cs = (int)_sav_long_variable_parse_cond_targs[_trans];
		
		if ( _sav_long_variable_parse_cond_actions[_trans] != 0 ) {
			
			_acts = ( _sav_long_variable_parse_actions + (_sav_long_variable_parse_cond_actions[_trans]));
			_nacts = (unsigned int)(*( _acts));
			_acts += 1;
			while ( _nacts > 0 ) {
				switch ( (*( _acts)) )
				{
					case 0:  {
						{
#line 13 "src/spss/readstat_sav_parse.rl"
							
							memcpy(temp_key, str_start, str_len);
							temp_key[str_len] = '\0';
						}
						
#line 272 "src/spss/readstat_sav_parse.c"
						
						break; 
					}
					case 1:  {
						{
#line 20 "src/spss/readstat_sav_parse.rl"
							str_start = p; }
						
#line 281 "src/spss/readstat_sav_parse.c"
						
						break; 
					}
					case 2:  {
						{
#line 20 "src/spss/readstat_sav_parse.rl"
							str_len = p - str_start; }
						
#line 290 "src/spss/readstat_sav_parse.c"
						
						break; 
					}
					case 3:  {
						{
#line 102 "src/spss/readstat_sav_parse.rl"
							
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
						
#line 324 "src/spss/readstat_sav_parse.c"
						
						break; 
					}
					case 4:  {
						{
#line 129 "src/spss/readstat_sav_parse.rl"
							
							if (str_len > sizeof(temp_val) - 1)
							str_len = sizeof(temp_val) - 1;
							memcpy(temp_val, str_start, str_len);
							temp_val[str_len] = '\0';
						}
						
#line 338 "src/spss/readstat_sav_parse.c"
						
						break; 
					}
					case 5:  {
						{
#line 136 "src/spss/readstat_sav_parse.rl"
							str_start = p; }
						
#line 347 "src/spss/readstat_sav_parse.c"
						
						break; 
					}
					case 6:  {
						{
#line 136 "src/spss/readstat_sav_parse.rl"
							str_len = p - str_start; if (str_len > 64) str_len = 64; }
						
#line 356 "src/spss/readstat_sav_parse.c"
						
						break; 
					}
				}
				_nacts -= 1;
				_acts += 1;
			}
			
		}
		
		if ( p == eof ) {
			if ( cs >= 11 )
				goto _out;
		}
		else {
			if ( cs != 0 ) {
				p += 1;
				goto _resume;
			}
		}
		_out: {}
	}
	
#line 144 "src/spss/readstat_sav_parse.rl"
	
	
	if (cs < 
#line 384 "src/spss/readstat_sav_parse.c"
	11
#line 146 "src/spss/readstat_sav_parse.rl"
	|| p != pe) {
		if (ctx->handle.error) {
			snprintf(error_buf, sizeof(error_buf), "Error parsing string \"%.*s\" around byte #%ld/%d, character %c", 
			count, (char *)data, (long)(p - c_data), count, p < pe ? *p : '?');
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


#line 407 "src/spss/readstat_sav_parse.c"
static const signed char _sav_very_long_string_parse_actions[] = {
	0, 1, 1, 1, 3, 1, 4, 2,
	2, 0, 2, 5, 4, 0
};

static const signed char _sav_very_long_string_parse_key_offsets[] = {
	0, 0, 7, 21, 35, 49, 63, 77,
	91, 105, 106, 108, 112, 114, 0
};

static const unsigned char _sav_very_long_string_parse_trans_keys[] = {
	255u, 0u, 63u, 91u, 96u, 123u, 127u, 47u,
	61u, 96u, 255u, 0u, 34u, 37u, 45u, 58u,
	63u, 91u, 94u, 123u, 127u, 47u, 61u, 96u,
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
	127u, 61u, 48u, 57u, 0u, 9u, 48u, 57u,
	0u, 9u, 255u, 0u, 63u, 91u, 96u, 123u,
	127u, 0u
};

static const signed char _sav_very_long_string_parse_single_lengths[] = {
	0, 1, 4, 4, 4, 4, 4, 4,
	4, 1, 0, 2, 2, 1, 0
};

static const signed char _sav_very_long_string_parse_range_lengths[] = {
	0, 3, 5, 5, 5, 5, 5, 5,
	5, 0, 1, 1, 0, 3, 0
};

static const signed char _sav_very_long_string_parse_index_offsets[] = {
	0, 0, 5, 15, 25, 35, 45, 55,
	65, 75, 77, 79, 83, 86, 0
};

static const signed char _sav_very_long_string_parse_cond_targs[] = {
	0, 0, 0, 0, 2, 0, 10, 0,
	0, 0, 0, 0, 0, 0, 3, 0,
	10, 0, 0, 0, 0, 0, 0, 0,
	4, 0, 10, 0, 0, 0, 0, 0,
	0, 0, 5, 0, 10, 0, 0, 0,
	0, 0, 0, 0, 6, 0, 10, 0,
	0, 0, 0, 0, 0, 0, 7, 0,
	10, 0, 0, 0, 0, 0, 0, 0,
	8, 0, 10, 0, 0, 0, 0, 0,
	0, 0, 9, 10, 0, 11, 0, 12,
	13, 11, 0, 12, 13, 0, 0, 0,
	0, 0, 2, 0, 1, 2, 3, 4,
	5, 6, 7, 8, 9, 10, 11, 12,
	13, 0
};

static const signed char _sav_very_long_string_parse_cond_actions[] = {
	0, 0, 0, 0, 1, 0, 7, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	7, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 7, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 7, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 7, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	7, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 7, 0, 0, 0, 0, 0,
	0, 0, 0, 7, 0, 10, 0, 3,
	3, 5, 0, 0, 0, 0, 0, 0,
	0, 0, 1, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 3, 0,
	0, 0
};

static const signed char _sav_very_long_string_parse_eof_trans[] = {
	92, 93, 94, 95, 96, 97, 98, 99,
	100, 101, 102, 103, 104, 105, 0
};

static const int sav_very_long_string_parse_start = 1;

static const int sav_very_long_string_parse_en_main = 1;


#line 170 "src/spss/readstat_sav_parse.rl"


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
	
	
#line 522 "src/spss/readstat_sav_parse.c"
	{
		cs = (int)sav_very_long_string_parse_start;
	}
	
#line 527 "src/spss/readstat_sav_parse.c"
	{
		int _klen;
		unsigned int _trans = 0;
		const unsigned char * _keys;
		const signed char * _acts;
		unsigned int _nacts;
		_resume: {}
		if ( p == pe && p != eof )
			goto _out;
		if ( p == eof ) {
			if ( _sav_very_long_string_parse_eof_trans[cs] > 0 ) {
				_trans = (unsigned int)_sav_very_long_string_parse_eof_trans[cs] - 1;
			}
		}
		else {
			_keys = ( _sav_very_long_string_parse_trans_keys + (_sav_very_long_string_parse_key_offsets[cs]));
			_trans = (unsigned int)_sav_very_long_string_parse_index_offsets[cs];
			
			_klen = (int)_sav_very_long_string_parse_single_lengths[cs];
			if ( _klen > 0 ) {
				const unsigned char *_lower = _keys;
				const unsigned char *_upper = _keys + _klen - 1;
				const unsigned char *_mid;
				while ( 1 ) {
					if ( _upper < _lower ) {
						_keys += _klen;
						_trans += (unsigned int)_klen;
						break;
					}
					
					_mid = _lower + ((_upper-_lower) >> 1);
					if ( ( (*( p))) < (*( _mid)) )
						_upper = _mid - 1;
					else if ( ( (*( p))) > (*( _mid)) )
						_lower = _mid + 1;
					else {
						_trans += (unsigned int)(_mid - _keys);
						goto _match;
					}
				}
			}
			
			_klen = (int)_sav_very_long_string_parse_range_lengths[cs];
			if ( _klen > 0 ) {
				const unsigned char *_lower = _keys;
				const unsigned char *_upper = _keys + (_klen<<1) - 2;
				const unsigned char *_mid;
				while ( 1 ) {
					if ( _upper < _lower ) {
						_trans += (unsigned int)_klen;
						break;
					}
					
					_mid = _lower + (((_upper-_lower) >> 1) & ~1);
					if ( ( (*( p))) < (*( _mid)) )
						_upper = _mid - 2;
					else if ( ( (*( p))) > (*( _mid + 1)) )
						_lower = _mid + 2;
					else {
						_trans += (unsigned int)((_mid - _keys)>>1);
						break;
					}
				}
			}
			
			_match: {}
		}
		cs = (int)_sav_very_long_string_parse_cond_targs[_trans];
		
		if ( _sav_very_long_string_parse_cond_actions[_trans] != 0 ) {
			
			_acts = ( _sav_very_long_string_parse_actions + (_sav_very_long_string_parse_cond_actions[_trans]));
			_nacts = (unsigned int)(*( _acts));
			_acts += 1;
			while ( _nacts > 0 ) {
				switch ( (*( _acts)) )
				{
					case 0:  {
						{
#line 13 "src/spss/readstat_sav_parse.rl"
							
							memcpy(temp_key, str_start, str_len);
							temp_key[str_len] = '\0';
						}
						
#line 613 "src/spss/readstat_sav_parse.c"
						
						break; 
					}
					case 1:  {
						{
#line 20 "src/spss/readstat_sav_parse.rl"
							str_start = p; }
						
#line 622 "src/spss/readstat_sav_parse.c"
						
						break; 
					}
					case 2:  {
						{
#line 20 "src/spss/readstat_sav_parse.rl"
							str_len = p - str_start; }
						
#line 631 "src/spss/readstat_sav_parse.c"
						
						break; 
					}
					case 3:  {
						{
#line 195 "src/spss/readstat_sav_parse.rl"
							
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
						
#line 662 "src/spss/readstat_sav_parse.c"
						
						break; 
					}
					case 4:  {
						{
#line 219 "src/spss/readstat_sav_parse.rl"
							
							if ((( (*( p)))) != '\0') {
								unsigned char digit = (( (*( p)))) - '0';
								if (temp_val <= (UINT_MAX - digit) / 10) {
									temp_val = 10 * temp_val + digit;
								} else {
									{p += 1; goto _out; }
								}
							}
						}
						
#line 680 "src/spss/readstat_sav_parse.c"
						
						break; 
					}
					case 5:  {
						{
#line 230 "src/spss/readstat_sav_parse.rl"
							temp_val = 0; }
						
#line 689 "src/spss/readstat_sav_parse.c"
						
						break; 
					}
				}
				_nacts -= 1;
				_acts += 1;
			}
			
		}
		
		if ( p == eof ) {
			if ( cs >= 11 )
				goto _out;
		}
		else {
			if ( cs != 0 ) {
				p += 1;
				goto _resume;
			}
		}
		_out: {}
	}
	
#line 238 "src/spss/readstat_sav_parse.rl"
	
	
	if (cs < 
#line 717 "src/spss/readstat_sav_parse.c"
	11
#line 240 "src/spss/readstat_sav_parse.rl"
	|| p != pe) {
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
