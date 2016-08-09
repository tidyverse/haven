
#line 1 "src/stata/readstat_dta_parse_timestamp.rl"

#include <time.h>
#include "../readstat.h"
#include "../readstat_iconv.h"

#include "readstat_dta.h"
#include "readstat_dta_parse_timestamp.h"


#line 13 "src/stata/readstat_dta_parse_timestamp.c"
static const char _dta_timestamp_parse_actions[] = {
	0, 1, 0, 1, 2, 1, 3, 1, 
	4, 1, 5, 1, 6, 1, 7, 1, 
	8, 1, 9, 1, 10, 1, 11, 1, 
	12, 1, 13, 1, 14, 1, 15, 1, 
	16, 1, 17, 2, 1, 0
};

static const char _dta_timestamp_parse_key_offsets[] = {
	0, 0, 3, 5, 8, 16, 18, 19, 
	20, 22, 25, 28, 31, 33, 34, 35, 
	36, 37, 38, 39, 40, 41, 43, 44, 
	45, 47, 48, 49, 50, 52, 53, 54, 
	55, 56, 57, 58, 59, 60, 61, 62, 
	63
};

static const char _dta_timestamp_parse_trans_keys[] = {
	32, 48, 57, 48, 57, 32, 48, 57, 
	65, 68, 70, 74, 77, 78, 79, 83, 
	112, 117, 114, 32, 48, 57, 32, 48, 
	57, 32, 48, 57, 58, 48, 57, 48, 
	57, 103, 32, 101, 99, 32, 101, 98, 
	32, 97, 117, 110, 32, 108, 110, 32, 
	32, 97, 114, 121, 32, 32, 111, 118, 
	32, 99, 116, 32, 101, 112, 32, 48, 
	57, 0
};

static const char _dta_timestamp_parse_single_lengths[] = {
	0, 1, 0, 1, 8, 2, 1, 1, 
	0, 1, 1, 1, 0, 1, 1, 1, 
	1, 1, 1, 1, 1, 2, 1, 1, 
	2, 1, 1, 1, 2, 1, 1, 1, 
	1, 1, 1, 1, 1, 1, 1, 1, 
	0
};

static const char _dta_timestamp_parse_range_lengths[] = {
	0, 1, 1, 1, 0, 0, 0, 0, 
	1, 1, 1, 1, 1, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	1
};

static const unsigned char _dta_timestamp_parse_index_offsets[] = {
	0, 0, 3, 5, 8, 17, 20, 22, 
	24, 26, 29, 32, 35, 37, 39, 41, 
	43, 45, 47, 49, 51, 53, 56, 58, 
	60, 63, 65, 67, 69, 72, 74, 76, 
	78, 80, 82, 84, 86, 88, 90, 92, 
	94
};

static const char _dta_timestamp_parse_trans_targs[] = {
	2, 3, 0, 3, 0, 4, 3, 0, 
	5, 15, 18, 21, 27, 31, 34, 37, 
	0, 6, 13, 0, 7, 0, 8, 0, 
	9, 0, 10, 9, 0, 10, 11, 0, 
	12, 11, 0, 40, 0, 14, 0, 8, 
	0, 16, 0, 17, 0, 8, 0, 19, 
	0, 20, 0, 8, 0, 22, 24, 0, 
	23, 0, 8, 0, 25, 26, 0, 8, 
	0, 8, 0, 28, 0, 29, 30, 0, 
	8, 0, 8, 0, 32, 0, 33, 0, 
	8, 0, 35, 0, 36, 0, 8, 0, 
	38, 0, 39, 0, 8, 0, 40, 0, 
	0
};

static const char _dta_timestamp_parse_trans_actions[] = {
	0, 35, 0, 35, 0, 3, 1, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 11, 0, 
	35, 0, 29, 1, 0, 0, 35, 0, 
	31, 1, 0, 35, 0, 0, 0, 19, 
	0, 0, 0, 0, 0, 27, 0, 0, 
	0, 0, 0, 7, 0, 0, 0, 0, 
	0, 0, 5, 0, 0, 0, 0, 17, 
	0, 15, 0, 0, 0, 0, 0, 0, 
	9, 0, 13, 0, 0, 0, 0, 0, 
	25, 0, 0, 0, 0, 0, 23, 0, 
	0, 0, 0, 0, 21, 0, 1, 0, 
	0
};

static const char _dta_timestamp_parse_eof_actions[] = {
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	33
};

static const int dta_timestamp_parse_start = 1;

static const int dta_timestamp_parse_en_main = 1;


#line 12 "src/stata/readstat_dta_parse_timestamp.rl"


readstat_error_t dta_parse_timestamp(const char *data, size_t len, struct tm *timestamp, dta_ctx_t *ctx) {
    readstat_error_t retval = READSTAT_OK;
    char error_buf[8192];
    const char *p = data;
    const char *pe = p + len;
    const char *eof = pe;
    int cs;
    int temp_val = 0;
    
#line 128 "src/stata/readstat_dta_parse_timestamp.c"
	{
	cs = dta_timestamp_parse_start;
	}

#line 133 "src/stata/readstat_dta_parse_timestamp.c"
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
	_keys = _dta_timestamp_parse_trans_keys + _dta_timestamp_parse_key_offsets[cs];
	_trans = _dta_timestamp_parse_index_offsets[cs];

	_klen = _dta_timestamp_parse_single_lengths[cs];
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

	_klen = _dta_timestamp_parse_range_lengths[cs];
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
	cs = _dta_timestamp_parse_trans_targs[_trans];

	if ( _dta_timestamp_parse_trans_actions[_trans] == 0 )
		goto _again;

	_acts = _dta_timestamp_parse_actions + _dta_timestamp_parse_trans_actions[_trans];
	_nacts = (unsigned int) *_acts++;
	while ( _nacts-- > 0 )
	{
		switch ( *_acts++ )
		{
	case 0:
#line 23 "src/stata/readstat_dta_parse_timestamp.rl"
	{
            temp_val = 10 * temp_val + ((*p) - '0');
        }
	break;
	case 1:
#line 27 "src/stata/readstat_dta_parse_timestamp.rl"
	{ temp_val = 0; }
	break;
	case 2:
#line 29 "src/stata/readstat_dta_parse_timestamp.rl"
	{ timestamp->tm_mday = temp_val; }
	break;
	case 3:
#line 32 "src/stata/readstat_dta_parse_timestamp.rl"
	{ timestamp->tm_mon = 0; }
	break;
	case 4:
#line 33 "src/stata/readstat_dta_parse_timestamp.rl"
	{ timestamp->tm_mon = 1; }
	break;
	case 5:
#line 34 "src/stata/readstat_dta_parse_timestamp.rl"
	{ timestamp->tm_mon = 2; }
	break;
	case 6:
#line 35 "src/stata/readstat_dta_parse_timestamp.rl"
	{ timestamp->tm_mon = 3; }
	break;
	case 7:
#line 36 "src/stata/readstat_dta_parse_timestamp.rl"
	{ timestamp->tm_mon = 4; }
	break;
	case 8:
#line 37 "src/stata/readstat_dta_parse_timestamp.rl"
	{ timestamp->tm_mon = 5; }
	break;
	case 9:
#line 38 "src/stata/readstat_dta_parse_timestamp.rl"
	{ timestamp->tm_mon = 6; }
	break;
	case 10:
#line 39 "src/stata/readstat_dta_parse_timestamp.rl"
	{ timestamp->tm_mon = 7; }
	break;
	case 11:
#line 40 "src/stata/readstat_dta_parse_timestamp.rl"
	{ timestamp->tm_mon = 8; }
	break;
	case 12:
#line 41 "src/stata/readstat_dta_parse_timestamp.rl"
	{ timestamp->tm_mon = 9; }
	break;
	case 13:
#line 42 "src/stata/readstat_dta_parse_timestamp.rl"
	{ timestamp->tm_mon = 10; }
	break;
	case 14:
#line 43 "src/stata/readstat_dta_parse_timestamp.rl"
	{ timestamp->tm_mon = 11; }
	break;
	case 15:
#line 45 "src/stata/readstat_dta_parse_timestamp.rl"
	{ timestamp->tm_year = temp_val - 1900; }
	break;
	case 16:
#line 47 "src/stata/readstat_dta_parse_timestamp.rl"
	{ timestamp->tm_hour = temp_val; }
	break;
#line 276 "src/stata/readstat_dta_parse_timestamp.c"
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
	const char *__acts = _dta_timestamp_parse_actions + _dta_timestamp_parse_eof_actions[cs];
	unsigned int __nacts = (unsigned int) *__acts++;
	while ( __nacts-- > 0 ) {
		switch ( *__acts++ ) {
	case 17:
#line 49 "src/stata/readstat_dta_parse_timestamp.rl"
	{ timestamp->tm_min = temp_val; }
	break;
#line 296 "src/stata/readstat_dta_parse_timestamp.c"
		}
	}
	}

	_out: {}
	}

#line 55 "src/stata/readstat_dta_parse_timestamp.rl"


    if (cs < 40|| p != pe) {
        if (ctx->error_handler) {
            snprintf(error_buf, sizeof(error_buf), "Invalid timestamp string (length=%d): %*s", (int)len, (int)-len, data);
            ctx->error_handler(error_buf, ctx->user_ctx);
        }
        retval = READSTAT_ERROR_BAD_TIMESTAMP;
    }

    (void)dta_timestamp_parse_en_main;

    return retval;
}
