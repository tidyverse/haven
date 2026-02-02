
#line 1 "src/spss/readstat_sav_parse_mr_name.rl"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "../readstat.h"
#include "../readstat_malloc.h"
#include "../readstat_iconv.h"
#include "../readstat_convert.h"
#include "readstat_sav.h"


#line 11 "src/spss/readstat_sav_parse_mr_name.c"
static const char _mr_extractor_actions[] = {
	0, 1, 0, 1, 1, 1, 2, 1, 
	3, 1, 4
};

static const char _mr_extractor_key_offsets[] = {
	0, 0, 2, 4, 6, 9, 11, 14, 
	19, 24, 29, 31, 36, 41
};

static const char _mr_extractor_trans_keys[] = {
	32, 61, 32, 61, 67, 68, 32, 48, 
	57, 48, 57, 32, 48, 57, 0, 32, 
	61, 9, 13, 0, 32, 61, 9, 13, 
	0, 32, 61, 9, 13, 32, 61, 0, 
	32, 61, 9, 13, 0, 32, 61, 9, 
	13, 0, 32, 9, 13, 0
};

static const char _mr_extractor_single_lengths[] = {
	0, 2, 2, 0, 1, 0, 1, 3, 
	3, 3, 2, 3, 3, 2
};

static const char _mr_extractor_range_lengths[] = {
	0, 0, 0, 1, 1, 1, 1, 1, 
	1, 1, 0, 1, 1, 1
};

static const char _mr_extractor_index_offsets[] = {
	0, 0, 3, 6, 8, 11, 13, 16, 
	21, 26, 31, 34, 39, 44
};

static const char _mr_extractor_indicies[] = {
	1, 1, 0, 1, 2, 0, 3, 1, 
	4, 5, 1, 6, 1, 7, 6, 1, 
	9, 10, 1, 9, 8, 11, 12, 1, 
	11, 8, 9, 10, 1, 9, 8, 1, 
	1, 8, 13, 14, 1, 13, 8, 11, 
	15, 1, 11, 8, 15, 15, 15, 1, 
	0
};

static const char _mr_extractor_trans_targs[] = {
	2, 0, 3, 4, 5, 4, 6, 8, 
	7, 9, 10, 11, 8, 11, 12, 13
};

static const char _mr_extractor_trans_actions[] = {
	0, 0, 1, 3, 5, 0, 0, 7, 
	0, 9, 9, 0, 0, 9, 9, 0
};

static const int mr_extractor_start = 1;

static const int mr_extractor_en_main = 1;


#line 121 "src/spss/readstat_sav_parse_mr_name.rl"


readstat_error_t extract_mr_data(const char *line, mr_set_t *result, sav_ctx_t *ctx) {
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
	cs = mr_extractor_start;
	}

#line 142 "src/spss/readstat_sav_parse_mr_name.rl"
    
#line 92 "src/spss/readstat_sav_parse_mr_name.c"
	{
	int _klen;
	unsigned int _trans;
	const char *_acts;
	unsigned int _nacts;
	const char *_keys;

	if ( p == pe )
		goto _test_eof;
	if ( cs == 0 )
		goto _out;
_resume:
	_keys = _mr_extractor_trans_keys + _mr_extractor_key_offsets[cs];
	_trans = _mr_extractor_index_offsets[cs];

	_klen = _mr_extractor_single_lengths[cs];
	if ( _klen > 0 ) {
		const char *_lower = _keys;
		const char *_mid;
		const char *_upper = _keys + _klen - 1;
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

	_klen = _mr_extractor_range_lengths[cs];
	if ( _klen > 0 ) {
		const char *_lower = _keys;
		const char *_mid;
		const char *_upper = _keys + (_klen<<1) - 2;
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
	_trans = _mr_extractor_indicies[_trans];
	cs = _mr_extractor_trans_targs[_trans];

	if ( _mr_extractor_trans_actions[_trans] == 0 )
		goto _again;

	_acts = _mr_extractor_actions + _mr_extractor_trans_actions[_trans];
	_nacts = (unsigned int) *_acts++;
	while ( _nacts-- > 0 )
	{
		switch ( *_acts++ )
		{
	case 0:
#line 13 "src/spss/readstat_sav_parse_mr_name.rl"
	{
        size_t src_len = p - start;
        size_t dst_len = 4 * src_len + 1;  // UTF-8 expansion: up to 4 bytes per char
        mr_name = (char *)readstat_malloc(dst_len);
        if (mr_name == NULL) {
            retval = READSTAT_ERROR_MALLOC;
            goto cleanup;
        }
        retval = readstat_convert(mr_name, dst_len, start, src_len, ctx->converter);
        if (retval != READSTAT_OK) {
            goto cleanup;
        }
    }
	break;
	case 1:
#line 27 "src/spss/readstat_sav_parse_mr_name.rl"
	{
        mr_type = *p;
        start = p + 1;
    }
	break;
	case 2:
#line 32 "src/spss/readstat_sav_parse_mr_name.rl"
	{
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
	break;
	case 3:
#line 61 "src/spss/readstat_sav_parse_mr_name.rl"
	{
        char *lbl_len_str = (char *)readstat_malloc(p - start + 1);
        if (lbl_len_str == NULL) {
            retval = READSTAT_ERROR_MALLOC;
            goto cleanup;
        }
        memcpy(lbl_len_str, start, p - start);
        lbl_len_str[p - start] = '\0';
        int len = strtol(lbl_len_str, NULL, 10);
        free(lbl_len_str);
        size_t dst_len = 4 * len + 1;  // UTF-8 expansion: up to 4 bytes per char
        mr_label = (char *)readstat_malloc(dst_len);
        if (mr_label == NULL) {
            retval = READSTAT_ERROR_MALLOC;
            goto cleanup;
        }
        retval = readstat_convert(mr_label, dst_len, p + 1, len, ctx->converter);
        if (retval != READSTAT_OK) {
            goto cleanup;
        }
        p = p + 1 + len;
        start = p + 1;
    }
	break;
	case 4:
#line 85 "src/spss/readstat_sav_parse_mr_name.rl"
	{
        size_t src_len = p - start;
        size_t dst_len = 4 * src_len + 1;  // UTF-8 expansion: up to 4 bytes per char
        char *subvar = (char *)readstat_malloc(dst_len);
        if (subvar == NULL) {
            retval = READSTAT_ERROR_MALLOC;
            goto cleanup;
        }
        retval = readstat_convert(subvar, dst_len, start, src_len, ctx->converter);
        if (retval != READSTAT_OK) {
            free(subvar);
            goto cleanup;
        }
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
	break;
#line 266 "src/spss/readstat_sav_parse_mr_name.c"
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

#line 143 "src/spss/readstat_sav_parse_mr_name.rl"

    // Check if FSM finished successfully
    if (cs < 8 || p != pe) {
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


readstat_error_t parse_mr_line(const char *line, mr_set_t *result, sav_ctx_t *ctx) {
    *result = (mr_set_t){0};
    return extract_mr_data(line, result, ctx);
}


#line 317 "src/spss/readstat_sav_parse_mr_name.c"
static const char _mr_parser_actions[] = {
	0, 1, 0
};

static const char _mr_parser_key_offsets[] = {
	0, 0, 1, 2, 4
};

static const char _mr_parser_trans_keys[] = {
	36, 10, 0, 10, 10, 0
};

static const char _mr_parser_single_lengths[] = {
	0, 1, 1, 2, 1
};

static const char _mr_parser_range_lengths[] = {
	0, 0, 0, 0, 0
};

static const char _mr_parser_index_offsets[] = {
	0, 0, 2, 4, 7
};

static const char _mr_parser_indicies[] = {
	0, 1, 2, 0, 3, 2, 0, 2, 
	0, 0
};

static const char _mr_parser_trans_targs[] = {
	2, 0, 3, 4
};

static const char _mr_parser_trans_actions[] = {
	0, 0, 1, 0
};

static const int mr_parser_start = 1;

static const int mr_parser_en_main = 1;


#line 216 "src/spss/readstat_sav_parse_mr_name.rl"


readstat_error_t parse_mr_string(const char *line, mr_set_t **mr_sets, size_t *n_mr_lines, sav_ctx_t *ctx) {
    readstat_error_t retval = READSTAT_OK;
    int cs = 0;
    char *p = (char *)line;
    char *start = p;
    char *pe = p + strlen(p) + 1;
    *mr_sets = NULL;
    *n_mr_lines = 0;

    
#line 369 "src/spss/readstat_sav_parse_mr_name.c"
	{
	cs = mr_parser_start;
	}

#line 228 "src/spss/readstat_sav_parse_mr_name.rl"
    
#line 372 "src/spss/readstat_sav_parse_mr_name.c"
	{
	int _klen;
	unsigned int _trans;
	const char *_acts;
	unsigned int _nacts;
	const char *_keys;

	if ( p == pe )
		goto _test_eof;
	if ( cs == 0 )
		goto _out;
_resume:
	_keys = _mr_parser_trans_keys + _mr_parser_key_offsets[cs];
	_trans = _mr_parser_index_offsets[cs];

	_klen = _mr_parser_single_lengths[cs];
	if ( _klen > 0 ) {
		const char *_lower = _keys;
		const char *_mid;
		const char *_upper = _keys + _klen - 1;
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

	_klen = _mr_parser_range_lengths[cs];
	if ( _klen > 0 ) {
		const char *_lower = _keys;
		const char *_mid;
		const char *_upper = _keys + (_klen<<1) - 2;
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
	_trans = _mr_parser_indicies[_trans];
	cs = _mr_parser_trans_targs[_trans];

	if ( _mr_parser_trans_actions[_trans] == 0 )
		goto _again;

	_acts = _mr_parser_actions + _mr_parser_trans_actions[_trans];
	_nacts = (unsigned int) *_acts++;
	while ( _nacts-- > 0 )
	{
		switch ( *_acts++ )
		{
	case 0:
#line 186 "src/spss/readstat_sav_parse_mr_name.rl"
	{
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
        retval = parse_mr_line(mln, &(*mr_sets)[*n_mr_lines], ctx);
        free(mln);
        if (retval != READSTAT_OK) {
            goto cleanup;
        }
        (*n_mr_lines)++;
        start = p + 1;
    }
	break;
#line 470 "src/spss/readstat_sav_parse_mr_name.c"
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

#line 229 "src/spss/readstat_sav_parse_mr_name.rl"

    if (cs < 4 || p != pe) {
        retval = READSTAT_ERROR_BAD_MR_STRING;
        goto cleanup;
    }

    (void)mr_parser_en_main;

cleanup:
    return retval;
}
