
#line 1 "src/spss/readstat_spss_parse.rl"

#include <stdlib.h>
#include "../readstat.h"

#include "readstat_spss.h"
#include "readstat_spss_parse.h"


#line 12 "src/spss/readstat_spss_parse.c"
static const char _spss_format_parser_actions[] = {
	0, 1, 1, 1, 2, 1, 3, 1, 
	4, 1, 5, 1, 6, 1, 7, 1, 
	8, 1, 9, 1, 10, 1, 11, 1, 
	12, 1, 13, 1, 14, 1, 15, 1, 
	16, 1, 17, 1, 18, 1, 19, 1, 
	20, 1, 21, 1, 22, 1, 23, 1, 
	24, 1, 25, 1, 26, 1, 27, 1, 
	28, 1, 29, 1, 30, 1, 31, 1, 
	32, 1, 33, 1, 34, 1, 35, 1, 
	36, 1, 37, 1, 38, 1, 39, 1, 
	40, 2, 0, 1, 3, 4, 0, 1, 
	3, 5, 0, 1, 3, 6, 0, 1, 
	3, 7, 0, 1, 3, 8, 0, 1, 
	3, 9, 0, 1, 3, 10, 0, 1, 
	3, 11, 0, 1, 3, 12, 0, 1, 
	3, 13, 0, 1, 3, 14, 0, 1, 
	3, 15, 0, 1, 3, 16, 0, 1, 
	3, 17, 0, 1, 3, 18, 0, 1, 
	3, 19, 0, 1, 3, 20, 0, 1, 
	3, 21, 0, 1, 3, 22, 0, 1, 
	3, 23, 0, 1, 3, 24, 0, 1, 
	3, 25, 0, 1, 3, 26, 0, 1, 
	3, 27, 0, 1, 3, 28, 0, 1, 
	3, 29, 0, 1, 3, 30, 0, 1, 
	3, 31, 0, 1, 3, 32, 0, 1, 
	3, 33, 0, 1, 3, 34, 0, 1, 
	3, 35, 0, 1, 3, 36, 0, 1, 
	3, 37, 0, 1, 3, 38, 0, 1, 
	3, 39, 0, 1, 3, 40, 0, 1
	
};

static const short _spss_format_parser_key_offsets[] = {
	0, 0, 34, 36, 38, 40, 42, 44, 
	46, 50, 60, 62, 64, 66, 72, 74, 
	76, 78, 80, 82, 86, 88, 90, 92, 
	94, 96, 98, 100, 102, 104, 106, 108, 
	110, 112, 114, 118, 122, 124, 126, 128, 
	130, 132, 134, 136, 138, 140, 142, 144, 
	146, 148, 150, 152, 154, 156, 158, 160, 
	162, 164, 166, 168, 172, 174, 176, 178, 
	180, 182, 184, 186, 188, 194, 197, 199, 
	201, 203, 205, 207, 209, 211, 213, 215, 
	219, 221, 223, 225, 227, 231, 233, 235, 
	237, 239, 241, 243, 245, 247, 255, 257, 
	261, 263, 265, 267, 271, 273, 275, 277, 
	279, 281, 283
};

static const char _spss_format_parser_trans_keys[] = {
	65, 67, 68, 69, 70, 73, 74, 77, 
	78, 80, 81, 82, 83, 84, 87, 89, 
	90, 97, 99, 100, 101, 102, 105, 106, 
	109, 110, 112, 113, 114, 115, 116, 119, 
	121, 122, 48, 57, 65, 97, 84, 116, 
	69, 101, 69, 101, 88, 120, 67, 79, 
	99, 111, 65, 66, 67, 68, 69, 97, 
	98, 99, 100, 101, 77, 109, 77, 109, 
	65, 97, 65, 79, 84, 97, 111, 116, 
	84, 116, 69, 101, 73, 105, 77, 109, 
	69, 101, 76, 84, 108, 116, 76, 108, 
	65, 97, 82, 114, 73, 105, 77, 109, 
	69, 101, 65, 97, 84, 116, 69, 101, 
	66, 98, 68, 100, 65, 97, 84, 116, 
	69, 101, 79, 84, 111, 116, 78, 89, 
	110, 121, 84, 116, 72, 104, 82, 114, 
	73, 105, 77, 109, 69, 101, 84, 116, 
	66, 98, 69, 101, 88, 120, 89, 121, 
	82, 114, 66, 98, 69, 101, 88, 120, 
	68, 100, 65, 97, 84, 116, 69, 101, 
	73, 105, 77, 109, 69, 101, 75, 107, 
	68, 89, 100, 121, 65, 97, 89, 121, 
	82, 114, 77, 109, 68, 100, 72, 104, 
	77, 109, 83, 115, 68, 72, 100, 104, 
	48, 57, 46, 48, 57, 48, 57, 48, 
	57, 48, 57, 48, 57, 48, 57, 48, 
	57, 48, 57, 48, 57, 48, 57, 84, 
	116, 48, 57, 48, 57, 48, 57, 48, 
	57, 48, 57, 68, 100, 48, 57, 48, 
	57, 48, 57, 48, 57, 48, 57, 48, 
	57, 48, 57, 48, 57, 48, 57, 67, 
	73, 75, 99, 105, 107, 48, 57, 48, 
	57, 72, 104, 48, 57, 48, 57, 48, 
	57, 48, 57, 72, 104, 48, 57, 48, 
	57, 48, 57, 48, 57, 48, 57, 48, 
	57, 48, 57, 48, 57, 0
};

static const char _spss_format_parser_single_lengths[] = {
	0, 34, 0, 2, 2, 2, 2, 2, 
	4, 10, 2, 2, 2, 6, 2, 2, 
	2, 2, 2, 4, 2, 2, 2, 2, 
	2, 2, 2, 2, 2, 2, 2, 2, 
	2, 2, 4, 4, 2, 2, 2, 2, 
	2, 2, 2, 2, 2, 2, 2, 2, 
	2, 2, 2, 2, 2, 2, 2, 2, 
	2, 2, 2, 4, 2, 2, 2, 2, 
	2, 2, 2, 2, 4, 1, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 2, 
	0, 0, 0, 0, 2, 0, 0, 0, 
	0, 0, 0, 0, 0, 6, 0, 2, 
	0, 0, 0, 2, 0, 0, 0, 0, 
	0, 0, 0
};

static const char _spss_format_parser_range_lengths[] = {
	0, 0, 1, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 1, 1, 1, 1, 
	1, 1, 1, 1, 1, 1, 1, 1, 
	1, 1, 1, 1, 1, 1, 1, 1, 
	1, 1, 1, 1, 1, 1, 1, 1, 
	1, 1, 1, 1, 1, 1, 1, 1, 
	1, 1, 1
};

static const short _spss_format_parser_index_offsets[] = {
	0, 0, 35, 37, 40, 43, 46, 49, 
	52, 57, 68, 71, 74, 77, 84, 87, 
	90, 93, 96, 99, 104, 107, 110, 113, 
	116, 119, 122, 125, 128, 131, 134, 137, 
	140, 143, 146, 151, 156, 159, 162, 165, 
	168, 171, 174, 177, 180, 183, 186, 189, 
	192, 195, 198, 201, 204, 207, 210, 213, 
	216, 219, 222, 225, 230, 233, 236, 239, 
	242, 245, 248, 251, 254, 260, 263, 265, 
	267, 269, 271, 273, 275, 277, 279, 281, 
	285, 287, 289, 291, 293, 297, 299, 301, 
	303, 305, 307, 309, 311, 313, 321, 323, 
	327, 329, 331, 333, 337, 339, 341, 343, 
	345, 347, 349
};

static const unsigned char _spss_format_parser_indicies[] = {
	0, 2, 3, 4, 5, 6, 7, 8, 
	9, 10, 11, 12, 13, 14, 15, 16, 
	17, 0, 2, 3, 4, 5, 6, 7, 
	8, 9, 10, 11, 12, 13, 14, 15, 
	16, 17, 1, 18, 1, 19, 19, 1, 
	20, 20, 1, 21, 21, 1, 22, 22, 
	1, 23, 23, 1, 24, 25, 24, 25, 
	1, 26, 27, 28, 29, 30, 26, 27, 
	28, 29, 30, 1, 31, 31, 1, 32, 
	32, 1, 33, 33, 1, 34, 35, 36, 
	34, 35, 36, 1, 37, 37, 1, 38, 
	38, 1, 39, 39, 1, 40, 40, 1, 
	41, 41, 1, 42, 43, 42, 43, 1, 
	44, 44, 1, 45, 45, 1, 46, 46, 
	1, 47, 47, 1, 48, 48, 1, 49, 
	49, 1, 50, 50, 1, 51, 51, 1, 
	52, 52, 1, 53, 53, 1, 54, 54, 
	1, 55, 55, 1, 56, 56, 1, 57, 
	57, 1, 58, 59, 58, 59, 1, 60, 
	61, 60, 61, 1, 62, 62, 1, 63, 
	63, 1, 64, 64, 1, 65, 65, 1, 
	66, 66, 1, 67, 67, 1, 68, 68, 
	1, 69, 69, 1, 70, 70, 1, 71, 
	71, 1, 72, 72, 1, 73, 73, 1, 
	74, 74, 1, 75, 75, 1, 76, 76, 
	1, 77, 77, 1, 78, 78, 1, 79, 
	79, 1, 80, 80, 1, 81, 81, 1, 
	82, 82, 1, 83, 83, 1, 84, 84, 
	1, 85, 86, 85, 86, 1, 87, 87, 
	1, 88, 88, 1, 89, 89, 1, 90, 
	90, 1, 91, 91, 1, 92, 92, 1, 
	93, 93, 1, 94, 94, 1, 96, 97, 
	96, 97, 95, 1, 98, 99, 1, 100, 
	1, 101, 1, 102, 1, 103, 1, 104, 
	1, 105, 1, 106, 1, 107, 1, 108, 
	1, 110, 110, 109, 1, 111, 1, 112, 
	1, 113, 1, 114, 1, 116, 116, 115, 
	1, 117, 1, 118, 1, 119, 1, 120, 
	1, 121, 1, 122, 1, 123, 1, 124, 
	1, 126, 127, 128, 126, 127, 128, 125, 
	1, 129, 1, 131, 131, 130, 1, 132, 
	1, 133, 1, 134, 1, 136, 136, 135, 
	1, 137, 1, 138, 1, 139, 1, 140, 
	1, 141, 1, 142, 1, 143, 1, 0
};

static const char _spss_format_parser_trans_targs[] = {
	68, 0, 8, 13, 84, 86, 29, 30, 
	34, 92, 93, 46, 48, 51, 55, 58, 
	63, 106, 70, 4, 5, 71, 7, 72, 
	9, 10, 73, 74, 75, 76, 77, 11, 
	12, 78, 14, 19, 23, 15, 79, 17, 
	18, 80, 20, 82, 21, 22, 81, 24, 
	25, 83, 27, 28, 85, 87, 31, 32, 
	33, 88, 35, 39, 36, 38, 37, 89, 
	90, 40, 41, 91, 94, 95, 45, 96, 
	47, 98, 99, 50, 100, 52, 53, 54, 
	101, 56, 57, 102, 59, 60, 62, 61, 
	103, 104, 64, 65, 66, 67, 105, 69, 
	3, 6, 2, 69, 70, 69, 69, 69, 
	69, 69, 69, 69, 69, 69, 16, 69, 
	69, 69, 69, 69, 26, 69, 69, 69, 
	69, 69, 69, 69, 69, 69, 42, 43, 
	97, 69, 69, 44, 69, 69, 69, 69, 
	49, 69, 69, 69, 69, 69, 69, 69
};

static const unsigned char _spss_format_parser_trans_actions[] = {
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 81, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 84, 
	0, 0, 3, 1, 1, 160, 88, 204, 
	208, 212, 216, 220, 92, 144, 0, 152, 
	96, 200, 168, 140, 0, 224, 100, 104, 
	164, 180, 184, 172, 136, 112, 0, 0, 
	0, 196, 116, 0, 108, 120, 188, 124, 
	0, 128, 228, 148, 176, 192, 156, 132
};

static const unsigned char _spss_format_parser_eof_actions[] = {
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 7, 3, 5, 45, 
	9, 67, 69, 71, 73, 75, 11, 37, 
	41, 13, 65, 49, 35, 77, 15, 17, 
	47, 55, 57, 51, 33, 21, 63, 23, 
	19, 25, 59, 27, 29, 79, 39, 53, 
	61, 43, 31
};

static const int spss_format_parser_start = 1;

static const int spss_format_parser_en_main = 1;


#line 11 "src/spss/readstat_spss_parse.rl"


readstat_error_t spss_parse_format(const char *data, int count, spss_format_t *fmt) {
    unsigned char *p = (unsigned char *)data;
    unsigned char *pe = (unsigned char *)data + count;
    unsigned char *eof = pe;

    int cs;
    unsigned int integer = 0;

    
#line 276 "src/spss/readstat_spss_parse.c"
	{
	cs = spss_format_parser_start;
	}

#line 281 "src/spss/readstat_spss_parse.c"
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
	_keys = _spss_format_parser_trans_keys + _spss_format_parser_key_offsets[cs];
	_trans = _spss_format_parser_index_offsets[cs];

	_klen = _spss_format_parser_single_lengths[cs];
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

	_klen = _spss_format_parser_range_lengths[cs];
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
	_trans = _spss_format_parser_indicies[_trans];
	cs = _spss_format_parser_trans_targs[_trans];

	if ( _spss_format_parser_trans_actions[_trans] == 0 )
		goto _again;

	_acts = _spss_format_parser_actions + _spss_format_parser_trans_actions[_trans];
	_nacts = (unsigned int) *_acts++;
	while ( _nacts-- > 0 )
	{
		switch ( *_acts++ )
		{
	case 0:
#line 22 "src/spss/readstat_spss_parse.rl"
	{
            integer = 0;
        }
	break;
	case 1:
#line 26 "src/spss/readstat_spss_parse.rl"
	{
            integer = 10 * integer + ((*p) - '0');
        }
	break;
	case 2:
#line 30 "src/spss/readstat_spss_parse.rl"
	{
            fmt->width = integer;
        }
	break;
	case 4:
#line 38 "src/spss/readstat_spss_parse.rl"
	{ fmt->type = SPSS_FORMAT_TYPE_A; }
	break;
	case 5:
#line 39 "src/spss/readstat_spss_parse.rl"
	{ fmt->type = SPSS_FORMAT_TYPE_AHEX; }
	break;
	case 6:
#line 40 "src/spss/readstat_spss_parse.rl"
	{ fmt->type = SPSS_FORMAT_TYPE_COMMA; }
	break;
	case 7:
#line 41 "src/spss/readstat_spss_parse.rl"
	{ fmt->type = SPSS_FORMAT_TYPE_DOLLAR; }
	break;
	case 8:
#line 42 "src/spss/readstat_spss_parse.rl"
	{ fmt->type = SPSS_FORMAT_TYPE_F; }
	break;
	case 9:
#line 43 "src/spss/readstat_spss_parse.rl"
	{ fmt->type = SPSS_FORMAT_TYPE_IB; }
	break;
	case 10:
#line 44 "src/spss/readstat_spss_parse.rl"
	{ fmt->type = SPSS_FORMAT_TYPE_PIBHEX; }
	break;
	case 11:
#line 45 "src/spss/readstat_spss_parse.rl"
	{ fmt->type = SPSS_FORMAT_TYPE_P; }
	break;
	case 12:
#line 46 "src/spss/readstat_spss_parse.rl"
	{ fmt->type = SPSS_FORMAT_TYPE_PIB; }
	break;
	case 13:
#line 47 "src/spss/readstat_spss_parse.rl"
	{ fmt->type = SPSS_FORMAT_TYPE_PK; }
	break;
	case 14:
#line 48 "src/spss/readstat_spss_parse.rl"
	{ fmt->type = SPSS_FORMAT_TYPE_RB; }
	break;
	case 15:
#line 49 "src/spss/readstat_spss_parse.rl"
	{ fmt->type = SPSS_FORMAT_TYPE_RBHEX; }
	break;
	case 16:
#line 50 "src/spss/readstat_spss_parse.rl"
	{ fmt->type = SPSS_FORMAT_TYPE_Z; }
	break;
	case 17:
#line 51 "src/spss/readstat_spss_parse.rl"
	{ fmt->type = SPSS_FORMAT_TYPE_N; }
	break;
	case 18:
#line 52 "src/spss/readstat_spss_parse.rl"
	{ fmt->type = SPSS_FORMAT_TYPE_E; }
	break;
	case 19:
#line 53 "src/spss/readstat_spss_parse.rl"
	{ fmt->type = SPSS_FORMAT_TYPE_DATE; }
	break;
	case 20:
#line 54 "src/spss/readstat_spss_parse.rl"
	{ fmt->type = SPSS_FORMAT_TYPE_TIME; }
	break;
	case 21:
#line 55 "src/spss/readstat_spss_parse.rl"
	{ fmt->type = SPSS_FORMAT_TYPE_DATETIME; }
	break;
	case 22:
#line 56 "src/spss/readstat_spss_parse.rl"
	{ fmt->type = SPSS_FORMAT_TYPE_YMDHMS; }
	break;
	case 23:
#line 57 "src/spss/readstat_spss_parse.rl"
	{ fmt->type = SPSS_FORMAT_TYPE_ADATE; }
	break;
	case 24:
#line 58 "src/spss/readstat_spss_parse.rl"
	{ fmt->type = SPSS_FORMAT_TYPE_JDATE; }
	break;
	case 25:
#line 59 "src/spss/readstat_spss_parse.rl"
	{ fmt->type = SPSS_FORMAT_TYPE_DTIME; }
	break;
	case 26:
#line 60 "src/spss/readstat_spss_parse.rl"
	{ fmt->type = SPSS_FORMAT_TYPE_MTIME; }
	break;
	case 27:
#line 61 "src/spss/readstat_spss_parse.rl"
	{ fmt->type = SPSS_FORMAT_TYPE_WKDAY; }
	break;
	case 28:
#line 62 "src/spss/readstat_spss_parse.rl"
	{ fmt->type = SPSS_FORMAT_TYPE_MONTH; }
	break;
	case 29:
#line 63 "src/spss/readstat_spss_parse.rl"
	{ fmt->type = SPSS_FORMAT_TYPE_MOYR; }
	break;
	case 30:
#line 64 "src/spss/readstat_spss_parse.rl"
	{ fmt->type = SPSS_FORMAT_TYPE_QYR; }
	break;
	case 31:
#line 65 "src/spss/readstat_spss_parse.rl"
	{ fmt->type = SPSS_FORMAT_TYPE_WKYR; }
	break;
	case 32:
#line 66 "src/spss/readstat_spss_parse.rl"
	{ fmt->type = SPSS_FORMAT_TYPE_PCT; }
	break;
	case 33:
#line 67 "src/spss/readstat_spss_parse.rl"
	{ fmt->type = SPSS_FORMAT_TYPE_DOT; }
	break;
	case 34:
#line 68 "src/spss/readstat_spss_parse.rl"
	{ fmt->type = SPSS_FORMAT_TYPE_CCA; }
	break;
	case 35:
#line 69 "src/spss/readstat_spss_parse.rl"
	{ fmt->type = SPSS_FORMAT_TYPE_CCB; }
	break;
	case 36:
#line 70 "src/spss/readstat_spss_parse.rl"
	{ fmt->type = SPSS_FORMAT_TYPE_CCC; }
	break;
	case 37:
#line 71 "src/spss/readstat_spss_parse.rl"
	{ fmt->type = SPSS_FORMAT_TYPE_CCD; }
	break;
	case 38:
#line 72 "src/spss/readstat_spss_parse.rl"
	{ fmt->type = SPSS_FORMAT_TYPE_CCE; }
	break;
	case 39:
#line 73 "src/spss/readstat_spss_parse.rl"
	{ fmt->type = SPSS_FORMAT_TYPE_EDATE; }
	break;
	case 40:
#line 74 "src/spss/readstat_spss_parse.rl"
	{ fmt->type = SPSS_FORMAT_TYPE_SDATE; }
	break;
#line 521 "src/spss/readstat_spss_parse.c"
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
	const char *__acts = _spss_format_parser_actions + _spss_format_parser_eof_actions[cs];
	unsigned int __nacts = (unsigned int) *__acts++;
	while ( __nacts-- > 0 ) {
		switch ( *__acts++ ) {
	case 2:
#line 30 "src/spss/readstat_spss_parse.rl"
	{
            fmt->width = integer;
        }
	break;
	case 3:
#line 34 "src/spss/readstat_spss_parse.rl"
	{
            fmt->decimal_places = integer;
        }
	break;
	case 4:
#line 38 "src/spss/readstat_spss_parse.rl"
	{ fmt->type = SPSS_FORMAT_TYPE_A; }
	break;
	case 5:
#line 39 "src/spss/readstat_spss_parse.rl"
	{ fmt->type = SPSS_FORMAT_TYPE_AHEX; }
	break;
	case 6:
#line 40 "src/spss/readstat_spss_parse.rl"
	{ fmt->type = SPSS_FORMAT_TYPE_COMMA; }
	break;
	case 7:
#line 41 "src/spss/readstat_spss_parse.rl"
	{ fmt->type = SPSS_FORMAT_TYPE_DOLLAR; }
	break;
	case 8:
#line 42 "src/spss/readstat_spss_parse.rl"
	{ fmt->type = SPSS_FORMAT_TYPE_F; }
	break;
	case 9:
#line 43 "src/spss/readstat_spss_parse.rl"
	{ fmt->type = SPSS_FORMAT_TYPE_IB; }
	break;
	case 10:
#line 44 "src/spss/readstat_spss_parse.rl"
	{ fmt->type = SPSS_FORMAT_TYPE_PIBHEX; }
	break;
	case 11:
#line 45 "src/spss/readstat_spss_parse.rl"
	{ fmt->type = SPSS_FORMAT_TYPE_P; }
	break;
	case 12:
#line 46 "src/spss/readstat_spss_parse.rl"
	{ fmt->type = SPSS_FORMAT_TYPE_PIB; }
	break;
	case 13:
#line 47 "src/spss/readstat_spss_parse.rl"
	{ fmt->type = SPSS_FORMAT_TYPE_PK; }
	break;
	case 14:
#line 48 "src/spss/readstat_spss_parse.rl"
	{ fmt->type = SPSS_FORMAT_TYPE_RB; }
	break;
	case 15:
#line 49 "src/spss/readstat_spss_parse.rl"
	{ fmt->type = SPSS_FORMAT_TYPE_RBHEX; }
	break;
	case 16:
#line 50 "src/spss/readstat_spss_parse.rl"
	{ fmt->type = SPSS_FORMAT_TYPE_Z; }
	break;
	case 17:
#line 51 "src/spss/readstat_spss_parse.rl"
	{ fmt->type = SPSS_FORMAT_TYPE_N; }
	break;
	case 18:
#line 52 "src/spss/readstat_spss_parse.rl"
	{ fmt->type = SPSS_FORMAT_TYPE_E; }
	break;
	case 19:
#line 53 "src/spss/readstat_spss_parse.rl"
	{ fmt->type = SPSS_FORMAT_TYPE_DATE; }
	break;
	case 20:
#line 54 "src/spss/readstat_spss_parse.rl"
	{ fmt->type = SPSS_FORMAT_TYPE_TIME; }
	break;
	case 21:
#line 55 "src/spss/readstat_spss_parse.rl"
	{ fmt->type = SPSS_FORMAT_TYPE_DATETIME; }
	break;
	case 22:
#line 56 "src/spss/readstat_spss_parse.rl"
	{ fmt->type = SPSS_FORMAT_TYPE_YMDHMS; }
	break;
	case 23:
#line 57 "src/spss/readstat_spss_parse.rl"
	{ fmt->type = SPSS_FORMAT_TYPE_ADATE; }
	break;
	case 24:
#line 58 "src/spss/readstat_spss_parse.rl"
	{ fmt->type = SPSS_FORMAT_TYPE_JDATE; }
	break;
	case 25:
#line 59 "src/spss/readstat_spss_parse.rl"
	{ fmt->type = SPSS_FORMAT_TYPE_DTIME; }
	break;
	case 26:
#line 60 "src/spss/readstat_spss_parse.rl"
	{ fmt->type = SPSS_FORMAT_TYPE_MTIME; }
	break;
	case 27:
#line 61 "src/spss/readstat_spss_parse.rl"
	{ fmt->type = SPSS_FORMAT_TYPE_WKDAY; }
	break;
	case 28:
#line 62 "src/spss/readstat_spss_parse.rl"
	{ fmt->type = SPSS_FORMAT_TYPE_MONTH; }
	break;
	case 29:
#line 63 "src/spss/readstat_spss_parse.rl"
	{ fmt->type = SPSS_FORMAT_TYPE_MOYR; }
	break;
	case 30:
#line 64 "src/spss/readstat_spss_parse.rl"
	{ fmt->type = SPSS_FORMAT_TYPE_QYR; }
	break;
	case 31:
#line 65 "src/spss/readstat_spss_parse.rl"
	{ fmt->type = SPSS_FORMAT_TYPE_WKYR; }
	break;
	case 32:
#line 66 "src/spss/readstat_spss_parse.rl"
	{ fmt->type = SPSS_FORMAT_TYPE_PCT; }
	break;
	case 33:
#line 67 "src/spss/readstat_spss_parse.rl"
	{ fmt->type = SPSS_FORMAT_TYPE_DOT; }
	break;
	case 34:
#line 68 "src/spss/readstat_spss_parse.rl"
	{ fmt->type = SPSS_FORMAT_TYPE_CCA; }
	break;
	case 35:
#line 69 "src/spss/readstat_spss_parse.rl"
	{ fmt->type = SPSS_FORMAT_TYPE_CCB; }
	break;
	case 36:
#line 70 "src/spss/readstat_spss_parse.rl"
	{ fmt->type = SPSS_FORMAT_TYPE_CCC; }
	break;
	case 37:
#line 71 "src/spss/readstat_spss_parse.rl"
	{ fmt->type = SPSS_FORMAT_TYPE_CCD; }
	break;
	case 38:
#line 72 "src/spss/readstat_spss_parse.rl"
	{ fmt->type = SPSS_FORMAT_TYPE_CCE; }
	break;
	case 39:
#line 73 "src/spss/readstat_spss_parse.rl"
	{ fmt->type = SPSS_FORMAT_TYPE_EDATE; }
	break;
	case 40:
#line 74 "src/spss/readstat_spss_parse.rl"
	{ fmt->type = SPSS_FORMAT_TYPE_SDATE; }
	break;
#line 697 "src/spss/readstat_spss_parse.c"
		}
	}
	}

	_out: {}
	}

#line 87 "src/spss/readstat_spss_parse.rl"


    /* suppress warning */
    (void)spss_format_parser_en_main;

    if (cs < 68 || p != eof) {
        return READSTAT_ERROR_PARSE;
    }

    return READSTAT_OK;
}
