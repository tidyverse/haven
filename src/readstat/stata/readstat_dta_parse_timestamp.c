
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
	0, 0, 3, 5, 8, 16, 20, 21, 
	22, 24, 27, 30, 33, 35, 36, 37, 
	38, 39, 41, 42, 43, 44, 46, 47, 
	48, 49, 53, 54, 55, 57, 58, 59, 
	60, 62, 64, 66, 67, 68, 70, 72, 
	73, 74, 75, 77, 78, 79, 80, 82, 
	83, 84, 85
};

static const char _dta_timestamp_parse_trans_keys[] = {
	32, 48, 57, 48, 57, 32, 48, 57, 
	65, 68, 70, 74, 77, 78, 79, 83, 
	80, 85, 112, 117, 82, 32, 48, 57, 
	32, 48, 57, 32, 48, 57, 58, 48, 
	57, 48, 57, 71, 32, 114, 103, 69, 
	101, 67, 32, 99, 69, 101, 66, 32, 
	98, 65, 85, 97, 117, 78, 32, 76, 
	78, 32, 32, 110, 108, 110, 65, 97, 
	82, 89, 32, 32, 114, 121, 79, 111, 
	86, 32, 118, 67, 99, 84, 32, 116, 
	69, 101, 80, 32, 112, 48, 57, 0
};

static const char _dta_timestamp_parse_single_lengths[] = {
	0, 1, 0, 1, 8, 4, 1, 1, 
	0, 1, 1, 1, 0, 1, 1, 1, 
	1, 2, 1, 1, 1, 2, 1, 1, 
	1, 4, 1, 1, 2, 1, 1, 1, 
	2, 2, 2, 1, 1, 2, 2, 1, 
	1, 1, 2, 1, 1, 1, 2, 1, 
	1, 1, 0
};

static const char _dta_timestamp_parse_range_lengths[] = {
	0, 1, 1, 1, 0, 0, 0, 0, 
	1, 1, 1, 1, 1, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 1
};

static const unsigned char _dta_timestamp_parse_index_offsets[] = {
	0, 0, 3, 5, 8, 17, 22, 24, 
	26, 28, 31, 34, 37, 39, 41, 43, 
	45, 47, 50, 52, 54, 56, 59, 61, 
	63, 65, 70, 72, 74, 77, 79, 81, 
	83, 86, 89, 92, 94, 96, 99, 102, 
	104, 106, 108, 111, 113, 115, 117, 120, 
	122, 124, 126
};

static const char _dta_timestamp_parse_trans_targs[] = {
	2, 3, 0, 3, 0, 4, 3, 0, 
	5, 17, 21, 25, 33, 38, 42, 46, 
	0, 6, 13, 15, 16, 0, 7, 0, 
	8, 0, 9, 0, 10, 9, 0, 10, 
	11, 0, 12, 11, 0, 50, 0, 14, 
	0, 8, 0, 7, 0, 14, 0, 18, 
	20, 0, 19, 0, 8, 0, 19, 0, 
	22, 24, 0, 23, 0, 8, 0, 23, 
	0, 26, 28, 31, 32, 0, 27, 0, 
	8, 0, 29, 30, 0, 8, 0, 8, 
	0, 27, 0, 29, 30, 0, 34, 37, 
	0, 35, 36, 0, 8, 0, 8, 0, 
	35, 36, 0, 39, 41, 0, 40, 0, 
	8, 0, 40, 0, 43, 45, 0, 44, 
	0, 8, 0, 44, 0, 47, 49, 0, 
	48, 0, 8, 0, 48, 0, 50, 0, 
	0
};

static const char _dta_timestamp_parse_trans_actions[] = {
	0, 35, 0, 35, 0, 3, 1, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	11, 0, 35, 0, 29, 1, 0, 0, 
	35, 0, 31, 1, 0, 35, 0, 0, 
	0, 19, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 27, 0, 0, 0, 
	0, 0, 0, 0, 0, 7, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	5, 0, 0, 0, 0, 17, 0, 15, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 9, 0, 13, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	25, 0, 0, 0, 0, 0, 0, 0, 
	0, 23, 0, 0, 0, 0, 0, 0, 
	0, 0, 21, 0, 0, 0, 1, 0, 
	0
};

static const char _dta_timestamp_parse_eof_actions[] = {
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 33
};

static const int dta_timestamp_parse_start = 1;

static const int dta_timestamp_parse_en_main = 1;


#line 12 "src/stata/readstat_dta_parse_timestamp.rl"


readstat_error_t dta_parse_timestamp(const char *data, size_t len, struct tm *timestamp, dta_ctx_t *ctx) {
    readstat_error_t retval = READSTAT_OK;
    const char *p = data;
    const char *pe = p + len;
    const char *eof = pe;
    int cs;
    int temp_val = 0;
    
#line 142 "src/stata/readstat_dta_parse_timestamp.c"
	{
	cs = dta_timestamp_parse_start;
	}

#line 147 "src/stata/readstat_dta_parse_timestamp.c"
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
#line 22 "src/stata/readstat_dta_parse_timestamp.rl"
	{
            temp_val = 10 * temp_val + ((*p) - '0');
        }
	break;
	case 1:
#line 26 "src/stata/readstat_dta_parse_timestamp.rl"
	{ temp_val = 0; }
	break;
	case 2:
#line 28 "src/stata/readstat_dta_parse_timestamp.rl"
	{ timestamp->tm_mday = temp_val; }
	break;
	case 3:
#line 31 "src/stata/readstat_dta_parse_timestamp.rl"
	{ timestamp->tm_mon = 0; }
	break;
	case 4:
#line 32 "src/stata/readstat_dta_parse_timestamp.rl"
	{ timestamp->tm_mon = 1; }
	break;
	case 5:
#line 33 "src/stata/readstat_dta_parse_timestamp.rl"
	{ timestamp->tm_mon = 2; }
	break;
	case 6:
#line 34 "src/stata/readstat_dta_parse_timestamp.rl"
	{ timestamp->tm_mon = 3; }
	break;
	case 7:
#line 35 "src/stata/readstat_dta_parse_timestamp.rl"
	{ timestamp->tm_mon = 4; }
	break;
	case 8:
#line 36 "src/stata/readstat_dta_parse_timestamp.rl"
	{ timestamp->tm_mon = 5; }
	break;
	case 9:
#line 37 "src/stata/readstat_dta_parse_timestamp.rl"
	{ timestamp->tm_mon = 6; }
	break;
	case 10:
#line 38 "src/stata/readstat_dta_parse_timestamp.rl"
	{ timestamp->tm_mon = 7; }
	break;
	case 11:
#line 39 "src/stata/readstat_dta_parse_timestamp.rl"
	{ timestamp->tm_mon = 8; }
	break;
	case 12:
#line 40 "src/stata/readstat_dta_parse_timestamp.rl"
	{ timestamp->tm_mon = 9; }
	break;
	case 13:
#line 41 "src/stata/readstat_dta_parse_timestamp.rl"
	{ timestamp->tm_mon = 10; }
	break;
	case 14:
#line 42 "src/stata/readstat_dta_parse_timestamp.rl"
	{ timestamp->tm_mon = 11; }
	break;
	case 15:
#line 44 "src/stata/readstat_dta_parse_timestamp.rl"
	{ timestamp->tm_year = temp_val - 1900; }
	break;
	case 16:
#line 46 "src/stata/readstat_dta_parse_timestamp.rl"
	{ timestamp->tm_hour = temp_val; }
	break;
#line 290 "src/stata/readstat_dta_parse_timestamp.c"
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
#line 48 "src/stata/readstat_dta_parse_timestamp.rl"
	{ timestamp->tm_min = temp_val; }
	break;
#line 310 "src/stata/readstat_dta_parse_timestamp.c"
		}
	}
	}

	_out: {}
	}

#line 54 "src/stata/readstat_dta_parse_timestamp.rl"


    if (cs < 50|| p != pe) {
        if (ctx->error_handler) {
            snprintf(ctx->error_buf, sizeof(ctx->error_buf), "Invalid timestamp string (length=%d): %*s", (int)len, (int)-len, data);
            ctx->error_handler(ctx->error_buf, ctx->user_ctx);
        }
        retval = READSTAT_ERROR_BAD_TIMESTAMP;
    }

    (void)dta_timestamp_parse_en_main;

    return retval;
}
