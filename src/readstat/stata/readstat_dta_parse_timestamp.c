
#line 1 "src/stata/readstat_dta_parse_timestamp.rl"

#include <time.h>
#include "../readstat.h"
#include "readstat_dta_parse_timestamp.h"


#line 10 "src/stata/readstat_dta_parse_timestamp.c"
static const char _dta_timestamp_parse_actions[] = {
	0, 1, 0, 1, 2, 1, 3, 1, 
	4, 1, 5, 1, 6, 1, 7, 1, 
	8, 1, 9, 1, 10, 1, 11, 1, 
	12, 1, 13, 1, 14, 1, 15, 1, 
	16, 1, 17, 2, 1, 0
};

static const char _dta_timestamp_parse_key_offsets[] = {
	0, 0, 3, 5, 8, 26, 34, 36, 
	37, 39, 42, 45, 48, 50, 52, 53, 
	55, 59, 63, 64, 66, 68, 70, 71, 
	73, 75, 76, 80, 82, 86, 87, 88, 
	90, 96, 97, 98, 100, 102, 103, 107, 
	109, 110, 112, 114, 115
};

static const char _dta_timestamp_parse_trans_keys[] = {
	32, 48, 57, 48, 57, 32, 48, 57, 
	65, 68, 69, 70, 74, 77, 78, 79, 
	83, 97, 100, 101, 102, 106, 109, 110, 
	111, 115, 66, 71, 80, 85, 98, 103, 
	112, 117, 82, 114, 32, 48, 57, 32, 
	48, 57, 32, 48, 57, 58, 48, 57, 
	48, 57, 79, 111, 32, 71, 103, 69, 
	73, 101, 105, 67, 90, 99, 122, 32, 
	67, 99, 78, 110, 69, 101, 32, 69, 
	101, 66, 98, 32, 65, 85, 97, 117, 
	78, 110, 76, 78, 108, 110, 32, 32, 
	65, 97, 73, 82, 89, 105, 114, 121, 
	32, 32, 79, 111, 86, 118, 32, 67, 
	75, 99, 107, 84, 116, 32, 69, 101, 
	80, 112, 32, 48, 57, 0
};

static const char _dta_timestamp_parse_single_lengths[] = {
	0, 1, 0, 1, 18, 8, 2, 1, 
	0, 1, 1, 1, 0, 2, 1, 2, 
	4, 4, 1, 2, 2, 2, 1, 2, 
	2, 1, 4, 2, 4, 1, 1, 2, 
	6, 1, 1, 2, 2, 1, 4, 2, 
	1, 2, 2, 1, 0
};

static const char _dta_timestamp_parse_range_lengths[] = {
	0, 1, 1, 1, 0, 0, 0, 0, 
	1, 1, 1, 1, 1, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 1
};

static const unsigned char _dta_timestamp_parse_index_offsets[] = {
	0, 0, 3, 5, 8, 27, 36, 39, 
	41, 43, 46, 49, 52, 54, 57, 59, 
	62, 67, 72, 74, 77, 80, 83, 85, 
	88, 91, 93, 98, 101, 106, 108, 110, 
	113, 120, 122, 124, 127, 130, 132, 137, 
	140, 142, 145, 148, 150
};

static const char _dta_timestamp_parse_indicies[] = {
	0, 2, 1, 2, 1, 3, 4, 1, 
	5, 6, 7, 8, 9, 10, 11, 12, 
	13, 5, 6, 7, 8, 9, 10, 11, 
	12, 13, 1, 14, 15, 14, 16, 14, 
	15, 14, 16, 1, 17, 17, 1, 18, 
	1, 19, 1, 20, 21, 1, 22, 23, 
	1, 25, 24, 1, 26, 1, 27, 27, 
	1, 28, 1, 27, 27, 1, 29, 30, 
	29, 30, 1, 31, 31, 31, 31, 1, 
	32, 1, 31, 31, 1, 33, 33, 1, 
	34, 34, 1, 35, 1, 36, 36, 1, 
	37, 37, 1, 38, 1, 39, 40, 39, 
	40, 1, 34, 34, 1, 41, 42, 41, 
	42, 1, 43, 1, 44, 1, 45, 45, 
	1, 46, 47, 46, 46, 47, 46, 1, 
	48, 1, 49, 1, 50, 50, 1, 51, 
	51, 1, 52, 1, 53, 53, 53, 53, 
	1, 54, 54, 1, 55, 1, 56, 56, 
	1, 57, 57, 1, 58, 1, 59, 1, 
	0
};

static const char _dta_timestamp_parse_trans_targs[] = {
	2, 0, 3, 4, 3, 5, 16, 20, 
	23, 26, 31, 35, 38, 41, 6, 13, 
	15, 7, 8, 9, 10, 9, 10, 11, 
	11, 12, 44, 14, 8, 17, 19, 18, 
	8, 21, 22, 8, 24, 25, 8, 27, 
	28, 29, 30, 8, 8, 32, 33, 34, 
	8, 8, 36, 37, 8, 39, 40, 8, 
	42, 43, 8, 44
};

static const char _dta_timestamp_parse_trans_actions[] = {
	0, 0, 35, 3, 1, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 11, 35, 29, 1, 0, 35, 
	1, 31, 35, 0, 19, 0, 0, 0, 
	27, 0, 0, 5, 0, 0, 7, 0, 
	0, 0, 0, 17, 15, 0, 0, 0, 
	13, 9, 0, 0, 25, 0, 0, 23, 
	0, 0, 21, 1
};

static const char _dta_timestamp_parse_eof_actions[] = {
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 33
};

static const int dta_timestamp_parse_start = 1;

static const int dta_timestamp_parse_en_main = 1;


#line 9 "src/stata/readstat_dta_parse_timestamp.rl"


readstat_error_t dta_parse_timestamp(const char *data, size_t len, struct tm *timestamp,
        readstat_error_handler error_handler, void *user_ctx) {
    readstat_error_t retval = READSTAT_OK;
    const char *p = data;
    const char *pe = p + len;
    const char *eof = pe;
    int cs;
    unsigned int temp_val = 0;
    
#line 144 "src/stata/readstat_dta_parse_timestamp.c"
	{
	cs = dta_timestamp_parse_start;
	}

#line 149 "src/stata/readstat_dta_parse_timestamp.c"
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
	_trans = _dta_timestamp_parse_indicies[_trans];
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
#line 20 "src/stata/readstat_dta_parse_timestamp.rl"
	{
            temp_val = 10 * temp_val + ((*p) - '0');
        }
	break;
	case 1:
#line 24 "src/stata/readstat_dta_parse_timestamp.rl"
	{ temp_val = 0; }
	break;
	case 2:
#line 26 "src/stata/readstat_dta_parse_timestamp.rl"
	{ timestamp->tm_mday = temp_val; }
	break;
	case 3:
#line 29 "src/stata/readstat_dta_parse_timestamp.rl"
	{ timestamp->tm_mon = 0; }
	break;
	case 4:
#line 30 "src/stata/readstat_dta_parse_timestamp.rl"
	{ timestamp->tm_mon = 1; }
	break;
	case 5:
#line 31 "src/stata/readstat_dta_parse_timestamp.rl"
	{ timestamp->tm_mon = 2; }
	break;
	case 6:
#line 32 "src/stata/readstat_dta_parse_timestamp.rl"
	{ timestamp->tm_mon = 3; }
	break;
	case 7:
#line 33 "src/stata/readstat_dta_parse_timestamp.rl"
	{ timestamp->tm_mon = 4; }
	break;
	case 8:
#line 34 "src/stata/readstat_dta_parse_timestamp.rl"
	{ timestamp->tm_mon = 5; }
	break;
	case 9:
#line 35 "src/stata/readstat_dta_parse_timestamp.rl"
	{ timestamp->tm_mon = 6; }
	break;
	case 10:
#line 36 "src/stata/readstat_dta_parse_timestamp.rl"
	{ timestamp->tm_mon = 7; }
	break;
	case 11:
#line 37 "src/stata/readstat_dta_parse_timestamp.rl"
	{ timestamp->tm_mon = 8; }
	break;
	case 12:
#line 38 "src/stata/readstat_dta_parse_timestamp.rl"
	{ timestamp->tm_mon = 9; }
	break;
	case 13:
#line 39 "src/stata/readstat_dta_parse_timestamp.rl"
	{ timestamp->tm_mon = 10; }
	break;
	case 14:
#line 40 "src/stata/readstat_dta_parse_timestamp.rl"
	{ timestamp->tm_mon = 11; }
	break;
	case 15:
#line 42 "src/stata/readstat_dta_parse_timestamp.rl"
	{ timestamp->tm_year = temp_val - 1900; }
	break;
	case 16:
#line 44 "src/stata/readstat_dta_parse_timestamp.rl"
	{ timestamp->tm_hour = temp_val; }
	break;
#line 293 "src/stata/readstat_dta_parse_timestamp.c"
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
#line 46 "src/stata/readstat_dta_parse_timestamp.rl"
	{ timestamp->tm_min = temp_val; }
	break;
#line 313 "src/stata/readstat_dta_parse_timestamp.c"
		}
	}
	}

	_out: {}
	}

#line 52 "src/stata/readstat_dta_parse_timestamp.rl"


    if (cs < 44|| p != pe) {
        char error_buf[1024];
        if (error_handler) {
            snprintf(error_buf, sizeof(error_buf), "Invalid timestamp string (length=%d): %.*s", (int)len, (int)len, data);
            error_handler(error_buf, user_ctx);
        }
        retval = READSTAT_ERROR_BAD_TIMESTAMP_STRING;
    }

    (void)dta_timestamp_parse_en_main;

    return retval;
}
