
#line 1 "src/readstat_sav_parse_timestamp.rl"

#include <time.h>
#include "readstat.h"
#include "readstat_sav.h"
#include "readstat_sav_parse_timestamp.h"


#line 11 "src/readstat_sav_parse_timestamp.c"
static const char _sav_time_parse_actions[] = {
	0, 1, 0, 1, 2, 1, 3, 1, 
	4, 2, 1, 0
};

static const char _sav_time_parse_key_offsets[] = {
	0, 0, 2, 4, 5, 7, 9, 10, 
	12, 14
};

static const char _sav_time_parse_trans_keys[] = {
	48, 57, 48, 57, 58, 48, 57, 48, 
	57, 58, 48, 57, 48, 57, 0
};

static const char _sav_time_parse_single_lengths[] = {
	0, 0, 0, 1, 0, 0, 1, 0, 
	0, 0
};

static const char _sav_time_parse_range_lengths[] = {
	0, 1, 1, 0, 1, 1, 0, 1, 
	1, 0
};

static const char _sav_time_parse_index_offsets[] = {
	0, 0, 2, 4, 6, 8, 10, 12, 
	14, 16
};

static const char _sav_time_parse_trans_targs[] = {
	2, 0, 3, 0, 4, 0, 5, 0, 
	6, 0, 7, 0, 8, 0, 9, 0, 
	0, 0
};

static const char _sav_time_parse_trans_actions[] = {
	9, 0, 1, 0, 3, 0, 9, 0, 
	1, 0, 5, 0, 9, 0, 1, 0, 
	0, 0
};

static const char _sav_time_parse_eof_actions[] = {
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 7
};

static const int sav_time_parse_start = 1;

static const int sav_time_parse_en_main = 1;


#line 10 "src/readstat_sav_parse_timestamp.rl"


readstat_error_t sav_parse_time(const char *data, size_t len, struct tm *timestamp, sav_ctx_t *ctx) {
    readstat_error_t retval = READSTAT_OK;
    char error_buf[8192];
    const char *p = data;
    const char *pe = p + len;
    const char *eof = pe;
    int cs;
    int temp_val = 0;
    
#line 76 "src/readstat_sav_parse_timestamp.c"
	{
	cs = sav_time_parse_start;
	}

#line 81 "src/readstat_sav_parse_timestamp.c"
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
	_keys = _sav_time_parse_trans_keys + _sav_time_parse_key_offsets[cs];
	_trans = _sav_time_parse_index_offsets[cs];

	_klen = _sav_time_parse_single_lengths[cs];
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

	_klen = _sav_time_parse_range_lengths[cs];
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
	cs = _sav_time_parse_trans_targs[_trans];

	if ( _sav_time_parse_trans_actions[_trans] == 0 )
		goto _again;

	_acts = _sav_time_parse_actions + _sav_time_parse_trans_actions[_trans];
	_nacts = (unsigned int) *_acts++;
	while ( _nacts-- > 0 )
	{
		switch ( *_acts++ )
		{
	case 0:
#line 21 "src/readstat_sav_parse_timestamp.rl"
	{
            temp_val = 10 * temp_val + ((*p) - '0');
        }
	break;
	case 1:
#line 25 "src/readstat_sav_parse_timestamp.rl"
	{ temp_val = 0; }
	break;
	case 2:
#line 27 "src/readstat_sav_parse_timestamp.rl"
	{ timestamp->tm_hour = temp_val; }
	break;
	case 3:
#line 29 "src/readstat_sav_parse_timestamp.rl"
	{ timestamp->tm_min = temp_val; }
	break;
#line 172 "src/readstat_sav_parse_timestamp.c"
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
	const char *__acts = _sav_time_parse_actions + _sav_time_parse_eof_actions[cs];
	unsigned int __nacts = (unsigned int) *__acts++;
	while ( __nacts-- > 0 ) {
		switch ( *__acts++ ) {
	case 4:
#line 31 "src/readstat_sav_parse_timestamp.rl"
	{ timestamp->tm_sec = temp_val; }
	break;
#line 192 "src/readstat_sav_parse_timestamp.c"
		}
	}
	}

	_out: {}
	}

#line 37 "src/readstat_sav_parse_timestamp.rl"


    if (cs < 9|| p != pe) {
        if (ctx->error_handler) {
            snprintf(error_buf, sizeof(error_buf), "Invalid time string (length=%d): %*s", (int)len, (int)-len, data);
            ctx->error_handler(error_buf, ctx->user_ctx);
        }
        retval = READSTAT_ERROR_BAD_TIMESTAMP;
    }

    (void)sav_time_parse_en_main;

    return retval;
}


#line 217 "src/readstat_sav_parse_timestamp.c"
static const char _sav_date_parse_actions[] = {
	0, 1, 0, 1, 1, 1, 3, 1, 
	4, 1, 5, 1, 6, 1, 7, 1, 
	8, 1, 9, 1, 10, 1, 11, 1, 
	12, 1, 13, 1, 14, 1, 15, 2, 
	2, 0
};

static const char _sav_date_parse_key_offsets[] = {
	0, 0, 2, 4, 5, 13, 15, 16, 
	17, 19, 21, 22, 23, 24, 25, 26, 
	27, 28, 29, 31, 32, 33, 35, 36, 
	37, 38, 40, 41, 42, 43, 44, 45, 
	46, 47, 48, 49, 50, 51
};

static const char _sav_date_parse_trans_keys[] = {
	48, 57, 48, 57, 32, 65, 68, 70, 
	74, 77, 78, 79, 83, 112, 117, 114, 
	32, 48, 57, 48, 57, 103, 32, 101, 
	99, 32, 101, 98, 32, 97, 117, 110, 
	32, 108, 110, 32, 32, 97, 114, 121, 
	32, 32, 111, 118, 32, 99, 116, 32, 
	101, 112, 32, 0
};

static const char _sav_date_parse_single_lengths[] = {
	0, 0, 0, 1, 8, 2, 1, 1, 
	0, 0, 1, 1, 1, 1, 1, 1, 
	1, 1, 2, 1, 1, 2, 1, 1, 
	1, 2, 1, 1, 1, 1, 1, 1, 
	1, 1, 1, 1, 1, 0
};

static const char _sav_date_parse_range_lengths[] = {
	0, 1, 1, 0, 0, 0, 0, 0, 
	1, 1, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0
};

static const char _sav_date_parse_index_offsets[] = {
	0, 0, 2, 4, 6, 15, 18, 20, 
	22, 24, 26, 28, 30, 32, 34, 36, 
	38, 40, 42, 45, 47, 49, 52, 54, 
	56, 58, 61, 63, 65, 67, 69, 71, 
	73, 75, 77, 79, 81, 83
};

static const char _sav_date_parse_trans_targs[] = {
	2, 0, 3, 0, 4, 0, 5, 12, 
	15, 18, 24, 28, 31, 34, 0, 6, 
	10, 0, 7, 0, 8, 0, 9, 0, 
	37, 0, 11, 0, 8, 0, 13, 0, 
	14, 0, 8, 0, 16, 0, 17, 0, 
	8, 0, 19, 21, 0, 20, 0, 8, 
	0, 22, 23, 0, 8, 0, 8, 0, 
	25, 0, 26, 27, 0, 8, 0, 8, 
	0, 29, 0, 30, 0, 8, 0, 32, 
	0, 33, 0, 8, 0, 35, 0, 36, 
	0, 8, 0, 0, 0
};

static const char _sav_date_parse_trans_actions[] = {
	31, 0, 1, 0, 5, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 13, 0, 31, 0, 
	1, 0, 0, 0, 21, 0, 0, 0, 
	0, 0, 29, 0, 0, 0, 0, 0, 
	9, 0, 0, 0, 0, 0, 0, 7, 
	0, 0, 0, 0, 19, 0, 17, 0, 
	0, 0, 0, 0, 0, 11, 0, 15, 
	0, 0, 0, 0, 0, 27, 0, 0, 
	0, 0, 0, 25, 0, 0, 0, 0, 
	0, 23, 0, 0, 0
};

static const char _sav_date_parse_eof_actions[] = {
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 3
};

static const int sav_date_parse_start = 1;

static const int sav_date_parse_en_main = 1;


#line 55 "src/readstat_sav_parse_timestamp.rl"


readstat_error_t sav_parse_date(const char *data, size_t len, struct tm *timestamp, sav_ctx_t *ctx) {
    readstat_error_t retval = READSTAT_OK;
    char error_buf[8192];
    const char *p = data;
    const char *pe = p + len;
    const char *eof = pe;
    int cs;
    int temp_val = 0;
    
#line 321 "src/readstat_sav_parse_timestamp.c"
	{
	cs = sav_date_parse_start;
	}

#line 326 "src/readstat_sav_parse_timestamp.c"
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
	_keys = _sav_date_parse_trans_keys + _sav_date_parse_key_offsets[cs];
	_trans = _sav_date_parse_index_offsets[cs];

	_klen = _sav_date_parse_single_lengths[cs];
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

	_klen = _sav_date_parse_range_lengths[cs];
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
	cs = _sav_date_parse_trans_targs[_trans];

	if ( _sav_date_parse_trans_actions[_trans] == 0 )
		goto _again;

	_acts = _sav_date_parse_actions + _sav_date_parse_trans_actions[_trans];
	_nacts = (unsigned int) *_acts++;
	while ( _nacts-- > 0 )
	{
		switch ( *_acts++ )
		{
	case 0:
#line 66 "src/readstat_sav_parse_timestamp.rl"
	{
            temp_val = 10 * temp_val + ((*p) - '0');
        }
	break;
	case 2:
#line 78 "src/readstat_sav_parse_timestamp.rl"
	{ temp_val = 0; }
	break;
	case 3:
#line 80 "src/readstat_sav_parse_timestamp.rl"
	{ timestamp->tm_mday = temp_val; }
	break;
	case 4:
#line 85 "src/readstat_sav_parse_timestamp.rl"
	{ timestamp->tm_mon = 0; }
	break;
	case 5:
#line 86 "src/readstat_sav_parse_timestamp.rl"
	{ timestamp->tm_mon = 1; }
	break;
	case 6:
#line 87 "src/readstat_sav_parse_timestamp.rl"
	{ timestamp->tm_mon = 2; }
	break;
	case 7:
#line 88 "src/readstat_sav_parse_timestamp.rl"
	{ timestamp->tm_mon = 3; }
	break;
	case 8:
#line 89 "src/readstat_sav_parse_timestamp.rl"
	{ timestamp->tm_mon = 4; }
	break;
	case 9:
#line 90 "src/readstat_sav_parse_timestamp.rl"
	{ timestamp->tm_mon = 5; }
	break;
	case 10:
#line 91 "src/readstat_sav_parse_timestamp.rl"
	{ timestamp->tm_mon = 6; }
	break;
	case 11:
#line 92 "src/readstat_sav_parse_timestamp.rl"
	{ timestamp->tm_mon = 7; }
	break;
	case 12:
#line 93 "src/readstat_sav_parse_timestamp.rl"
	{ timestamp->tm_mon = 8; }
	break;
	case 13:
#line 94 "src/readstat_sav_parse_timestamp.rl"
	{ timestamp->tm_mon = 9; }
	break;
	case 14:
#line 95 "src/readstat_sav_parse_timestamp.rl"
	{ timestamp->tm_mon = 10; }
	break;
	case 15:
#line 96 "src/readstat_sav_parse_timestamp.rl"
	{ timestamp->tm_mon = 11; }
	break;
#line 461 "src/readstat_sav_parse_timestamp.c"
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
	const char *__acts = _sav_date_parse_actions + _sav_date_parse_eof_actions[cs];
	unsigned int __nacts = (unsigned int) *__acts++;
	while ( __nacts-- > 0 ) {
		switch ( *__acts++ ) {
	case 1:
#line 70 "src/readstat_sav_parse_timestamp.rl"
	{
            if (temp_val < 70) {
                timestamp->tm_year = 100 + temp_val;
            } else {
                timestamp->tm_year = temp_val;
            }
        }
	break;
#line 487 "src/readstat_sav_parse_timestamp.c"
		}
	}
	}

	_out: {}
	}

#line 102 "src/readstat_sav_parse_timestamp.rl"


    if (cs < 37|| p != pe) {
        if (ctx->error_handler) {
            snprintf(error_buf, sizeof(error_buf), "Invalid date string (length=%d): %*s", (int)len, (int)-len, data);
            ctx->error_handler(error_buf, ctx->user_ctx);
        }
        retval = READSTAT_ERROR_BAD_TIMESTAMP;
    }

    (void)sav_date_parse_en_main;

    return retval;
}
