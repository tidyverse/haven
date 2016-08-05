
#line 1 "src/readstat_por_parse.rl"

#include "readstat.h"
#include "readstat_por_parse.h"


#line 9 "src/readstat_por_parse.c"
static const int por_field_parse_start = 1;

static const int por_field_parse_en_main = 1;


#line 8 "src/readstat_por_parse.rl"


int readstat_por_parse_double(const char *data, size_t len, double *result, 
        readstat_error_handler error_cb, void *user_ctx) {
    int retval = 0;
    double val = 0.0;
    long num = 0;
    long frac = 0;
    long exp = 0;
    
    long temp_val = 0;
    long frac_len = 0;
    
    const unsigned char *val_start = NULL;
    
    const unsigned char *p = (const unsigned char *)data;
    const unsigned char *eof = p + len;
    
    int cs;
    int is_negative = 0, exp_is_negative = 0;
    int success = 0;
    
    
#line 39 "src/readstat_por_parse.c"
	{
	cs = por_field_parse_start;
	}

#line 44 "src/readstat_por_parse.c"
	{
	switch ( cs )
	{
st1:
	p += 1;
case 1:
	switch( (*p) ) {
		case 32: goto st1;
		case 42: goto st2;
		case 45: goto st3;
		case 46: goto st4;
	}
	if ( (*p) > 57 ) {
		if ( 65 <= (*p) && (*p) <= 84 )
			goto tr5;
	} else if ( (*p) >= 48 )
		goto tr5;
	goto st0;
st0:
cs = 0;
	goto _out;
st2:
	p += 1;
case 2:
	if ( (*p) == 46 )
		goto tr6;
	goto st0;
tr6:
#line 50 "src/readstat_por_parse.rl"
	{ success = 1; {p++; cs = 12; goto _out;} }
	goto st12;
st12:
	p += 1;
case 12:
#line 79 "src/readstat_por_parse.c"
	goto st0;
st3:
	p += 1;
case 3:
	if ( (*p) == 46 )
		goto tr7;
	if ( (*p) > 57 ) {
		if ( 65 <= (*p) && (*p) <= 84 )
			goto tr8;
	} else if ( (*p) >= 48 )
		goto tr8;
	goto st0;
tr7:
#line 46 "src/readstat_por_parse.rl"
	{ is_negative = 1; }
	goto st4;
st4:
	p += 1;
case 4:
#line 99 "src/readstat_por_parse.c"
	if ( (*p) > 57 ) {
		if ( 65 <= (*p) && (*p) <= 84 )
			goto tr9;
	} else if ( (*p) >= 48 )
		goto tr9;
	goto st0;
tr9:
#line 39 "src/readstat_por_parse.rl"
	{ temp_val = 0; val_start = p; }
#line 31 "src/readstat_por_parse.rl"
	{
            if ((*p) >= '0' && (*p) <= '9') {
                temp_val = 30 * temp_val + ((*p) - '0');
            } else if ((*p) >= 'A' && (*p) <= 'T') {
                temp_val = 30 * temp_val + (10 + (*p) - 'A');
            }
        }
	goto st5;
tr11:
#line 31 "src/readstat_por_parse.rl"
	{
            if ((*p) >= '0' && (*p) <= '9') {
                temp_val = 30 * temp_val + ((*p) - '0');
            } else if ((*p) >= 'A' && (*p) <= 'T') {
                temp_val = 30 * temp_val + (10 + (*p) - 'A');
            }
        }
	goto st5;
st5:
	p += 1;
case 5:
#line 131 "src/readstat_por_parse.c"
	if ( (*p) == 47 )
		goto tr10;
	if ( (*p) > 57 ) {
		if ( 65 <= (*p) && (*p) <= 84 )
			goto tr11;
	} else if ( (*p) >= 48 )
		goto tr11;
	goto st0;
tr10:
#line 41 "src/readstat_por_parse.rl"
	{ frac = temp_val; frac_len = (p - val_start); }
#line 50 "src/readstat_por_parse.rl"
	{ success = 1; {p++; cs = 13; goto _out;} }
	goto st13;
tr15:
#line 43 "src/readstat_por_parse.rl"
	{ num = temp_val; }
#line 50 "src/readstat_por_parse.rl"
	{ success = 1; {p++; cs = 13; goto _out;} }
	goto st13;
tr18:
#line 44 "src/readstat_por_parse.rl"
	{ exp = temp_val; }
#line 50 "src/readstat_por_parse.rl"
	{ success = 1; {p++; cs = 13; goto _out;} }
	goto st13;
st13:
	p += 1;
case 13:
#line 161 "src/readstat_por_parse.c"
	goto st0;
tr5:
#line 39 "src/readstat_por_parse.rl"
	{ temp_val = 0; val_start = p; }
#line 31 "src/readstat_por_parse.rl"
	{
            if ((*p) >= '0' && (*p) <= '9') {
                temp_val = 30 * temp_val + ((*p) - '0');
            } else if ((*p) >= 'A' && (*p) <= 'T') {
                temp_val = 30 * temp_val + (10 + (*p) - 'A');
            }
        }
	goto st6;
tr8:
#line 43 "src/readstat_por_parse.rl"
	{ is_negative = 1; }
#line 39 "src/readstat_por_parse.rl"
	{ temp_val = 0; val_start = p; }
#line 31 "src/readstat_por_parse.rl"
	{
            if ((*p) >= '0' && (*p) <= '9') {
                temp_val = 30 * temp_val + ((*p) - '0');
            } else if ((*p) >= 'A' && (*p) <= 'T') {
                temp_val = 30 * temp_val + (10 + (*p) - 'A');
            }
        }
	goto st6;
tr16:
#line 31 "src/readstat_por_parse.rl"
	{
            if ((*p) >= '0' && (*p) <= '9') {
                temp_val = 30 * temp_val + ((*p) - '0');
            } else if ((*p) >= 'A' && (*p) <= 'T') {
                temp_val = 30 * temp_val + (10 + (*p) - 'A');
            }
        }
	goto st6;
st6:
	p += 1;
case 6:
#line 202 "src/readstat_por_parse.c"
	switch( (*p) ) {
		case 43: goto tr12;
		case 45: goto tr13;
		case 46: goto tr14;
		case 47: goto tr15;
	}
	if ( (*p) > 57 ) {
		if ( 65 <= (*p) && (*p) <= 84 )
			goto tr16;
	} else if ( (*p) >= 48 )
		goto tr16;
	goto st0;
tr12:
#line 43 "src/readstat_por_parse.rl"
	{ num = temp_val; }
	goto st7;
tr22:
#line 41 "src/readstat_por_parse.rl"
	{ frac = temp_val; frac_len = (p - val_start); }
	goto st7;
st7:
	p += 1;
case 7:
#line 226 "src/readstat_por_parse.c"
	if ( (*p) > 57 ) {
		if ( 65 <= (*p) && (*p) <= 84 )
			goto tr17;
	} else if ( (*p) >= 48 )
		goto tr17;
	goto st0;
tr17:
#line 39 "src/readstat_por_parse.rl"
	{ temp_val = 0; val_start = p; }
#line 31 "src/readstat_por_parse.rl"
	{
            if ((*p) >= '0' && (*p) <= '9') {
                temp_val = 30 * temp_val + ((*p) - '0');
            } else if ((*p) >= 'A' && (*p) <= 'T') {
                temp_val = 30 * temp_val + (10 + (*p) - 'A');
            }
        }
	goto st8;
tr19:
#line 31 "src/readstat_por_parse.rl"
	{
            if ((*p) >= '0' && (*p) <= '9') {
                temp_val = 30 * temp_val + ((*p) - '0');
            } else if ((*p) >= 'A' && (*p) <= 'T') {
                temp_val = 30 * temp_val + (10 + (*p) - 'A');
            }
        }
	goto st8;
tr20:
#line 44 "src/readstat_por_parse.rl"
	{ exp_is_negative = 1; }
#line 39 "src/readstat_por_parse.rl"
	{ temp_val = 0; val_start = p; }
#line 31 "src/readstat_por_parse.rl"
	{
            if ((*p) >= '0' && (*p) <= '9') {
                temp_val = 30 * temp_val + ((*p) - '0');
            } else if ((*p) >= 'A' && (*p) <= 'T') {
                temp_val = 30 * temp_val + (10 + (*p) - 'A');
            }
        }
	goto st8;
st8:
	p += 1;
case 8:
#line 272 "src/readstat_por_parse.c"
	if ( (*p) == 47 )
		goto tr18;
	if ( (*p) > 57 ) {
		if ( 65 <= (*p) && (*p) <= 84 )
			goto tr19;
	} else if ( (*p) >= 48 )
		goto tr19;
	goto st0;
tr13:
#line 43 "src/readstat_por_parse.rl"
	{ num = temp_val; }
	goto st9;
tr23:
#line 41 "src/readstat_por_parse.rl"
	{ frac = temp_val; frac_len = (p - val_start); }
	goto st9;
st9:
	p += 1;
case 9:
#line 292 "src/readstat_por_parse.c"
	if ( (*p) > 57 ) {
		if ( 65 <= (*p) && (*p) <= 84 )
			goto tr20;
	} else if ( (*p) >= 48 )
		goto tr20;
	goto st0;
tr14:
#line 43 "src/readstat_por_parse.rl"
	{ num = temp_val; }
	goto st10;
st10:
	p += 1;
case 10:
#line 306 "src/readstat_por_parse.c"
	if ( (*p) > 57 ) {
		if ( 65 <= (*p) && (*p) <= 84 )
			goto tr21;
	} else if ( (*p) >= 48 )
		goto tr21;
	goto st0;
tr21:
#line 39 "src/readstat_por_parse.rl"
	{ temp_val = 0; val_start = p; }
#line 31 "src/readstat_por_parse.rl"
	{
            if ((*p) >= '0' && (*p) <= '9') {
                temp_val = 30 * temp_val + ((*p) - '0');
            } else if ((*p) >= 'A' && (*p) <= 'T') {
                temp_val = 30 * temp_val + (10 + (*p) - 'A');
            }
        }
	goto st11;
tr24:
#line 31 "src/readstat_por_parse.rl"
	{
            if ((*p) >= '0' && (*p) <= '9') {
                temp_val = 30 * temp_val + ((*p) - '0');
            } else if ((*p) >= 'A' && (*p) <= 'T') {
                temp_val = 30 * temp_val + (10 + (*p) - 'A');
            }
        }
	goto st11;
st11:
	p += 1;
case 11:
#line 338 "src/readstat_por_parse.c"
	switch( (*p) ) {
		case 43: goto tr22;
		case 45: goto tr23;
		case 47: goto tr10;
	}
	if ( (*p) > 57 ) {
		if ( 65 <= (*p) && (*p) <= 84 )
			goto tr24;
	} else if ( (*p) >= 48 )
		goto tr24;
	goto st0;
	}

	if ( p == eof )
	{
	switch ( cs ) {
	case 12: 
#line 48 "src/readstat_por_parse.rl"
	{ val = NAN; }
	break;
#line 359 "src/readstat_por_parse.c"
	}
	}

	_out: {}
	}

#line 54 "src/readstat_por_parse.rl"


    val = 1.0 * num;
    if (frac_len)
        val += frac / pow(30.0, frac_len);
    if (exp_is_negative)
        exp *= -1;
    if (exp) {
        val *= pow(10.0, exp);
    }
    if (is_negative)
        val *= -1;
    
    if (!success) {
        retval = -1;
        if (error_cb) {
            char error_buf[1024];
            snprintf(error_buf, sizeof(error_buf), "Read bytes: %ld Ending state: %d\n", (long)(p - (const unsigned char *)data), cs);
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
