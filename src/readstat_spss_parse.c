
#line 1 "src/readstat_spss_parse.rl"

#include <stdlib.h>
#include "readstat.h"
#include "readstat_spss.h"
#include "readstat_spss_parse.h"


#line 11 "src/readstat_spss_parse.c"
static const int spss_format_parser_start = 1;

static const int spss_format_parser_en_main = 1;


#line 10 "src/readstat_spss_parse.rl"


readstat_error_t spss_parse_format(const char *data, int count, spss_format_t *fmt) {
    unsigned char *p = (unsigned char *)data;
    unsigned char *pe = (unsigned char *)data + count;
    unsigned char *eof = pe;

    int cs;
    int integer = 0;

    
#line 29 "src/readstat_spss_parse.c"
	{
	cs = spss_format_parser_start;
	}

#line 34 "src/readstat_spss_parse.c"
	{
	if ( p == pe )
		goto _test_eof;
	switch ( cs )
	{
case 1:
	switch( (*p) ) {
		case 65: goto st60;
		case 67: goto st8;
		case 68: goto st13;
		case 69: goto st76;
		case 70: goto st78;
		case 73: goto st29;
		case 74: goto st30;
		case 77: goto st34;
		case 78: goto st83;
		case 80: goto st84;
		case 81: goto st43;
		case 82: goto st45;
		case 83: goto st48;
		case 84: goto st52;
		case 87: goto st55;
		case 90: goto st96;
		case 97: goto st60;
		case 99: goto st8;
		case 100: goto st13;
		case 101: goto st76;
		case 102: goto st78;
		case 105: goto st29;
		case 106: goto st30;
		case 109: goto st34;
		case 110: goto st83;
		case 112: goto st84;
		case 113: goto st43;
		case 114: goto st45;
		case 115: goto st48;
		case 116: goto st52;
		case 119: goto st55;
		case 122: goto st96;
	}
	goto st0;
st0:
cs = 0;
	goto _out;
st60:
	if ( ++p == pe )
		goto _test_eof60;
case 60:
	switch( (*p) ) {
		case 68: goto st3;
		case 72: goto st6;
		case 100: goto st3;
		case 104: goto st6;
	}
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr85;
	goto st0;
tr85:
#line 37 "src/readstat_spss_parse.rl"
	{ fmt->type = SPSS_FORMAT_TYPE_A; }
#line 21 "src/readstat_spss_parse.rl"
	{
            integer = 0;
        }
#line 25 "src/readstat_spss_parse.rl"
	{
            integer = 10 * integer + ((*p) - '0');
        }
	goto st61;
tr89:
#line 25 "src/readstat_spss_parse.rl"
	{
            integer = 10 * integer + ((*p) - '0');
        }
	goto st61;
tr91:
#line 55 "src/readstat_spss_parse.rl"
	{ fmt->type = SPSS_FORMAT_TYPE_ADATE; }
#line 21 "src/readstat_spss_parse.rl"
	{
            integer = 0;
        }
#line 25 "src/readstat_spss_parse.rl"
	{
            integer = 10 * integer + ((*p) - '0');
        }
	goto st61;
tr92:
#line 38 "src/readstat_spss_parse.rl"
	{ fmt->type = SPSS_FORMAT_TYPE_AHEX; }
#line 21 "src/readstat_spss_parse.rl"
	{
            integer = 0;
        }
#line 25 "src/readstat_spss_parse.rl"
	{
            integer = 10 * integer + ((*p) - '0');
        }
	goto st61;
tr93:
#line 65 "src/readstat_spss_parse.rl"
	{ fmt->type = SPSS_FORMAT_TYPE_CCA; }
#line 21 "src/readstat_spss_parse.rl"
	{
            integer = 0;
        }
#line 25 "src/readstat_spss_parse.rl"
	{
            integer = 10 * integer + ((*p) - '0');
        }
	goto st61;
tr94:
#line 66 "src/readstat_spss_parse.rl"
	{ fmt->type = SPSS_FORMAT_TYPE_CCB; }
#line 21 "src/readstat_spss_parse.rl"
	{
            integer = 0;
        }
#line 25 "src/readstat_spss_parse.rl"
	{
            integer = 10 * integer + ((*p) - '0');
        }
	goto st61;
tr95:
#line 67 "src/readstat_spss_parse.rl"
	{ fmt->type = SPSS_FORMAT_TYPE_CCC; }
#line 21 "src/readstat_spss_parse.rl"
	{
            integer = 0;
        }
#line 25 "src/readstat_spss_parse.rl"
	{
            integer = 10 * integer + ((*p) - '0');
        }
	goto st61;
tr96:
#line 68 "src/readstat_spss_parse.rl"
	{ fmt->type = SPSS_FORMAT_TYPE_CCD; }
#line 21 "src/readstat_spss_parse.rl"
	{
            integer = 0;
        }
#line 25 "src/readstat_spss_parse.rl"
	{
            integer = 10 * integer + ((*p) - '0');
        }
	goto st61;
tr97:
#line 69 "src/readstat_spss_parse.rl"
	{ fmt->type = SPSS_FORMAT_TYPE_CCE; }
#line 21 "src/readstat_spss_parse.rl"
	{
            integer = 0;
        }
#line 25 "src/readstat_spss_parse.rl"
	{
            integer = 10 * integer + ((*p) - '0');
        }
	goto st61;
tr98:
#line 39 "src/readstat_spss_parse.rl"
	{ fmt->type = SPSS_FORMAT_TYPE_COMMA; }
#line 21 "src/readstat_spss_parse.rl"
	{
            integer = 0;
        }
#line 25 "src/readstat_spss_parse.rl"
	{
            integer = 10 * integer + ((*p) - '0');
        }
	goto st61;
tr99:
#line 52 "src/readstat_spss_parse.rl"
	{ fmt->type = SPSS_FORMAT_TYPE_DATE; }
#line 21 "src/readstat_spss_parse.rl"
	{
            integer = 0;
        }
#line 25 "src/readstat_spss_parse.rl"
	{
            integer = 10 * integer + ((*p) - '0');
        }
	goto st61;
tr101:
#line 54 "src/readstat_spss_parse.rl"
	{ fmt->type = SPSS_FORMAT_TYPE_DATETIME; }
#line 21 "src/readstat_spss_parse.rl"
	{
            integer = 0;
        }
#line 25 "src/readstat_spss_parse.rl"
	{
            integer = 10 * integer + ((*p) - '0');
        }
	goto st61;
tr102:
#line 40 "src/readstat_spss_parse.rl"
	{ fmt->type = SPSS_FORMAT_TYPE_DOLLAR; }
#line 21 "src/readstat_spss_parse.rl"
	{
            integer = 0;
        }
#line 25 "src/readstat_spss_parse.rl"
	{
            integer = 10 * integer + ((*p) - '0');
        }
	goto st61;
tr103:
#line 64 "src/readstat_spss_parse.rl"
	{ fmt->type = SPSS_FORMAT_TYPE_DOT; }
#line 21 "src/readstat_spss_parse.rl"
	{
            integer = 0;
        }
#line 25 "src/readstat_spss_parse.rl"
	{
            integer = 10 * integer + ((*p) - '0');
        }
	goto st61;
tr104:
#line 57 "src/readstat_spss_parse.rl"
	{ fmt->type = SPSS_FORMAT_TYPE_DTIME; }
#line 21 "src/readstat_spss_parse.rl"
	{
            integer = 0;
        }
#line 25 "src/readstat_spss_parse.rl"
	{
            integer = 10 * integer + ((*p) - '0');
        }
	goto st61;
tr105:
#line 51 "src/readstat_spss_parse.rl"
	{ fmt->type = SPSS_FORMAT_TYPE_E; }
#line 21 "src/readstat_spss_parse.rl"
	{
            integer = 0;
        }
#line 25 "src/readstat_spss_parse.rl"
	{
            integer = 10 * integer + ((*p) - '0');
        }
	goto st61;
tr107:
#line 70 "src/readstat_spss_parse.rl"
	{ fmt->type = SPSS_FORMAT_TYPE_EDATE; }
#line 21 "src/readstat_spss_parse.rl"
	{
            integer = 0;
        }
#line 25 "src/readstat_spss_parse.rl"
	{
            integer = 10 * integer + ((*p) - '0');
        }
	goto st61;
tr108:
#line 41 "src/readstat_spss_parse.rl"
	{ fmt->type = SPSS_FORMAT_TYPE_F; }
#line 21 "src/readstat_spss_parse.rl"
	{
            integer = 0;
        }
#line 25 "src/readstat_spss_parse.rl"
	{
            integer = 10 * integer + ((*p) - '0');
        }
	goto st61;
tr109:
#line 42 "src/readstat_spss_parse.rl"
	{ fmt->type = SPSS_FORMAT_TYPE_IB; }
#line 21 "src/readstat_spss_parse.rl"
	{
            integer = 0;
        }
#line 25 "src/readstat_spss_parse.rl"
	{
            integer = 10 * integer + ((*p) - '0');
        }
	goto st61;
tr110:
#line 56 "src/readstat_spss_parse.rl"
	{ fmt->type = SPSS_FORMAT_TYPE_JDATE; }
#line 21 "src/readstat_spss_parse.rl"
	{
            integer = 0;
        }
#line 25 "src/readstat_spss_parse.rl"
	{
            integer = 10 * integer + ((*p) - '0');
        }
	goto st61;
tr111:
#line 59 "src/readstat_spss_parse.rl"
	{ fmt->type = SPSS_FORMAT_TYPE_MONTH; }
#line 21 "src/readstat_spss_parse.rl"
	{
            integer = 0;
        }
#line 25 "src/readstat_spss_parse.rl"
	{
            integer = 10 * integer + ((*p) - '0');
        }
	goto st61;
tr112:
#line 60 "src/readstat_spss_parse.rl"
	{ fmt->type = SPSS_FORMAT_TYPE_MOYR; }
#line 21 "src/readstat_spss_parse.rl"
	{
            integer = 0;
        }
#line 25 "src/readstat_spss_parse.rl"
	{
            integer = 10 * integer + ((*p) - '0');
        }
	goto st61;
tr113:
#line 50 "src/readstat_spss_parse.rl"
	{ fmt->type = SPSS_FORMAT_TYPE_N; }
#line 21 "src/readstat_spss_parse.rl"
	{
            integer = 0;
        }
#line 25 "src/readstat_spss_parse.rl"
	{
            integer = 10 * integer + ((*p) - '0');
        }
	goto st61;
tr114:
#line 44 "src/readstat_spss_parse.rl"
	{ fmt->type = SPSS_FORMAT_TYPE_P; }
#line 21 "src/readstat_spss_parse.rl"
	{
            integer = 0;
        }
#line 25 "src/readstat_spss_parse.rl"
	{
            integer = 10 * integer + ((*p) - '0');
        }
	goto st61;
tr118:
#line 63 "src/readstat_spss_parse.rl"
	{ fmt->type = SPSS_FORMAT_TYPE_PCT; }
#line 21 "src/readstat_spss_parse.rl"
	{
            integer = 0;
        }
#line 25 "src/readstat_spss_parse.rl"
	{
            integer = 10 * integer + ((*p) - '0');
        }
	goto st61;
tr119:
#line 45 "src/readstat_spss_parse.rl"
	{ fmt->type = SPSS_FORMAT_TYPE_PIB; }
#line 21 "src/readstat_spss_parse.rl"
	{
            integer = 0;
        }
#line 25 "src/readstat_spss_parse.rl"
	{
            integer = 10 * integer + ((*p) - '0');
        }
	goto st61;
tr121:
#line 43 "src/readstat_spss_parse.rl"
	{ fmt->type = SPSS_FORMAT_TYPE_PIBHEX; }
#line 21 "src/readstat_spss_parse.rl"
	{
            integer = 0;
        }
#line 25 "src/readstat_spss_parse.rl"
	{
            integer = 10 * integer + ((*p) - '0');
        }
	goto st61;
tr122:
#line 46 "src/readstat_spss_parse.rl"
	{ fmt->type = SPSS_FORMAT_TYPE_PK; }
#line 21 "src/readstat_spss_parse.rl"
	{
            integer = 0;
        }
#line 25 "src/readstat_spss_parse.rl"
	{
            integer = 10 * integer + ((*p) - '0');
        }
	goto st61;
tr123:
#line 61 "src/readstat_spss_parse.rl"
	{ fmt->type = SPSS_FORMAT_TYPE_QYR; }
#line 21 "src/readstat_spss_parse.rl"
	{
            integer = 0;
        }
#line 25 "src/readstat_spss_parse.rl"
	{
            integer = 10 * integer + ((*p) - '0');
        }
	goto st61;
tr124:
#line 47 "src/readstat_spss_parse.rl"
	{ fmt->type = SPSS_FORMAT_TYPE_RB; }
#line 21 "src/readstat_spss_parse.rl"
	{
            integer = 0;
        }
#line 25 "src/readstat_spss_parse.rl"
	{
            integer = 10 * integer + ((*p) - '0');
        }
	goto st61;
tr126:
#line 48 "src/readstat_spss_parse.rl"
	{ fmt->type = SPSS_FORMAT_TYPE_RBHEX; }
#line 21 "src/readstat_spss_parse.rl"
	{
            integer = 0;
        }
#line 25 "src/readstat_spss_parse.rl"
	{
            integer = 10 * integer + ((*p) - '0');
        }
	goto st61;
tr127:
#line 71 "src/readstat_spss_parse.rl"
	{ fmt->type = SPSS_FORMAT_TYPE_SDATE; }
#line 21 "src/readstat_spss_parse.rl"
	{
            integer = 0;
        }
#line 25 "src/readstat_spss_parse.rl"
	{
            integer = 10 * integer + ((*p) - '0');
        }
	goto st61;
tr128:
#line 53 "src/readstat_spss_parse.rl"
	{ fmt->type = SPSS_FORMAT_TYPE_TIME; }
#line 21 "src/readstat_spss_parse.rl"
	{
            integer = 0;
        }
#line 25 "src/readstat_spss_parse.rl"
	{
            integer = 10 * integer + ((*p) - '0');
        }
	goto st61;
tr129:
#line 58 "src/readstat_spss_parse.rl"
	{ fmt->type = SPSS_FORMAT_TYPE_WKDAY; }
#line 21 "src/readstat_spss_parse.rl"
	{
            integer = 0;
        }
#line 25 "src/readstat_spss_parse.rl"
	{
            integer = 10 * integer + ((*p) - '0');
        }
	goto st61;
tr130:
#line 62 "src/readstat_spss_parse.rl"
	{ fmt->type = SPSS_FORMAT_TYPE_WKYR; }
#line 21 "src/readstat_spss_parse.rl"
	{
            integer = 0;
        }
#line 25 "src/readstat_spss_parse.rl"
	{
            integer = 10 * integer + ((*p) - '0');
        }
	goto st61;
tr131:
#line 49 "src/readstat_spss_parse.rl"
	{ fmt->type = SPSS_FORMAT_TYPE_Z; }
#line 21 "src/readstat_spss_parse.rl"
	{
            integer = 0;
        }
#line 25 "src/readstat_spss_parse.rl"
	{
            integer = 10 * integer + ((*p) - '0');
        }
	goto st61;
st61:
	if ( ++p == pe )
		goto _test_eof61;
case 61:
#line 522 "src/readstat_spss_parse.c"
	if ( (*p) == 46 )
		goto tr88;
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr89;
	goto st0;
tr88:
#line 29 "src/readstat_spss_parse.rl"
	{
            fmt->width = integer;
        }
	goto st2;
st2:
	if ( ++p == pe )
		goto _test_eof2;
case 2:
#line 538 "src/readstat_spss_parse.c"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr17;
	goto st0;
tr17:
#line 21 "src/readstat_spss_parse.rl"
	{
            integer = 0;
        }
#line 25 "src/readstat_spss_parse.rl"
	{
            integer = 10 * integer + ((*p) - '0');
        }
	goto st62;
tr90:
#line 25 "src/readstat_spss_parse.rl"
	{
            integer = 10 * integer + ((*p) - '0');
        }
	goto st62;
st62:
	if ( ++p == pe )
		goto _test_eof62;
case 62:
#line 562 "src/readstat_spss_parse.c"
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr90;
	goto st0;
st3:
	if ( ++p == pe )
		goto _test_eof3;
case 3:
	switch( (*p) ) {
		case 65: goto st4;
		case 97: goto st4;
	}
	goto st0;
st4:
	if ( ++p == pe )
		goto _test_eof4;
case 4:
	switch( (*p) ) {
		case 84: goto st5;
		case 116: goto st5;
	}
	goto st0;
st5:
	if ( ++p == pe )
		goto _test_eof5;
case 5:
	switch( (*p) ) {
		case 69: goto st63;
		case 101: goto st63;
	}
	goto st0;
st63:
	if ( ++p == pe )
		goto _test_eof63;
case 63:
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr91;
	goto st0;
st6:
	if ( ++p == pe )
		goto _test_eof6;
case 6:
	switch( (*p) ) {
		case 69: goto st7;
		case 101: goto st7;
	}
	goto st0;
st7:
	if ( ++p == pe )
		goto _test_eof7;
case 7:
	switch( (*p) ) {
		case 88: goto st64;
		case 120: goto st64;
	}
	goto st0;
st64:
	if ( ++p == pe )
		goto _test_eof64;
case 64:
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr92;
	goto st0;
st8:
	if ( ++p == pe )
		goto _test_eof8;
case 8:
	switch( (*p) ) {
		case 67: goto st9;
		case 79: goto st10;
		case 99: goto st9;
		case 111: goto st10;
	}
	goto st0;
st9:
	if ( ++p == pe )
		goto _test_eof9;
case 9:
	switch( (*p) ) {
		case 65: goto st65;
		case 66: goto st66;
		case 67: goto st67;
		case 68: goto st68;
		case 69: goto st69;
		case 97: goto st65;
		case 98: goto st66;
		case 99: goto st67;
		case 100: goto st68;
		case 101: goto st69;
	}
	goto st0;
st65:
	if ( ++p == pe )
		goto _test_eof65;
case 65:
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr93;
	goto st0;
st66:
	if ( ++p == pe )
		goto _test_eof66;
case 66:
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr94;
	goto st0;
st67:
	if ( ++p == pe )
		goto _test_eof67;
case 67:
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr95;
	goto st0;
st68:
	if ( ++p == pe )
		goto _test_eof68;
case 68:
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr96;
	goto st0;
st69:
	if ( ++p == pe )
		goto _test_eof69;
case 69:
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr97;
	goto st0;
st10:
	if ( ++p == pe )
		goto _test_eof10;
case 10:
	switch( (*p) ) {
		case 77: goto st11;
		case 109: goto st11;
	}
	goto st0;
st11:
	if ( ++p == pe )
		goto _test_eof11;
case 11:
	switch( (*p) ) {
		case 77: goto st12;
		case 109: goto st12;
	}
	goto st0;
st12:
	if ( ++p == pe )
		goto _test_eof12;
case 12:
	switch( (*p) ) {
		case 65: goto st70;
		case 97: goto st70;
	}
	goto st0;
st70:
	if ( ++p == pe )
		goto _test_eof70;
case 70:
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr98;
	goto st0;
st13:
	if ( ++p == pe )
		goto _test_eof13;
case 13:
	switch( (*p) ) {
		case 65: goto st14;
		case 79: goto st19;
		case 84: goto st23;
		case 97: goto st14;
		case 111: goto st19;
		case 116: goto st23;
	}
	goto st0;
st14:
	if ( ++p == pe )
		goto _test_eof14;
case 14:
	switch( (*p) ) {
		case 84: goto st15;
		case 116: goto st15;
	}
	goto st0;
st15:
	if ( ++p == pe )
		goto _test_eof15;
case 15:
	switch( (*p) ) {
		case 69: goto st71;
		case 101: goto st71;
	}
	goto st0;
st71:
	if ( ++p == pe )
		goto _test_eof71;
case 71:
	switch( (*p) ) {
		case 84: goto st16;
		case 116: goto st16;
	}
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr99;
	goto st0;
st16:
	if ( ++p == pe )
		goto _test_eof16;
case 16:
	switch( (*p) ) {
		case 73: goto st17;
		case 105: goto st17;
	}
	goto st0;
st17:
	if ( ++p == pe )
		goto _test_eof17;
case 17:
	switch( (*p) ) {
		case 77: goto st18;
		case 109: goto st18;
	}
	goto st0;
st18:
	if ( ++p == pe )
		goto _test_eof18;
case 18:
	switch( (*p) ) {
		case 69: goto st72;
		case 101: goto st72;
	}
	goto st0;
st72:
	if ( ++p == pe )
		goto _test_eof72;
case 72:
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr101;
	goto st0;
st19:
	if ( ++p == pe )
		goto _test_eof19;
case 19:
	switch( (*p) ) {
		case 76: goto st20;
		case 84: goto st74;
		case 108: goto st20;
		case 116: goto st74;
	}
	goto st0;
st20:
	if ( ++p == pe )
		goto _test_eof20;
case 20:
	switch( (*p) ) {
		case 76: goto st21;
		case 108: goto st21;
	}
	goto st0;
st21:
	if ( ++p == pe )
		goto _test_eof21;
case 21:
	switch( (*p) ) {
		case 65: goto st22;
		case 97: goto st22;
	}
	goto st0;
st22:
	if ( ++p == pe )
		goto _test_eof22;
case 22:
	switch( (*p) ) {
		case 82: goto st73;
		case 114: goto st73;
	}
	goto st0;
st73:
	if ( ++p == pe )
		goto _test_eof73;
case 73:
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr102;
	goto st0;
st74:
	if ( ++p == pe )
		goto _test_eof74;
case 74:
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr103;
	goto st0;
st23:
	if ( ++p == pe )
		goto _test_eof23;
case 23:
	switch( (*p) ) {
		case 73: goto st24;
		case 105: goto st24;
	}
	goto st0;
st24:
	if ( ++p == pe )
		goto _test_eof24;
case 24:
	switch( (*p) ) {
		case 77: goto st25;
		case 109: goto st25;
	}
	goto st0;
st25:
	if ( ++p == pe )
		goto _test_eof25;
case 25:
	switch( (*p) ) {
		case 69: goto st75;
		case 101: goto st75;
	}
	goto st0;
st75:
	if ( ++p == pe )
		goto _test_eof75;
case 75:
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr104;
	goto st0;
st76:
	if ( ++p == pe )
		goto _test_eof76;
case 76:
	switch( (*p) ) {
		case 68: goto st26;
		case 100: goto st26;
	}
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr105;
	goto st0;
st26:
	if ( ++p == pe )
		goto _test_eof26;
case 26:
	switch( (*p) ) {
		case 65: goto st27;
		case 97: goto st27;
	}
	goto st0;
st27:
	if ( ++p == pe )
		goto _test_eof27;
case 27:
	switch( (*p) ) {
		case 84: goto st28;
		case 116: goto st28;
	}
	goto st0;
st28:
	if ( ++p == pe )
		goto _test_eof28;
case 28:
	switch( (*p) ) {
		case 69: goto st77;
		case 101: goto st77;
	}
	goto st0;
st77:
	if ( ++p == pe )
		goto _test_eof77;
case 77:
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr107;
	goto st0;
st78:
	if ( ++p == pe )
		goto _test_eof78;
case 78:
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr108;
	goto st0;
st29:
	if ( ++p == pe )
		goto _test_eof29;
case 29:
	switch( (*p) ) {
		case 66: goto st79;
		case 98: goto st79;
	}
	goto st0;
st79:
	if ( ++p == pe )
		goto _test_eof79;
case 79:
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr109;
	goto st0;
st30:
	if ( ++p == pe )
		goto _test_eof30;
case 30:
	switch( (*p) ) {
		case 68: goto st31;
		case 100: goto st31;
	}
	goto st0;
st31:
	if ( ++p == pe )
		goto _test_eof31;
case 31:
	switch( (*p) ) {
		case 65: goto st32;
		case 97: goto st32;
	}
	goto st0;
st32:
	if ( ++p == pe )
		goto _test_eof32;
case 32:
	switch( (*p) ) {
		case 84: goto st33;
		case 116: goto st33;
	}
	goto st0;
st33:
	if ( ++p == pe )
		goto _test_eof33;
case 33:
	switch( (*p) ) {
		case 69: goto st80;
		case 101: goto st80;
	}
	goto st0;
st80:
	if ( ++p == pe )
		goto _test_eof80;
case 80:
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr110;
	goto st0;
st34:
	if ( ++p == pe )
		goto _test_eof34;
case 34:
	switch( (*p) ) {
		case 79: goto st35;
		case 111: goto st35;
	}
	goto st0;
st35:
	if ( ++p == pe )
		goto _test_eof35;
case 35:
	switch( (*p) ) {
		case 78: goto st36;
		case 89: goto st38;
		case 110: goto st36;
		case 121: goto st38;
	}
	goto st0;
st36:
	if ( ++p == pe )
		goto _test_eof36;
case 36:
	switch( (*p) ) {
		case 84: goto st37;
		case 116: goto st37;
	}
	goto st0;
st37:
	if ( ++p == pe )
		goto _test_eof37;
case 37:
	switch( (*p) ) {
		case 72: goto st81;
		case 104: goto st81;
	}
	goto st0;
st81:
	if ( ++p == pe )
		goto _test_eof81;
case 81:
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr111;
	goto st0;
st38:
	if ( ++p == pe )
		goto _test_eof38;
case 38:
	switch( (*p) ) {
		case 82: goto st82;
		case 114: goto st82;
	}
	goto st0;
st82:
	if ( ++p == pe )
		goto _test_eof82;
case 82:
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr112;
	goto st0;
st83:
	if ( ++p == pe )
		goto _test_eof83;
case 83:
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr113;
	goto st0;
st84:
	if ( ++p == pe )
		goto _test_eof84;
case 84:
	switch( (*p) ) {
		case 67: goto st39;
		case 73: goto st40;
		case 75: goto st88;
		case 99: goto st39;
		case 105: goto st40;
		case 107: goto st88;
	}
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr114;
	goto st0;
st39:
	if ( ++p == pe )
		goto _test_eof39;
case 39:
	switch( (*p) ) {
		case 84: goto st85;
		case 116: goto st85;
	}
	goto st0;
st85:
	if ( ++p == pe )
		goto _test_eof85;
case 85:
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr118;
	goto st0;
st40:
	if ( ++p == pe )
		goto _test_eof40;
case 40:
	switch( (*p) ) {
		case 66: goto st86;
		case 98: goto st86;
	}
	goto st0;
st86:
	if ( ++p == pe )
		goto _test_eof86;
case 86:
	switch( (*p) ) {
		case 72: goto st41;
		case 104: goto st41;
	}
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr119;
	goto st0;
st41:
	if ( ++p == pe )
		goto _test_eof41;
case 41:
	switch( (*p) ) {
		case 69: goto st42;
		case 101: goto st42;
	}
	goto st0;
st42:
	if ( ++p == pe )
		goto _test_eof42;
case 42:
	switch( (*p) ) {
		case 88: goto st87;
		case 120: goto st87;
	}
	goto st0;
st87:
	if ( ++p == pe )
		goto _test_eof87;
case 87:
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr121;
	goto st0;
st88:
	if ( ++p == pe )
		goto _test_eof88;
case 88:
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr122;
	goto st0;
st43:
	if ( ++p == pe )
		goto _test_eof43;
case 43:
	switch( (*p) ) {
		case 89: goto st44;
		case 121: goto st44;
	}
	goto st0;
st44:
	if ( ++p == pe )
		goto _test_eof44;
case 44:
	switch( (*p) ) {
		case 82: goto st89;
		case 114: goto st89;
	}
	goto st0;
st89:
	if ( ++p == pe )
		goto _test_eof89;
case 89:
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr123;
	goto st0;
st45:
	if ( ++p == pe )
		goto _test_eof45;
case 45:
	switch( (*p) ) {
		case 66: goto st90;
		case 98: goto st90;
	}
	goto st0;
st90:
	if ( ++p == pe )
		goto _test_eof90;
case 90:
	switch( (*p) ) {
		case 72: goto st46;
		case 104: goto st46;
	}
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr124;
	goto st0;
st46:
	if ( ++p == pe )
		goto _test_eof46;
case 46:
	switch( (*p) ) {
		case 69: goto st47;
		case 101: goto st47;
	}
	goto st0;
st47:
	if ( ++p == pe )
		goto _test_eof47;
case 47:
	switch( (*p) ) {
		case 88: goto st91;
		case 120: goto st91;
	}
	goto st0;
st91:
	if ( ++p == pe )
		goto _test_eof91;
case 91:
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr126;
	goto st0;
st48:
	if ( ++p == pe )
		goto _test_eof48;
case 48:
	switch( (*p) ) {
		case 68: goto st49;
		case 100: goto st49;
	}
	goto st0;
st49:
	if ( ++p == pe )
		goto _test_eof49;
case 49:
	switch( (*p) ) {
		case 65: goto st50;
		case 97: goto st50;
	}
	goto st0;
st50:
	if ( ++p == pe )
		goto _test_eof50;
case 50:
	switch( (*p) ) {
		case 84: goto st51;
		case 116: goto st51;
	}
	goto st0;
st51:
	if ( ++p == pe )
		goto _test_eof51;
case 51:
	switch( (*p) ) {
		case 69: goto st92;
		case 101: goto st92;
	}
	goto st0;
st92:
	if ( ++p == pe )
		goto _test_eof92;
case 92:
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr127;
	goto st0;
st52:
	if ( ++p == pe )
		goto _test_eof52;
case 52:
	switch( (*p) ) {
		case 73: goto st53;
		case 105: goto st53;
	}
	goto st0;
st53:
	if ( ++p == pe )
		goto _test_eof53;
case 53:
	switch( (*p) ) {
		case 77: goto st54;
		case 109: goto st54;
	}
	goto st0;
st54:
	if ( ++p == pe )
		goto _test_eof54;
case 54:
	switch( (*p) ) {
		case 69: goto st93;
		case 101: goto st93;
	}
	goto st0;
st93:
	if ( ++p == pe )
		goto _test_eof93;
case 93:
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr128;
	goto st0;
st55:
	if ( ++p == pe )
		goto _test_eof55;
case 55:
	switch( (*p) ) {
		case 75: goto st56;
		case 107: goto st56;
	}
	goto st0;
st56:
	if ( ++p == pe )
		goto _test_eof56;
case 56:
	switch( (*p) ) {
		case 68: goto st57;
		case 89: goto st59;
		case 100: goto st57;
		case 121: goto st59;
	}
	goto st0;
st57:
	if ( ++p == pe )
		goto _test_eof57;
case 57:
	switch( (*p) ) {
		case 65: goto st58;
		case 97: goto st58;
	}
	goto st0;
st58:
	if ( ++p == pe )
		goto _test_eof58;
case 58:
	switch( (*p) ) {
		case 89: goto st94;
		case 121: goto st94;
	}
	goto st0;
st94:
	if ( ++p == pe )
		goto _test_eof94;
case 94:
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr129;
	goto st0;
st59:
	if ( ++p == pe )
		goto _test_eof59;
case 59:
	switch( (*p) ) {
		case 82: goto st95;
		case 114: goto st95;
	}
	goto st0;
st95:
	if ( ++p == pe )
		goto _test_eof95;
case 95:
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr130;
	goto st0;
st96:
	if ( ++p == pe )
		goto _test_eof96;
case 96:
	if ( 48 <= (*p) && (*p) <= 57 )
		goto tr131;
	goto st0;
	}
	_test_eof60: cs = 60; goto _test_eof; 
	_test_eof61: cs = 61; goto _test_eof; 
	_test_eof2: cs = 2; goto _test_eof; 
	_test_eof62: cs = 62; goto _test_eof; 
	_test_eof3: cs = 3; goto _test_eof; 
	_test_eof4: cs = 4; goto _test_eof; 
	_test_eof5: cs = 5; goto _test_eof; 
	_test_eof63: cs = 63; goto _test_eof; 
	_test_eof6: cs = 6; goto _test_eof; 
	_test_eof7: cs = 7; goto _test_eof; 
	_test_eof64: cs = 64; goto _test_eof; 
	_test_eof8: cs = 8; goto _test_eof; 
	_test_eof9: cs = 9; goto _test_eof; 
	_test_eof65: cs = 65; goto _test_eof; 
	_test_eof66: cs = 66; goto _test_eof; 
	_test_eof67: cs = 67; goto _test_eof; 
	_test_eof68: cs = 68; goto _test_eof; 
	_test_eof69: cs = 69; goto _test_eof; 
	_test_eof10: cs = 10; goto _test_eof; 
	_test_eof11: cs = 11; goto _test_eof; 
	_test_eof12: cs = 12; goto _test_eof; 
	_test_eof70: cs = 70; goto _test_eof; 
	_test_eof13: cs = 13; goto _test_eof; 
	_test_eof14: cs = 14; goto _test_eof; 
	_test_eof15: cs = 15; goto _test_eof; 
	_test_eof71: cs = 71; goto _test_eof; 
	_test_eof16: cs = 16; goto _test_eof; 
	_test_eof17: cs = 17; goto _test_eof; 
	_test_eof18: cs = 18; goto _test_eof; 
	_test_eof72: cs = 72; goto _test_eof; 
	_test_eof19: cs = 19; goto _test_eof; 
	_test_eof20: cs = 20; goto _test_eof; 
	_test_eof21: cs = 21; goto _test_eof; 
	_test_eof22: cs = 22; goto _test_eof; 
	_test_eof73: cs = 73; goto _test_eof; 
	_test_eof74: cs = 74; goto _test_eof; 
	_test_eof23: cs = 23; goto _test_eof; 
	_test_eof24: cs = 24; goto _test_eof; 
	_test_eof25: cs = 25; goto _test_eof; 
	_test_eof75: cs = 75; goto _test_eof; 
	_test_eof76: cs = 76; goto _test_eof; 
	_test_eof26: cs = 26; goto _test_eof; 
	_test_eof27: cs = 27; goto _test_eof; 
	_test_eof28: cs = 28; goto _test_eof; 
	_test_eof77: cs = 77; goto _test_eof; 
	_test_eof78: cs = 78; goto _test_eof; 
	_test_eof29: cs = 29; goto _test_eof; 
	_test_eof79: cs = 79; goto _test_eof; 
	_test_eof30: cs = 30; goto _test_eof; 
	_test_eof31: cs = 31; goto _test_eof; 
	_test_eof32: cs = 32; goto _test_eof; 
	_test_eof33: cs = 33; goto _test_eof; 
	_test_eof80: cs = 80; goto _test_eof; 
	_test_eof34: cs = 34; goto _test_eof; 
	_test_eof35: cs = 35; goto _test_eof; 
	_test_eof36: cs = 36; goto _test_eof; 
	_test_eof37: cs = 37; goto _test_eof; 
	_test_eof81: cs = 81; goto _test_eof; 
	_test_eof38: cs = 38; goto _test_eof; 
	_test_eof82: cs = 82; goto _test_eof; 
	_test_eof83: cs = 83; goto _test_eof; 
	_test_eof84: cs = 84; goto _test_eof; 
	_test_eof39: cs = 39; goto _test_eof; 
	_test_eof85: cs = 85; goto _test_eof; 
	_test_eof40: cs = 40; goto _test_eof; 
	_test_eof86: cs = 86; goto _test_eof; 
	_test_eof41: cs = 41; goto _test_eof; 
	_test_eof42: cs = 42; goto _test_eof; 
	_test_eof87: cs = 87; goto _test_eof; 
	_test_eof88: cs = 88; goto _test_eof; 
	_test_eof43: cs = 43; goto _test_eof; 
	_test_eof44: cs = 44; goto _test_eof; 
	_test_eof89: cs = 89; goto _test_eof; 
	_test_eof45: cs = 45; goto _test_eof; 
	_test_eof90: cs = 90; goto _test_eof; 
	_test_eof46: cs = 46; goto _test_eof; 
	_test_eof47: cs = 47; goto _test_eof; 
	_test_eof91: cs = 91; goto _test_eof; 
	_test_eof48: cs = 48; goto _test_eof; 
	_test_eof49: cs = 49; goto _test_eof; 
	_test_eof50: cs = 50; goto _test_eof; 
	_test_eof51: cs = 51; goto _test_eof; 
	_test_eof92: cs = 92; goto _test_eof; 
	_test_eof52: cs = 52; goto _test_eof; 
	_test_eof53: cs = 53; goto _test_eof; 
	_test_eof54: cs = 54; goto _test_eof; 
	_test_eof93: cs = 93; goto _test_eof; 
	_test_eof55: cs = 55; goto _test_eof; 
	_test_eof56: cs = 56; goto _test_eof; 
	_test_eof57: cs = 57; goto _test_eof; 
	_test_eof58: cs = 58; goto _test_eof; 
	_test_eof94: cs = 94; goto _test_eof; 
	_test_eof59: cs = 59; goto _test_eof; 
	_test_eof95: cs = 95; goto _test_eof; 
	_test_eof96: cs = 96; goto _test_eof; 

	_test_eof: {}
	if ( p == eof )
	{
	switch ( cs ) {
	case 61: 
#line 29 "src/readstat_spss_parse.rl"
	{
            fmt->width = integer;
        }
	break;
	case 62: 
#line 33 "src/readstat_spss_parse.rl"
	{
            fmt->decimal_places = integer;
        }
	break;
	case 60: 
#line 37 "src/readstat_spss_parse.rl"
	{ fmt->type = SPSS_FORMAT_TYPE_A; }
	break;
	case 64: 
#line 38 "src/readstat_spss_parse.rl"
	{ fmt->type = SPSS_FORMAT_TYPE_AHEX; }
	break;
	case 70: 
#line 39 "src/readstat_spss_parse.rl"
	{ fmt->type = SPSS_FORMAT_TYPE_COMMA; }
	break;
	case 73: 
#line 40 "src/readstat_spss_parse.rl"
	{ fmt->type = SPSS_FORMAT_TYPE_DOLLAR; }
	break;
	case 78: 
#line 41 "src/readstat_spss_parse.rl"
	{ fmt->type = SPSS_FORMAT_TYPE_F; }
	break;
	case 79: 
#line 42 "src/readstat_spss_parse.rl"
	{ fmt->type = SPSS_FORMAT_TYPE_IB; }
	break;
	case 87: 
#line 43 "src/readstat_spss_parse.rl"
	{ fmt->type = SPSS_FORMAT_TYPE_PIBHEX; }
	break;
	case 84: 
#line 44 "src/readstat_spss_parse.rl"
	{ fmt->type = SPSS_FORMAT_TYPE_P; }
	break;
	case 86: 
#line 45 "src/readstat_spss_parse.rl"
	{ fmt->type = SPSS_FORMAT_TYPE_PIB; }
	break;
	case 88: 
#line 46 "src/readstat_spss_parse.rl"
	{ fmt->type = SPSS_FORMAT_TYPE_PK; }
	break;
	case 90: 
#line 47 "src/readstat_spss_parse.rl"
	{ fmt->type = SPSS_FORMAT_TYPE_RB; }
	break;
	case 91: 
#line 48 "src/readstat_spss_parse.rl"
	{ fmt->type = SPSS_FORMAT_TYPE_RBHEX; }
	break;
	case 96: 
#line 49 "src/readstat_spss_parse.rl"
	{ fmt->type = SPSS_FORMAT_TYPE_Z; }
	break;
	case 83: 
#line 50 "src/readstat_spss_parse.rl"
	{ fmt->type = SPSS_FORMAT_TYPE_N; }
	break;
	case 76: 
#line 51 "src/readstat_spss_parse.rl"
	{ fmt->type = SPSS_FORMAT_TYPE_E; }
	break;
	case 71: 
#line 52 "src/readstat_spss_parse.rl"
	{ fmt->type = SPSS_FORMAT_TYPE_DATE; }
	break;
	case 93: 
#line 53 "src/readstat_spss_parse.rl"
	{ fmt->type = SPSS_FORMAT_TYPE_TIME; }
	break;
	case 72: 
#line 54 "src/readstat_spss_parse.rl"
	{ fmt->type = SPSS_FORMAT_TYPE_DATETIME; }
	break;
	case 63: 
#line 55 "src/readstat_spss_parse.rl"
	{ fmt->type = SPSS_FORMAT_TYPE_ADATE; }
	break;
	case 80: 
#line 56 "src/readstat_spss_parse.rl"
	{ fmt->type = SPSS_FORMAT_TYPE_JDATE; }
	break;
	case 75: 
#line 57 "src/readstat_spss_parse.rl"
	{ fmt->type = SPSS_FORMAT_TYPE_DTIME; }
	break;
	case 94: 
#line 58 "src/readstat_spss_parse.rl"
	{ fmt->type = SPSS_FORMAT_TYPE_WKDAY; }
	break;
	case 81: 
#line 59 "src/readstat_spss_parse.rl"
	{ fmt->type = SPSS_FORMAT_TYPE_MONTH; }
	break;
	case 82: 
#line 60 "src/readstat_spss_parse.rl"
	{ fmt->type = SPSS_FORMAT_TYPE_MOYR; }
	break;
	case 89: 
#line 61 "src/readstat_spss_parse.rl"
	{ fmt->type = SPSS_FORMAT_TYPE_QYR; }
	break;
	case 95: 
#line 62 "src/readstat_spss_parse.rl"
	{ fmt->type = SPSS_FORMAT_TYPE_WKYR; }
	break;
	case 85: 
#line 63 "src/readstat_spss_parse.rl"
	{ fmt->type = SPSS_FORMAT_TYPE_PCT; }
	break;
	case 74: 
#line 64 "src/readstat_spss_parse.rl"
	{ fmt->type = SPSS_FORMAT_TYPE_DOT; }
	break;
	case 65: 
#line 65 "src/readstat_spss_parse.rl"
	{ fmt->type = SPSS_FORMAT_TYPE_CCA; }
	break;
	case 66: 
#line 66 "src/readstat_spss_parse.rl"
	{ fmt->type = SPSS_FORMAT_TYPE_CCB; }
	break;
	case 67: 
#line 67 "src/readstat_spss_parse.rl"
	{ fmt->type = SPSS_FORMAT_TYPE_CCC; }
	break;
	case 68: 
#line 68 "src/readstat_spss_parse.rl"
	{ fmt->type = SPSS_FORMAT_TYPE_CCD; }
	break;
	case 69: 
#line 69 "src/readstat_spss_parse.rl"
	{ fmt->type = SPSS_FORMAT_TYPE_CCE; }
	break;
	case 77: 
#line 70 "src/readstat_spss_parse.rl"
	{ fmt->type = SPSS_FORMAT_TYPE_EDATE; }
	break;
	case 92: 
#line 71 "src/readstat_spss_parse.rl"
	{ fmt->type = SPSS_FORMAT_TYPE_SDATE; }
	break;
#line 1614 "src/readstat_spss_parse.c"
	}
	}

	_out: {}
	}

#line 84 "src/readstat_spss_parse.rl"


    /* suppress warning */
    (void)spss_format_parser_en_main;

    if (cs < 60 || p != eof) {
        return READSTAT_ERROR_PARSE;
    }

    return READSTAT_OK;
}
