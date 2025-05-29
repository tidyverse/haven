#line 1 "src/spss/readstat_sav_parse_mr_name.rl"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "../readstat.h"
#include "../readstat_malloc.h"


#line 8 "src/spss/readstat_sav_parse_mr_name.c"
static const signed char _mr_extractor_actions[] = {
	0, 1, 0, 1, 1, 1, 2, 1,
	3, 1, 4, 0
};

static const signed char _mr_extractor_key_offsets[] = {
	0, 0, 8, 17, 19, 22, 24, 27,
	36, 48, 0
};

static const char _mr_extractor_trans_keys[] = {
	46, 95, 48, 57, 65, 90, 97, 122,
	46, 61, 95, 48, 57, 65, 90, 97,
	122, 67, 68, 32, 48, 57, 48, 57,
	32, 48, 57, 32, 46, 95, 48, 57,
	65, 90, 97, 122, 0, 32, 46, 95,
	9, 13, 48, 57, 65, 90, 97, 122,
	46, 95, 48, 57, 65, 90, 97, 122,
	0
};

static const signed char _mr_extractor_single_lengths[] = {
	0, 2, 3, 0, 1, 0, 1, 3,
	4, 2, 0
};

static const signed char _mr_extractor_range_lengths[] = {
	0, 3, 3, 1, 1, 1, 1, 3,
	4, 3, 0
};

static const signed char _mr_extractor_index_offsets[] = {
	0, 0, 6, 13, 15, 18, 20, 23,
	30, 39, 0
};

static const signed char _mr_extractor_cond_targs[] = {
	2, 2, 2, 2, 2, 0, 2, 3,
	2, 2, 2, 2, 0, 4, 0, 5,
	4, 0, 6, 0, 7, 6, 0, 7,
	8, 8, 8, 8, 8, 0, 9, 9,
	8, 8, 9, 8, 8, 8, 0, 8,
	8, 8, 8, 8, 0, 0, 1, 2,
	3, 4, 5, 6, 7, 8, 9, 0
};

static const signed char _mr_extractor_cond_actions[] = {
	0, 0, 0, 0, 0, 0, 0, 1,
	0, 0, 0, 0, 0, 3, 0, 5,
	0, 0, 0, 0, 7, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 9, 9,
	0, 0, 9, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0
};

static const int mr_extractor_start = 1;

static const int mr_extractor_en_main = 1;


#line 107 "src/spss/readstat_sav_parse_mr_name.rl"


readstat_error_t extract_mr_data(const char *line, mr_set_t *result) {
	readstat_error_t retval = READSTAT_OK;
	
	// Variables needed for Ragel operation
	int cs = 0;
	char *p = (char *)line;
	char *start = p;
	char *pe = p + strlen(p) + 1;
	
	// Variables needed for passing Ragel intermediate results
	char mr_type = '\0';
	int mr_counted_value = -1;
	int mr_subvar_count = 0;
	char **mr_subvariables = NULL;
	char *mr_name = NULL;
	char *mr_label = NULL;
	
	// Execute Ragel finite state machine (FSM)

#line 89 "src/spss/readstat_sav_parse_mr_name.c"
	{
		cs = (int)mr_extractor_start;
	}
	
#line 127 "src/spss/readstat_sav_parse_mr_name.rl"


#line 94 "src/spss/readstat_sav_parse_mr_name.c"
	{
		int _klen;
		unsigned int _trans = 0;
		const char * _keys;
		const signed char * _acts;
		unsigned int _nacts;
		_resume: {}
		if ( p == pe )
			goto _out;
		_keys = ( _mr_extractor_trans_keys + (_mr_extractor_key_offsets[cs]));
		_trans = (unsigned int)_mr_extractor_index_offsets[cs];
		
		_klen = (int)_mr_extractor_single_lengths[cs];
		if ( _klen > 0 ) {
			const char *_lower = _keys;
			const char *_upper = _keys + _klen - 1;
			const char *_mid;
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
		
		_klen = (int)_mr_extractor_range_lengths[cs];
		if ( _klen > 0 ) {
			const char *_lower = _keys;
			const char *_upper = _keys + (_klen<<1) - 2;
			const char *_mid;
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
		cs = (int)_mr_extractor_cond_targs[_trans];
		
		if ( _mr_extractor_cond_actions[_trans] != 0 ) {
			
			_acts = ( _mr_extractor_actions + (_mr_extractor_cond_actions[_trans]));
			_nacts = (unsigned int)(*( _acts));
			_acts += 1;
			while ( _nacts > 0 ) {
				switch ( (*( _acts)) )
				{
					case 0:  {
							{
#line 10 "src/spss/readstat_sav_parse_mr_name.rl"
							
							mr_name = (char *)readstat_malloc(p - start + 1);
							if (mr_name == NULL) {
								retval = READSTAT_ERROR_MALLOC;
								goto cleanup;
							}
							memcpy(mr_name, start, p - start);
							mr_name[p - start] = '\0';
						}
						
#line 177 "src/spss/readstat_sav_parse_mr_name.c"

						break; 
					}
					case 1:  {
							{
#line 20 "src/spss/readstat_sav_parse_mr_name.rl"
							
							mr_type = *p;
							start = p + 1;
						}
						
#line 188 "src/spss/readstat_sav_parse_mr_name.c"

						break; 
					}
					case 2:  {
							{
#line 25 "src/spss/readstat_sav_parse_mr_name.rl"
							
							int n_cv_digs = p - start;
							char *n_dig_str = (char *)readstat_malloc(n_cv_digs + 1);
							if (n_dig_str == NULL) {
								retval = READSTAT_ERROR_MALLOC;
								goto cleanup;
							}
							memcpy(n_dig_str, start, n_cv_digs);
							n_dig_str[n_cv_digs] = '\0';
							int n_digs = strtol(n_dig_str, NULL, 10);
							free(n_dig_str);
							if (n_digs != 0) {
								char *cv = (char *)readstat_malloc(n_digs + 1);
								if (cv == NULL) {
									retval = READSTAT_ERROR_MALLOC;
									goto cleanup;
								}
								memcpy(cv, p + 1, n_digs);
								cv[n_digs] = '\0';
								mr_counted_value = strtol(cv, NULL, 10);
								free(cv);
								p = p + 1 + n_digs;
								start = p + 1;
							}
							else {
								mr_counted_value = -1;
							}
						}
						
#line 223 "src/spss/readstat_sav_parse_mr_name.c"

						break; 
					}
					case 3:  {
							{
#line 54 "src/spss/readstat_sav_parse_mr_name.rl"
							
							char *lbl_len_str = (char *)readstat_malloc(p - start + 1);
							if (lbl_len_str == NULL) {
								retval = READSTAT_ERROR_MALLOC;
								goto cleanup;
							}
							memcpy(lbl_len_str, start, p - start);
							lbl_len_str[p - start] = '\0';
							int len = strtol(lbl_len_str, NULL, 10);
							free(lbl_len_str);
							mr_label = (char *)readstat_malloc(len + 1);
							if (mr_label == NULL) {
								retval = READSTAT_ERROR_MALLOC;
								goto cleanup;
							}
							memcpy(mr_label, p + 1, len);
							mr_label[len] = '\0';
							p = p + 1 + len;
							start = p + 1;
						}
						
#line 250 "src/spss/readstat_sav_parse_mr_name.c"

						break; 
					}
					case 4:  {
							{
#line 75 "src/spss/readstat_sav_parse_mr_name.rl"
							
							int len = p - start;
							char *subvar = (char *)readstat_malloc(len + 1);
							if (subvar == NULL) {
								retval = READSTAT_ERROR_MALLOC;
								goto cleanup;
							}
							memcpy(subvar, start, len);
							subvar[len] = '\0';
							start = p + 1;    
							char **new_subvariables = readstat_realloc(mr_subvariables, sizeof(char *) * (mr_subvar_count + 1));
							if (new_subvariables == NULL) {
								free(subvar);
								retval = READSTAT_ERROR_MALLOC;
								goto cleanup;
							}
							mr_subvariables = new_subvariables;
							mr_subvariables[mr_subvar_count++] = subvar;
						}
						
#line 276 "src/spss/readstat_sav_parse_mr_name.c"

						break; 
					}
				}
				_nacts -= 1;
				_acts += 1;
			}
			
		}
		
		if ( cs != 0 ) {
			p += 1;
			goto _resume;
		}
		_out: {}
	}
	
#line 128 "src/spss/readstat_sav_parse_mr_name.rl"

	
	// Check if FSM finished successfully
	if (cs < 
#line 296 "src/spss/readstat_sav_parse_mr_name.c"
9
#line 131 "src/spss/readstat_sav_parse_mr_name.rl"
 || p != pe) {
		retval = READSTAT_ERROR_BAD_MR_STRING;
		goto cleanup;
	}
	
	(void)mr_extractor_en_main;
	
	// Assign parsed values to output parameter
	result->name = mr_name;
	result->label = mr_label;
	result->type = mr_type;
	result->counted_value = mr_counted_value;
	result->subvariables = mr_subvariables;
	result->num_subvars = mr_subvar_count;
	if (result->type == 'D') {
		result->is_dichotomy = 1;
	}
	
	cleanup:
	if (retval != READSTAT_OK) {
		if (mr_subvariables != NULL) {
			for (int i = 0; i < mr_subvar_count; i++) {
				if (mr_subvariables[i] != NULL) free(mr_subvariables[i]);
				}
			free(mr_subvariables);
		}
		if (mr_name != NULL) free(mr_name);
			if (mr_label != NULL) free(mr_label);
		}
	return retval;
}


readstat_error_t parse_mr_line(const char *line, mr_set_t *result) {
	*result = (mr_set_t){0};
	return extract_mr_data(line, result);
}


#line 335 "src/spss/readstat_sav_parse_mr_name.c"
static const signed char _mr_parser_actions[] = {
	0, 1, 0, 0
};

static const signed char _mr_parser_key_offsets[] = {
	0, 0, 1, 2, 4, 0
};

static const char _mr_parser_trans_keys[] = {
	36, 10, 0, 10, 10, 0
};

static const signed char _mr_parser_single_lengths[] = {
	0, 1, 1, 2, 1, 0
};

static const signed char _mr_parser_range_lengths[] = {
	0, 0, 0, 0, 0, 0
};

static const signed char _mr_parser_index_offsets[] = {
	0, 0, 2, 4, 7, 0
};

static const signed char _mr_parser_cond_targs[] = {
	2, 0, 3, 2, 4, 3, 2, 3,
	2, 0, 1, 2, 3, 4, 0
};

static const signed char _mr_parser_cond_actions[] = {
	0, 0, 1, 0, 0, 1, 0, 1,
	0, 0, 0, 0, 0, 0, 0
};

static const int mr_parser_start = 1;

static const int mr_parser_en_main = 1;


#line 202 "src/spss/readstat_sav_parse_mr_name.rl"


readstat_error_t parse_mr_string(const char *line, mr_set_t **mr_sets, size_t *n_mr_lines) {
	readstat_error_t retval = READSTAT_OK;
	int cs = 0;
	char *p = (char *)line;
	char *start = p;
	char *pe = p + strlen(p) + 1;
	*mr_sets = NULL;
	*n_mr_lines = 0;
	

#line 385 "src/spss/readstat_sav_parse_mr_name.c"
	{
		cs = (int)mr_parser_start;
	}
	
#line 213 "src/spss/readstat_sav_parse_mr_name.rl"


#line 390 "src/spss/readstat_sav_parse_mr_name.c"
	{
		int _klen;
		unsigned int _trans = 0;
		const char * _keys;
		const signed char * _acts;
		unsigned int _nacts;
		_resume: {}
		if ( p == pe )
			goto _out;
		_keys = ( _mr_parser_trans_keys + (_mr_parser_key_offsets[cs]));
		_trans = (unsigned int)_mr_parser_index_offsets[cs];
		
		_klen = (int)_mr_parser_single_lengths[cs];
		if ( _klen > 0 ) {
			const char *_lower = _keys;
			const char *_upper = _keys + _klen - 1;
			const char *_mid;
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
		
		_klen = (int)_mr_parser_range_lengths[cs];
		if ( _klen > 0 ) {
			const char *_lower = _keys;
			const char *_upper = _keys + (_klen<<1) - 2;
			const char *_mid;
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
		cs = (int)_mr_parser_cond_targs[_trans];
		
		if ( _mr_parser_cond_actions[_trans] != 0 ) {
			
			_acts = ( _mr_parser_actions + (_mr_parser_cond_actions[_trans]));
			_nacts = (unsigned int)(*( _acts));
			_acts += 1;
			while ( _nacts > 0 ) {
				switch ( (*( _acts)) )
				{
					case 0:  {
							{
#line 172 "src/spss/readstat_sav_parse_mr_name.rl"
							
							char *mln = (char *)readstat_malloc(p - start);
							if (mln == NULL) {
								retval = READSTAT_ERROR_MALLOC;
								goto cleanup;
							}
							memcpy(mln, start + 1, p - start);
							mln[p - start - 1] = '\0';
							mr_set_t *new_mr_sets = readstat_realloc(*mr_sets, ((*n_mr_lines) + 1) * sizeof(mr_set_t));
							if (new_mr_sets == NULL) {
								free(mln);
								retval = READSTAT_ERROR_MALLOC;
								goto cleanup;
							}
							*mr_sets = new_mr_sets;
							retval = parse_mr_line(mln, &(*mr_sets)[*n_mr_lines]);
							free(mln);
							if (retval != READSTAT_OK) {
								goto cleanup;
							}
							(*n_mr_lines)++;
							start = p + 1;
						}
						
#line 487 "src/spss/readstat_sav_parse_mr_name.c"

						break; 
					}
				}
				_nacts -= 1;
				_acts += 1;
			}
			
		}
		
		if ( cs != 0 ) {
			p += 1;
			goto _resume;
		}
		_out: {}
	}
	
#line 214 "src/spss/readstat_sav_parse_mr_name.rl"

	
	if (cs < 
#line 506 "src/spss/readstat_sav_parse_mr_name.c"
4
#line 216 "src/spss/readstat_sav_parse_mr_name.rl"
 || p != pe) {
		retval = READSTAT_ERROR_BAD_MR_STRING;
		goto cleanup;
	}
	
	(void)mr_parser_en_main;
	
	cleanup:
	return retval;
}
