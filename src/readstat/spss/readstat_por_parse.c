#line 1 "src/spss/readstat_por_parse.rl"
#include <sys/types.h>

#include "../readstat.h"
#include "readstat_por_parse.h"

#define POR_PARSE_MAX_DIGITS 128


#line 11 "src/spss/readstat_por_parse.c"
static const signed char _por_field_parse_actions[] = {
	0, 1, 0, 1, 1, 1, 2, 1,
	3, 1, 4, 1, 5, 1, 7, 2,
	6, 7, 0
};

static const signed char _por_field_parse_key_offsets[] = {
	0, 0, 8, 9, 14, 18, 25, 29,
	34, 42, 0
};

static const char _por_field_parse_trans_keys[] = {
	32, 42, 45, 46, 48, 57, 65, 84,
	46, 46, 48, 57, 65, 84, 48, 57,
	65, 84, 43, 45, 47, 48, 57, 65,
	84, 48, 57, 65, 84, 47, 48, 57,
	65, 84, 43, 45, 46, 47, 48, 57,
	65, 84, 0
};

static const signed char _por_field_parse_single_lengths[] = {
	0, 4, 1, 1, 0, 3, 0, 1,
	4, 0, 0
};

static const signed char _por_field_parse_range_lengths[] = {
	0, 2, 0, 2, 2, 2, 2, 2,
	2, 0, 0
};

static const signed char _por_field_parse_index_offsets[] = {
	0, 0, 7, 9, 13, 16, 22, 25,
	29, 36, 0
};

static const signed char _por_field_parse_cond_targs[] = {
	1, 2, 3, 4, 8, 8, 0, 9,
	0, 4, 8, 8, 0, 5, 5, 0,
	6, 6, 9, 5, 5, 0, 7, 7,
	0, 9, 7, 7, 0, 6, 6, 5,
	9, 8, 8, 0, 0, 0, 1, 2,
	3, 4, 5, 6, 7, 8, 9, 0
};

static const signed char _por_field_parse_cond_actions[] = {
	0, 0, 11, 7, 1, 1, 0, 15,
	0, 7, 1, 1, 0, 1, 1, 0,
	0, 9, 13, 1, 1, 0, 3, 3,
	0, 13, 3, 3, 0, 0, 9, 5,
	13, 1, 1, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0
};

static const int por_field_parse_start = 1;

static const int por_field_parse_en_main = 1;


#line 11 "src/spss/readstat_por_parse.rl"


static int por_base30_digit_value(unsigned char c) {
	if (c >= '0' && c <= '9')
		return c - '0';
	return 10 + c - 'A';
}

ssize_t readstat_por_parse_double(const char *data, size_t len, double *result,
readstat_error_handler error_cb, void *user_ctx) {
	ssize_t retval = 0;
	double val = 0.0;
	
	/* Significant digits of the mantissa, most significant first; leading
	* zeros are dropped and digits beyond POR_PARSE_MAX_DIGITS are folded
	* into the scale. */
	unsigned char digits[POR_PARSE_MAX_DIGITS] = { 0 };
	size_t n_digits = 0;
	long scale = 0;
	long exp = 0;
	
	const unsigned char *p = (const unsigned char *)data;
	const unsigned char *pe = p + len;
	
	int cs;
	int is_negative = 0, exp_is_negative = 0;
	int got_dot = 0;
	int is_missing = 0;
	int success = 0;
	
	
#line 102 "src/spss/readstat_por_parse.c"
	{
		cs = (int)por_field_parse_start;
	}
	
#line 107 "src/spss/readstat_por_parse.c"
	{
		int _klen;
		unsigned int _trans = 0;
		const char * _keys;
		const signed char * _acts;
		unsigned int _nacts;
		_resume: {}
		if ( p == pe )
			goto _out;
		_keys = ( _por_field_parse_trans_keys + (_por_field_parse_key_offsets[cs]));
		_trans = (unsigned int)_por_field_parse_index_offsets[cs];
		
		_klen = (int)_por_field_parse_single_lengths[cs];
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
		
		_klen = (int)_por_field_parse_range_lengths[cs];
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
		cs = (int)_por_field_parse_cond_targs[_trans];
		
		if ( _por_field_parse_cond_actions[_trans] != 0 ) {
			
			_acts = ( _por_field_parse_actions + (_por_field_parse_cond_actions[_trans]));
			_nacts = (unsigned int)(*( _acts));
			_acts += 1;
			while ( _nacts > 0 ) {
				switch ( (*( _acts)) )
				{
					case 0:  {
						{
#line 42 "src/spss/readstat_por_parse.rl"
							
							int digit = por_base30_digit_value((( (*( p)))));
							if (n_digits == 0 && digit == 0) {
								if (got_dot)
								scale--;
							} else if (n_digits < POR_PARSE_MAX_DIGITS) {
								digits[n_digits++] = digit;
								if (got_dot)
								scale--;
							} else if (!got_dot) {
								scale++;
							}
						}
						
#line 195 "src/spss/readstat_por_parse.c"
						
						break; 
					}
					case 1:  {
						{
#line 56 "src/spss/readstat_por_parse.rl"
							
							if (exp < 100000)
							exp = 30 * exp + por_base30_digit_value((( (*( p)))));
						}
						
#line 207 "src/spss/readstat_por_parse.c"
						
						break; 
					}
					case 2:  {
						{
#line 63 "src/spss/readstat_por_parse.rl"
							got_dot = 1; }
						
#line 216 "src/spss/readstat_por_parse.c"
						
						break; 
					}
					case 3:  {
						{
#line 64 "src/spss/readstat_por_parse.rl"
							got_dot = 1; }
						
#line 225 "src/spss/readstat_por_parse.c"
						
						break; 
					}
					case 4:  {
						{
#line 66 "src/spss/readstat_por_parse.rl"
							exp_is_negative = 1; }
						
#line 234 "src/spss/readstat_por_parse.c"
						
						break; 
					}
					case 5:  {
						{
#line 68 "src/spss/readstat_por_parse.rl"
							is_negative = 1; }
						
#line 243 "src/spss/readstat_por_parse.c"
						
						break; 
					}
					case 6:  {
						{
#line 70 "src/spss/readstat_por_parse.rl"
							is_missing = 1; }
						
#line 252 "src/spss/readstat_por_parse.c"
						
						break; 
					}
					case 7:  {
						{
#line 72 "src/spss/readstat_por_parse.rl"
							success = 1; {p += 1; goto _out; } }
						
#line 261 "src/spss/readstat_por_parse.c"
						
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
	
#line 76 "src/spss/readstat_por_parse.rl"
	
	
	if (is_missing) {
		val = NAN;
	} else {
		if (exp_is_negative)
			exp = -exp;
		val = por_base30_to_double(digits, n_digits, scale + exp);
		if (is_negative)
			val = -val;
	}
	
	if (!success) {
		retval = -1;
		if (error_cb) {
			char error_buf[1024];
			snprintf(error_buf, sizeof(error_buf), "Read bytes: %ld   String: %.*s  Ending state: %d",
			(long)(p - (const unsigned char *)data), (int)len, data, cs);
			error_cb(error_buf, user_ctx);
		}
	}
	
	if (retval == 0) {
		if (result)
			*result = val;
		
		retval = (p - (const unsigned char *)data);
	}
	
	/* suppress warning */
	(void)por_field_parse_en_main;
	
	return retval;
}
