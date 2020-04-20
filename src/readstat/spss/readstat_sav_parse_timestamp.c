
#line 1 "src/spss/readstat_sav_parse_timestamp.rl"

#include <time.h>
#include "../readstat.h"
#include "../readstat_iconv.h"

#include "readstat_sav.h"
#include "readstat_sav_parse_timestamp.h"


#line 13 "src/spss/readstat_sav_parse_timestamp.c"
static const char _sav_time_parse_actions[] = {
	0, 1, 0, 1, 2, 1, 3, 1, 
	4, 1, 5, 2, 1, 0
};

static const char _sav_time_parse_key_offsets[] = {
	0, 0, 3, 5, 6, 9, 11, 12, 
	15, 17, 19, 21, 23
};

static const char _sav_time_parse_trans_keys[] = {
	32, 48, 57, 48, 57, 58, 32, 48, 
	57, 48, 57, 58, 32, 48, 57, 48, 
	57, 48, 57, 48, 57, 48, 57, 0
};

static const char _sav_time_parse_single_lengths[] = {
	0, 1, 0, 1, 1, 0, 1, 1, 
	0, 0, 0, 0, 0
};

static const char _sav_time_parse_range_lengths[] = {
	0, 1, 1, 0, 1, 1, 0, 1, 
	1, 1, 1, 1, 0
};

static const char _sav_time_parse_index_offsets[] = {
	0, 0, 3, 5, 7, 10, 12, 14, 
	17, 19, 21, 23, 25
};

static const char _sav_time_parse_trans_targs[] = {
	2, 11, 0, 3, 0, 4, 0, 5, 
	10, 0, 6, 0, 7, 0, 8, 9, 
	0, 12, 0, 12, 0, 6, 0, 3, 
	0, 0, 0
};

static const char _sav_time_parse_trans_actions[] = {
	0, 3, 0, 11, 0, 5, 0, 0, 
	3, 0, 11, 0, 7, 0, 0, 3, 
	0, 11, 0, 1, 0, 1, 0, 1, 
	0, 0, 0
};

static const char _sav_time_parse_eof_actions[] = {
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 9
};

static const int sav_time_parse_start = 1;

static const int sav_time_parse_en_main = 1;


#line 12 "src/spss/readstat_sav_parse_timestamp.rl"


readstat_error_t sav_parse_time(const char *data, size_t len, struct tm *timestamp,
        readstat_error_handler error_cb, void *user_ctx) {
    readstat_error_t retval = READSTAT_OK;
    char error_buf[8192];
    const char *p = data;
    const char *pe = p + len;
    const char *eof = pe;
    int cs;
    int temp_val = 0;
    
#line 82 "src/spss/readstat_sav_parse_timestamp.c"
	{
	cs = sav_time_parse_start;
	}

#line 87 "src/spss/readstat_sav_parse_timestamp.c"
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
#line 24 "src/spss/readstat_sav_parse_timestamp.rl"
	{
            temp_val = 10 * temp_val + ((*p) - '0');
        }
	break;
	case 1:
#line 28 "src/spss/readstat_sav_parse_timestamp.rl"
	{ temp_val = 0; }
	break;
	case 2:
#line 28 "src/spss/readstat_sav_parse_timestamp.rl"
	{ temp_val = (*p) - '0'; }
	break;
	case 3:
#line 30 "src/spss/readstat_sav_parse_timestamp.rl"
	{ timestamp->tm_hour = temp_val; }
	break;
	case 4:
#line 32 "src/spss/readstat_sav_parse_timestamp.rl"
	{ timestamp->tm_min = temp_val; }
	break;
#line 182 "src/spss/readstat_sav_parse_timestamp.c"
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
	case 5:
#line 34 "src/spss/readstat_sav_parse_timestamp.rl"
	{ timestamp->tm_sec = temp_val; }
	break;
#line 202 "src/spss/readstat_sav_parse_timestamp.c"
		}
	}
	}

	_out: {}
	}

#line 40 "src/spss/readstat_sav_parse_timestamp.rl"


    if (cs < 12|| p != pe) {
        if (error_cb) {
            snprintf(error_buf, sizeof(error_buf),
                "Invalid time string (length=%d): %.*s", (int)len, (int)len, data);
            error_cb(error_buf, user_ctx);
        }
        retval = READSTAT_ERROR_BAD_TIMESTAMP_STRING;
    }

    (void)sav_time_parse_en_main;

    return retval;
}


#line 228 "src/spss/readstat_sav_parse_timestamp.c"
static const char _sav_date_parse_actions[] = {
	0, 1, 0, 1, 1, 1, 3, 1, 
	4, 1, 5, 1, 6, 1, 7, 1, 
	8, 1, 9, 1, 10, 1, 11, 1, 
	12, 1, 13, 1, 14, 1, 15, 2, 
	2, 0
};

static const char _sav_date_parse_key_offsets[] = {
	0, 0, 3, 6, 7, 15, 19, 20, 
	21, 24, 27, 28, 29, 30, 31, 33, 
	34, 35, 36, 38, 39, 40, 41, 45, 
	46, 47, 49, 50, 51, 52, 54, 56, 
	58, 59, 60, 62, 64, 65, 66, 67, 
	69, 70, 71, 72, 74, 75, 76, 77
};

static const char _sav_date_parse_trans_keys[] = {
	32, 48, 57, 32, 48, 57, 32, 65, 
	68, 70, 74, 77, 78, 79, 83, 80, 
	85, 112, 117, 82, 32, 32, 48, 57, 
	32, 48, 57, 71, 32, 114, 103, 69, 
	101, 67, 32, 99, 69, 101, 66, 32, 
	98, 65, 85, 97, 117, 78, 32, 76, 
	78, 32, 32, 110, 108, 110, 65, 97, 
	82, 89, 32, 32, 114, 121, 79, 111, 
	86, 32, 118, 67, 99, 84, 32, 116, 
	69, 101, 80, 32, 112, 0
};

static const char _sav_date_parse_single_lengths[] = {
	0, 1, 1, 1, 8, 4, 1, 1, 
	1, 1, 1, 1, 1, 1, 2, 1, 
	1, 1, 2, 1, 1, 1, 4, 1, 
	1, 2, 1, 1, 1, 2, 2, 2, 
	1, 1, 2, 2, 1, 1, 1, 2, 
	1, 1, 1, 2, 1, 1, 1, 0
};

static const char _sav_date_parse_range_lengths[] = {
	0, 1, 1, 0, 0, 0, 0, 0, 
	1, 1, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0
};

static const unsigned char _sav_date_parse_index_offsets[] = {
	0, 0, 3, 6, 8, 17, 22, 24, 
	26, 29, 32, 34, 36, 38, 40, 43, 
	45, 47, 49, 52, 54, 56, 58, 63, 
	65, 67, 70, 72, 74, 76, 79, 82, 
	85, 87, 89, 92, 95, 97, 99, 101, 
	104, 106, 108, 110, 113, 115, 117, 119
};

static const char _sav_date_parse_indicies[] = {
	0, 0, 1, 2, 2, 1, 3, 1, 
	4, 5, 6, 7, 8, 9, 10, 11, 
	1, 12, 13, 14, 15, 1, 16, 1, 
	17, 1, 18, 18, 1, 19, 19, 1, 
	20, 1, 21, 1, 16, 1, 20, 1, 
	22, 23, 1, 24, 1, 25, 1, 24, 
	1, 26, 27, 1, 28, 1, 29, 1, 
	28, 1, 30, 31, 32, 33, 1, 34, 
	1, 35, 1, 36, 37, 1, 38, 1, 
	39, 1, 34, 1, 36, 37, 1, 40, 
	41, 1, 42, 43, 1, 44, 1, 45, 
	1, 42, 43, 1, 46, 47, 1, 48, 
	1, 49, 1, 48, 1, 50, 51, 1, 
	52, 1, 53, 1, 52, 1, 54, 55, 
	1, 56, 1, 57, 1, 56, 1, 1, 
	0
};

static const char _sav_date_parse_trans_targs[] = {
	2, 0, 3, 4, 5, 14, 18, 22, 
	30, 35, 39, 43, 6, 10, 12, 13, 
	7, 8, 9, 47, 11, 8, 15, 17, 
	16, 8, 19, 21, 20, 8, 23, 25, 
	28, 29, 24, 8, 26, 27, 8, 8, 
	31, 34, 32, 33, 8, 8, 36, 38, 
	37, 8, 40, 42, 41, 8, 44, 46, 
	45, 8
};

static const char _sav_date_parse_trans_actions[] = {
	31, 0, 1, 5, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 13, 31, 1, 0, 21, 0, 0, 
	0, 29, 0, 0, 0, 9, 0, 0, 
	0, 0, 0, 7, 0, 0, 19, 17, 
	0, 0, 0, 0, 11, 15, 0, 0, 
	0, 27, 0, 0, 0, 25, 0, 0, 
	0, 23
};

static const char _sav_date_parse_eof_actions[] = {
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 3
};

static const int sav_date_parse_start = 1;

static const int sav_date_parse_en_main = 1;


#line 59 "src/spss/readstat_sav_parse_timestamp.rl"


readstat_error_t sav_parse_date(const char *data, size_t len, struct tm *timestamp,
        readstat_error_handler error_cb, void *user_ctx) {
    readstat_error_t retval = READSTAT_OK;
    char error_buf[8192];
    const char *p = data;
    const char *pe = p + len;
    const char *eof = pe;
    int cs;
    int temp_val = 0;
    
#line 354 "src/spss/readstat_sav_parse_timestamp.c"
	{
	cs = sav_date_parse_start;
	}

#line 359 "src/spss/readstat_sav_parse_timestamp.c"
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
	_trans = _sav_date_parse_indicies[_trans];
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
#line 71 "src/spss/readstat_sav_parse_timestamp.rl"
	{
            char digit = ((*p) - '0');
            if (digit >= 0 && digit <= 9) {
                temp_val = 10 * temp_val + digit;
            }
        }
	break;
	case 2:
#line 87 "src/spss/readstat_sav_parse_timestamp.rl"
	{ temp_val = 0; }
	break;
	case 3:
#line 89 "src/spss/readstat_sav_parse_timestamp.rl"
	{ timestamp->tm_mday = temp_val; }
	break;
	case 4:
#line 94 "src/spss/readstat_sav_parse_timestamp.rl"
	{ timestamp->tm_mon = 0; }
	break;
	case 5:
#line 95 "src/spss/readstat_sav_parse_timestamp.rl"
	{ timestamp->tm_mon = 1; }
	break;
	case 6:
#line 96 "src/spss/readstat_sav_parse_timestamp.rl"
	{ timestamp->tm_mon = 2; }
	break;
	case 7:
#line 97 "src/spss/readstat_sav_parse_timestamp.rl"
	{ timestamp->tm_mon = 3; }
	break;
	case 8:
#line 98 "src/spss/readstat_sav_parse_timestamp.rl"
	{ timestamp->tm_mon = 4; }
	break;
	case 9:
#line 99 "src/spss/readstat_sav_parse_timestamp.rl"
	{ timestamp->tm_mon = 5; }
	break;
	case 10:
#line 100 "src/spss/readstat_sav_parse_timestamp.rl"
	{ timestamp->tm_mon = 6; }
	break;
	case 11:
#line 101 "src/spss/readstat_sav_parse_timestamp.rl"
	{ timestamp->tm_mon = 7; }
	break;
	case 12:
#line 102 "src/spss/readstat_sav_parse_timestamp.rl"
	{ timestamp->tm_mon = 8; }
	break;
	case 13:
#line 103 "src/spss/readstat_sav_parse_timestamp.rl"
	{ timestamp->tm_mon = 9; }
	break;
	case 14:
#line 104 "src/spss/readstat_sav_parse_timestamp.rl"
	{ timestamp->tm_mon = 10; }
	break;
	case 15:
#line 105 "src/spss/readstat_sav_parse_timestamp.rl"
	{ timestamp->tm_mon = 11; }
	break;
#line 498 "src/spss/readstat_sav_parse_timestamp.c"
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
#line 78 "src/spss/readstat_sav_parse_timestamp.rl"
	{
            if (temp_val < 70) {
                timestamp->tm_year = 100 + temp_val;
            } else {
                timestamp->tm_year = temp_val;
            }
        }
	break;
#line 524 "src/spss/readstat_sav_parse_timestamp.c"
		}
	}
	}

	_out: {}
	}

#line 111 "src/spss/readstat_sav_parse_timestamp.rl"


    if (cs < 47|| p != pe) {
        if (error_cb) {
            snprintf(error_buf, sizeof(error_buf),
                "Invalid date string (length=%d): %.*s", (int)len, (int)len, data);
            error_cb(error_buf, user_ctx);
        }
        retval = READSTAT_ERROR_BAD_TIMESTAMP_STRING;
    }

    (void)sav_date_parse_en_main;

    return retval;
}
