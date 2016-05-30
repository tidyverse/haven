
#line 1 "src/readstat_sav_parse.rl"

#include <stdlib.h>
#include "readstat_sav.h"
#include "readstat_sav_parse.h"

typedef struct varlookup {
    char      name[8*4+1];
    int       index;
} varlookup_t;

static int compare_key_varlookup(const void *elem1, const void *elem2) {
    const char *key = (const char *)elem1;
    const varlookup_t *v = (const varlookup_t *)elem2;
    return strcmp(key, v->name);
}

static int compare_varlookups(const void *elem1, const void *elem2) {
    const varlookup_t *v1 = (const varlookup_t *)elem1;
    const varlookup_t *v2 = (const varlookup_t *)elem2;
    return strcmp(v1->name, v2->name);
}

static void build_lookup_table(varlookup_t *table, int var_count, sav_ctx_t *ctx) {
    int offset = 0;
    int i;
    for (i=0; i<ctx->var_index;) {
        spss_varinfo_t *info = &ctx->varinfo[i];
        varlookup_t *entry = &table[offset++];

        memcpy(entry->name, info->name, sizeof(info->name));
        entry->index = info->index;

        i += info->n_segments;
    }
    qsort(table, var_count, sizeof(varlookup_t), &compare_varlookups);
}


#line 42 "src/readstat_sav_parse.c"
static const int sav_long_variable_parse_start = 1;

static const int sav_long_variable_parse_en_main = 1;


#line 42 "src/readstat_sav_parse.rl"


readstat_error_t sav_parse_long_variable_names_record(void *data, int count, sav_ctx_t *ctx) {
    unsigned char *c_data = (unsigned char *)data;
    int var_count = ctx->var_index;
    varlookup_t *table = malloc(var_count * sizeof(varlookup_t));
    readstat_error_t retval = READSTAT_OK;

    char temp_key[4*8+1];
    char temp_val[4*64+1];
    unsigned char *str_start = NULL;
    size_t str_len = 0;
    
    char error_buf[8192];
    unsigned char *p = NULL;
    unsigned char *pe = NULL;
    unsigned char *output_buffer = NULL;

    build_lookup_table(table, var_count, ctx);

    if (ctx->converter) {
        size_t input_len = count;
        size_t output_len = input_len * 4;
        pe = p = output_buffer = malloc(output_len);
        size_t status = iconv(ctx->converter, 
                (readstat_iconv_inbuf_t)&data, &input_len,
                (char **)&pe, &output_len);
        if (status == (size_t)-1) {
            free(output_buffer);
            return READSTAT_ERROR_PARSE;
        }
    } else {
        p = c_data;
        pe = c_data + count;
    }
    unsigned char *eof = pe;

    int cs;

    
#line 89 "src/readstat_sav_parse.c"
	{
	cs = sav_long_variable_parse_start;
	}

#line 94 "src/readstat_sav_parse.c"
	{
	if ( p == pe )
		goto _test_eof;
	switch ( cs )
	{
case 1:
	if ( (*p) < 192u ) {
		if ( 64u <= (*p) && (*p) <= 90u )
			goto tr0;
	} else if ( (*p) > 223u ) {
		if ( (*p) > 239u ) {
			if ( 240u <= (*p) && (*p) <= 247u )
				goto tr4;
		} else if ( (*p) >= 224u )
			goto tr3;
	} else
		goto tr2;
	goto st0;
st0:
cs = 0;
	goto _out;
tr0:
#line 109 "src/readstat_sav_parse.rl"
	{ str_start = p; }
	goto st2;
st2:
	if ( ++p == pe )
		goto _test_eof2;
case 2:
#line 124 "src/readstat_sav_parse.c"
	switch( (*p) ) {
		case 46u: goto st3;
		case 61u: goto tr6;
		case 95u: goto st3;
	}
	if ( (*p) < 64u ) {
		if ( (*p) > 36u ) {
			if ( 48u <= (*p) && (*p) <= 57u )
				goto st3;
		} else if ( (*p) >= 35u )
			goto st3;
	} else if ( (*p) > 90u ) {
		if ( (*p) < 224u ) {
			if ( 192u <= (*p) && (*p) <= 223u )
				goto st224;
		} else if ( (*p) > 239u ) {
			if ( 240u <= (*p) && (*p) <= 247u )
				goto st226;
		} else
			goto st225;
	} else
		goto st3;
	goto st0;
st3:
	if ( ++p == pe )
		goto _test_eof3;
case 3:
	switch( (*p) ) {
		case 46u: goto st4;
		case 61u: goto tr6;
		case 95u: goto st4;
	}
	if ( (*p) < 64u ) {
		if ( (*p) > 36u ) {
			if ( 48u <= (*p) && (*p) <= 57u )
				goto st4;
		} else if ( (*p) >= 35u )
			goto st4;
	} else if ( (*p) > 90u ) {
		if ( (*p) < 224u ) {
			if ( 192u <= (*p) && (*p) <= 223u )
				goto st221;
		} else if ( (*p) > 239u ) {
			if ( 240u <= (*p) && (*p) <= 247u )
				goto st223;
		} else
			goto st222;
	} else
		goto st4;
	goto st0;
st4:
	if ( ++p == pe )
		goto _test_eof4;
case 4:
	switch( (*p) ) {
		case 46u: goto st5;
		case 61u: goto tr6;
		case 95u: goto st5;
	}
	if ( (*p) < 64u ) {
		if ( (*p) > 36u ) {
			if ( 48u <= (*p) && (*p) <= 57u )
				goto st5;
		} else if ( (*p) >= 35u )
			goto st5;
	} else if ( (*p) > 90u ) {
		if ( (*p) < 224u ) {
			if ( 192u <= (*p) && (*p) <= 223u )
				goto st218;
		} else if ( (*p) > 239u ) {
			if ( 240u <= (*p) && (*p) <= 247u )
				goto st220;
		} else
			goto st219;
	} else
		goto st5;
	goto st0;
st5:
	if ( ++p == pe )
		goto _test_eof5;
case 5:
	switch( (*p) ) {
		case 46u: goto st6;
		case 61u: goto tr6;
		case 95u: goto st6;
	}
	if ( (*p) < 64u ) {
		if ( (*p) > 36u ) {
			if ( 48u <= (*p) && (*p) <= 57u )
				goto st6;
		} else if ( (*p) >= 35u )
			goto st6;
	} else if ( (*p) > 90u ) {
		if ( (*p) < 224u ) {
			if ( 192u <= (*p) && (*p) <= 223u )
				goto st215;
		} else if ( (*p) > 239u ) {
			if ( 240u <= (*p) && (*p) <= 247u )
				goto st217;
		} else
			goto st216;
	} else
		goto st6;
	goto st0;
st6:
	if ( ++p == pe )
		goto _test_eof6;
case 6:
	switch( (*p) ) {
		case 46u: goto st7;
		case 61u: goto tr6;
		case 95u: goto st7;
	}
	if ( (*p) < 64u ) {
		if ( (*p) > 36u ) {
			if ( 48u <= (*p) && (*p) <= 57u )
				goto st7;
		} else if ( (*p) >= 35u )
			goto st7;
	} else if ( (*p) > 90u ) {
		if ( (*p) < 224u ) {
			if ( 192u <= (*p) && (*p) <= 223u )
				goto st212;
		} else if ( (*p) > 239u ) {
			if ( 240u <= (*p) && (*p) <= 247u )
				goto st214;
		} else
			goto st213;
	} else
		goto st7;
	goto st0;
st7:
	if ( ++p == pe )
		goto _test_eof7;
case 7:
	switch( (*p) ) {
		case 46u: goto st8;
		case 61u: goto tr6;
		case 95u: goto st8;
	}
	if ( (*p) < 64u ) {
		if ( (*p) > 36u ) {
			if ( 48u <= (*p) && (*p) <= 57u )
				goto st8;
		} else if ( (*p) >= 35u )
			goto st8;
	} else if ( (*p) > 90u ) {
		if ( (*p) < 224u ) {
			if ( 192u <= (*p) && (*p) <= 223u )
				goto st209;
		} else if ( (*p) > 239u ) {
			if ( 240u <= (*p) && (*p) <= 247u )
				goto st211;
		} else
			goto st210;
	} else
		goto st8;
	goto st0;
st8:
	if ( ++p == pe )
		goto _test_eof8;
case 8:
	switch( (*p) ) {
		case 46u: goto st9;
		case 61u: goto tr6;
		case 95u: goto st9;
	}
	if ( (*p) < 64u ) {
		if ( (*p) > 36u ) {
			if ( 48u <= (*p) && (*p) <= 57u )
				goto st9;
		} else if ( (*p) >= 35u )
			goto st9;
	} else if ( (*p) > 90u ) {
		if ( (*p) < 224u ) {
			if ( 192u <= (*p) && (*p) <= 223u )
				goto st206;
		} else if ( (*p) > 239u ) {
			if ( 240u <= (*p) && (*p) <= 247u )
				goto st208;
		} else
			goto st207;
	} else
		goto st9;
	goto st0;
st9:
	if ( ++p == pe )
		goto _test_eof9;
case 9:
	if ( (*p) == 61u )
		goto tr6;
	goto st0;
tr6:
#line 109 "src/readstat_sav_parse.rl"
	{ str_len = p - str_start; }
#line 93 "src/readstat_sav_parse.rl"
	{
            memcpy(temp_key, str_start, str_len);
            temp_key[str_len] = '\0';
        }
	goto st10;
st10:
	if ( ++p == pe )
		goto _test_eof10;
case 10:
#line 330 "src/readstat_sav_parse.c"
	if ( (*p) < 192u ) {
		if ( 32u <= (*p) && (*p) <= 126u )
			goto tr34;
	} else if ( (*p) > 223u ) {
		if ( (*p) > 239u ) {
			if ( 240u <= (*p) && (*p) <= 247u )
				goto tr37;
		} else if ( (*p) >= 224u )
			goto tr36;
	} else
		goto tr35;
	goto st0;
tr34:
#line 111 "src/readstat_sav_parse.rl"
	{ str_start = p; }
	goto st227;
st227:
	if ( ++p == pe )
		goto _test_eof227;
case 227:
#line 351 "src/readstat_sav_parse.c"
	if ( (*p) == 9u )
		goto tr233;
	if ( (*p) < 192u ) {
		if ( 32u <= (*p) && (*p) <= 126u )
			goto st229;
	} else if ( (*p) > 223u ) {
		if ( (*p) > 239u ) {
			if ( 240u <= (*p) && (*p) <= 247u )
				goto st202;
		} else if ( (*p) >= 224u )
			goto st201;
	} else
		goto st200;
	goto st0;
tr233:
#line 111 "src/readstat_sav_parse.rl"
	{ str_len = p - str_start; }
#line 98 "src/readstat_sav_parse.rl"
	{
            memcpy(temp_val, str_start, str_len);
            temp_val[str_len] = '\0';
        }
#line 82 "src/readstat_sav_parse.rl"
	{
            varlookup_t *found = bsearch(temp_key, table, var_count, sizeof(varlookup_t), &compare_key_varlookup);
            if (found) {
                memcpy(ctx->varinfo[found->index].longname, temp_val, str_len);
                ctx->varinfo[found->index].longname[str_len] = '\0';
            } else if (ctx->error_handler) {
                snprintf(error_buf, sizeof(error_buf), "Failed to find %s\n", temp_key);
                ctx->error_handler(error_buf, ctx->user_ctx);
            }
        }
	goto st228;
st228:
	if ( ++p == pe )
		goto _test_eof228;
case 228:
#line 390 "src/readstat_sav_parse.c"
	if ( (*p) < 192u ) {
		if ( 64u <= (*p) && (*p) <= 90u )
			goto tr0;
	} else if ( (*p) > 223u ) {
		if ( (*p) > 239u ) {
			if ( 240u <= (*p) && (*p) <= 247u )
				goto tr4;
		} else if ( (*p) >= 224u )
			goto tr3;
	} else
		goto tr2;
	goto st0;
tr2:
#line 109 "src/readstat_sav_parse.rl"
	{ str_start = p; }
	goto st11;
st11:
	if ( ++p == pe )
		goto _test_eof11;
case 11:
#line 411 "src/readstat_sav_parse.c"
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st2;
	goto st0;
tr3:
#line 109 "src/readstat_sav_parse.rl"
	{ str_start = p; }
	goto st12;
st12:
	if ( ++p == pe )
		goto _test_eof12;
case 12:
#line 423 "src/readstat_sav_parse.c"
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st11;
	goto st0;
tr4:
#line 109 "src/readstat_sav_parse.rl"
	{ str_start = p; }
	goto st13;
st13:
	if ( ++p == pe )
		goto _test_eof13;
case 13:
#line 435 "src/readstat_sav_parse.c"
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st12;
	goto st0;
st229:
	if ( ++p == pe )
		goto _test_eof229;
case 229:
	if ( (*p) == 9u )
		goto tr233;
	if ( (*p) < 192u ) {
		if ( 32u <= (*p) && (*p) <= 126u )
			goto st230;
	} else if ( (*p) > 223u ) {
		if ( (*p) > 239u ) {
			if ( 240u <= (*p) && (*p) <= 247u )
				goto st199;
		} else if ( (*p) >= 224u )
			goto st198;
	} else
		goto st197;
	goto st0;
st230:
	if ( ++p == pe )
		goto _test_eof230;
case 230:
	if ( (*p) == 9u )
		goto tr233;
	if ( (*p) < 192u ) {
		if ( 32u <= (*p) && (*p) <= 126u )
			goto st231;
	} else if ( (*p) > 223u ) {
		if ( (*p) > 239u ) {
			if ( 240u <= (*p) && (*p) <= 247u )
				goto st196;
		} else if ( (*p) >= 224u )
			goto st195;
	} else
		goto st194;
	goto st0;
st231:
	if ( ++p == pe )
		goto _test_eof231;
case 231:
	if ( (*p) == 9u )
		goto tr233;
	if ( (*p) < 192u ) {
		if ( 32u <= (*p) && (*p) <= 126u )
			goto st232;
	} else if ( (*p) > 223u ) {
		if ( (*p) > 239u ) {
			if ( 240u <= (*p) && (*p) <= 247u )
				goto st193;
		} else if ( (*p) >= 224u )
			goto st192;
	} else
		goto st191;
	goto st0;
st232:
	if ( ++p == pe )
		goto _test_eof232;
case 232:
	if ( (*p) == 9u )
		goto tr233;
	if ( (*p) < 192u ) {
		if ( 32u <= (*p) && (*p) <= 126u )
			goto st233;
	} else if ( (*p) > 223u ) {
		if ( (*p) > 239u ) {
			if ( 240u <= (*p) && (*p) <= 247u )
				goto st190;
		} else if ( (*p) >= 224u )
			goto st189;
	} else
		goto st188;
	goto st0;
st233:
	if ( ++p == pe )
		goto _test_eof233;
case 233:
	if ( (*p) == 9u )
		goto tr233;
	if ( (*p) < 192u ) {
		if ( 32u <= (*p) && (*p) <= 126u )
			goto st234;
	} else if ( (*p) > 223u ) {
		if ( (*p) > 239u ) {
			if ( 240u <= (*p) && (*p) <= 247u )
				goto st187;
		} else if ( (*p) >= 224u )
			goto st186;
	} else
		goto st185;
	goto st0;
st234:
	if ( ++p == pe )
		goto _test_eof234;
case 234:
	if ( (*p) == 9u )
		goto tr233;
	if ( (*p) < 192u ) {
		if ( 32u <= (*p) && (*p) <= 126u )
			goto st235;
	} else if ( (*p) > 223u ) {
		if ( (*p) > 239u ) {
			if ( 240u <= (*p) && (*p) <= 247u )
				goto st184;
		} else if ( (*p) >= 224u )
			goto st183;
	} else
		goto st182;
	goto st0;
st235:
	if ( ++p == pe )
		goto _test_eof235;
case 235:
	if ( (*p) == 9u )
		goto tr233;
	if ( (*p) < 192u ) {
		if ( 32u <= (*p) && (*p) <= 126u )
			goto st236;
	} else if ( (*p) > 223u ) {
		if ( (*p) > 239u ) {
			if ( 240u <= (*p) && (*p) <= 247u )
				goto st181;
		} else if ( (*p) >= 224u )
			goto st180;
	} else
		goto st179;
	goto st0;
st236:
	if ( ++p == pe )
		goto _test_eof236;
case 236:
	if ( (*p) == 9u )
		goto tr233;
	if ( (*p) < 192u ) {
		if ( 32u <= (*p) && (*p) <= 126u )
			goto st237;
	} else if ( (*p) > 223u ) {
		if ( (*p) > 239u ) {
			if ( 240u <= (*p) && (*p) <= 247u )
				goto st178;
		} else if ( (*p) >= 224u )
			goto st177;
	} else
		goto st176;
	goto st0;
st237:
	if ( ++p == pe )
		goto _test_eof237;
case 237:
	if ( (*p) == 9u )
		goto tr233;
	if ( (*p) < 192u ) {
		if ( 32u <= (*p) && (*p) <= 126u )
			goto st238;
	} else if ( (*p) > 223u ) {
		if ( (*p) > 239u ) {
			if ( 240u <= (*p) && (*p) <= 247u )
				goto st175;
		} else if ( (*p) >= 224u )
			goto st174;
	} else
		goto st173;
	goto st0;
st238:
	if ( ++p == pe )
		goto _test_eof238;
case 238:
	if ( (*p) == 9u )
		goto tr233;
	if ( (*p) < 192u ) {
		if ( 32u <= (*p) && (*p) <= 126u )
			goto st239;
	} else if ( (*p) > 223u ) {
		if ( (*p) > 239u ) {
			if ( 240u <= (*p) && (*p) <= 247u )
				goto st172;
		} else if ( (*p) >= 224u )
			goto st171;
	} else
		goto st170;
	goto st0;
st239:
	if ( ++p == pe )
		goto _test_eof239;
case 239:
	if ( (*p) == 9u )
		goto tr233;
	if ( (*p) < 192u ) {
		if ( 32u <= (*p) && (*p) <= 126u )
			goto st240;
	} else if ( (*p) > 223u ) {
		if ( (*p) > 239u ) {
			if ( 240u <= (*p) && (*p) <= 247u )
				goto st169;
		} else if ( (*p) >= 224u )
			goto st168;
	} else
		goto st167;
	goto st0;
st240:
	if ( ++p == pe )
		goto _test_eof240;
case 240:
	if ( (*p) == 9u )
		goto tr233;
	if ( (*p) < 192u ) {
		if ( 32u <= (*p) && (*p) <= 126u )
			goto st241;
	} else if ( (*p) > 223u ) {
		if ( (*p) > 239u ) {
			if ( 240u <= (*p) && (*p) <= 247u )
				goto st166;
		} else if ( (*p) >= 224u )
			goto st165;
	} else
		goto st164;
	goto st0;
st241:
	if ( ++p == pe )
		goto _test_eof241;
case 241:
	if ( (*p) == 9u )
		goto tr233;
	if ( (*p) < 192u ) {
		if ( 32u <= (*p) && (*p) <= 126u )
			goto st242;
	} else if ( (*p) > 223u ) {
		if ( (*p) > 239u ) {
			if ( 240u <= (*p) && (*p) <= 247u )
				goto st163;
		} else if ( (*p) >= 224u )
			goto st162;
	} else
		goto st161;
	goto st0;
st242:
	if ( ++p == pe )
		goto _test_eof242;
case 242:
	if ( (*p) == 9u )
		goto tr233;
	if ( (*p) < 192u ) {
		if ( 32u <= (*p) && (*p) <= 126u )
			goto st243;
	} else if ( (*p) > 223u ) {
		if ( (*p) > 239u ) {
			if ( 240u <= (*p) && (*p) <= 247u )
				goto st160;
		} else if ( (*p) >= 224u )
			goto st159;
	} else
		goto st158;
	goto st0;
st243:
	if ( ++p == pe )
		goto _test_eof243;
case 243:
	if ( (*p) == 9u )
		goto tr233;
	if ( (*p) < 192u ) {
		if ( 32u <= (*p) && (*p) <= 126u )
			goto st244;
	} else if ( (*p) > 223u ) {
		if ( (*p) > 239u ) {
			if ( 240u <= (*p) && (*p) <= 247u )
				goto st157;
		} else if ( (*p) >= 224u )
			goto st156;
	} else
		goto st155;
	goto st0;
st244:
	if ( ++p == pe )
		goto _test_eof244;
case 244:
	if ( (*p) == 9u )
		goto tr233;
	if ( (*p) < 192u ) {
		if ( 32u <= (*p) && (*p) <= 126u )
			goto st245;
	} else if ( (*p) > 223u ) {
		if ( (*p) > 239u ) {
			if ( 240u <= (*p) && (*p) <= 247u )
				goto st154;
		} else if ( (*p) >= 224u )
			goto st153;
	} else
		goto st152;
	goto st0;
st245:
	if ( ++p == pe )
		goto _test_eof245;
case 245:
	if ( (*p) == 9u )
		goto tr233;
	if ( (*p) < 192u ) {
		if ( 32u <= (*p) && (*p) <= 126u )
			goto st246;
	} else if ( (*p) > 223u ) {
		if ( (*p) > 239u ) {
			if ( 240u <= (*p) && (*p) <= 247u )
				goto st151;
		} else if ( (*p) >= 224u )
			goto st150;
	} else
		goto st149;
	goto st0;
st246:
	if ( ++p == pe )
		goto _test_eof246;
case 246:
	if ( (*p) == 9u )
		goto tr233;
	if ( (*p) < 192u ) {
		if ( 32u <= (*p) && (*p) <= 126u )
			goto st247;
	} else if ( (*p) > 223u ) {
		if ( (*p) > 239u ) {
			if ( 240u <= (*p) && (*p) <= 247u )
				goto st148;
		} else if ( (*p) >= 224u )
			goto st147;
	} else
		goto st146;
	goto st0;
st247:
	if ( ++p == pe )
		goto _test_eof247;
case 247:
	if ( (*p) == 9u )
		goto tr233;
	if ( (*p) < 192u ) {
		if ( 32u <= (*p) && (*p) <= 126u )
			goto st248;
	} else if ( (*p) > 223u ) {
		if ( (*p) > 239u ) {
			if ( 240u <= (*p) && (*p) <= 247u )
				goto st145;
		} else if ( (*p) >= 224u )
			goto st144;
	} else
		goto st143;
	goto st0;
st248:
	if ( ++p == pe )
		goto _test_eof248;
case 248:
	if ( (*p) == 9u )
		goto tr233;
	if ( (*p) < 192u ) {
		if ( 32u <= (*p) && (*p) <= 126u )
			goto st249;
	} else if ( (*p) > 223u ) {
		if ( (*p) > 239u ) {
			if ( 240u <= (*p) && (*p) <= 247u )
				goto st142;
		} else if ( (*p) >= 224u )
			goto st141;
	} else
		goto st140;
	goto st0;
st249:
	if ( ++p == pe )
		goto _test_eof249;
case 249:
	if ( (*p) == 9u )
		goto tr233;
	if ( (*p) < 192u ) {
		if ( 32u <= (*p) && (*p) <= 126u )
			goto st250;
	} else if ( (*p) > 223u ) {
		if ( (*p) > 239u ) {
			if ( 240u <= (*p) && (*p) <= 247u )
				goto st139;
		} else if ( (*p) >= 224u )
			goto st138;
	} else
		goto st137;
	goto st0;
st250:
	if ( ++p == pe )
		goto _test_eof250;
case 250:
	if ( (*p) == 9u )
		goto tr233;
	if ( (*p) < 192u ) {
		if ( 32u <= (*p) && (*p) <= 126u )
			goto st251;
	} else if ( (*p) > 223u ) {
		if ( (*p) > 239u ) {
			if ( 240u <= (*p) && (*p) <= 247u )
				goto st136;
		} else if ( (*p) >= 224u )
			goto st135;
	} else
		goto st134;
	goto st0;
st251:
	if ( ++p == pe )
		goto _test_eof251;
case 251:
	if ( (*p) == 9u )
		goto tr233;
	if ( (*p) < 192u ) {
		if ( 32u <= (*p) && (*p) <= 126u )
			goto st252;
	} else if ( (*p) > 223u ) {
		if ( (*p) > 239u ) {
			if ( 240u <= (*p) && (*p) <= 247u )
				goto st133;
		} else if ( (*p) >= 224u )
			goto st132;
	} else
		goto st131;
	goto st0;
st252:
	if ( ++p == pe )
		goto _test_eof252;
case 252:
	if ( (*p) == 9u )
		goto tr233;
	if ( (*p) < 192u ) {
		if ( 32u <= (*p) && (*p) <= 126u )
			goto st253;
	} else if ( (*p) > 223u ) {
		if ( (*p) > 239u ) {
			if ( 240u <= (*p) && (*p) <= 247u )
				goto st130;
		} else if ( (*p) >= 224u )
			goto st129;
	} else
		goto st128;
	goto st0;
st253:
	if ( ++p == pe )
		goto _test_eof253;
case 253:
	if ( (*p) == 9u )
		goto tr233;
	if ( (*p) < 192u ) {
		if ( 32u <= (*p) && (*p) <= 126u )
			goto st254;
	} else if ( (*p) > 223u ) {
		if ( (*p) > 239u ) {
			if ( 240u <= (*p) && (*p) <= 247u )
				goto st127;
		} else if ( (*p) >= 224u )
			goto st126;
	} else
		goto st125;
	goto st0;
st254:
	if ( ++p == pe )
		goto _test_eof254;
case 254:
	if ( (*p) == 9u )
		goto tr233;
	if ( (*p) < 192u ) {
		if ( 32u <= (*p) && (*p) <= 126u )
			goto st255;
	} else if ( (*p) > 223u ) {
		if ( (*p) > 239u ) {
			if ( 240u <= (*p) && (*p) <= 247u )
				goto st124;
		} else if ( (*p) >= 224u )
			goto st123;
	} else
		goto st122;
	goto st0;
st255:
	if ( ++p == pe )
		goto _test_eof255;
case 255:
	if ( (*p) == 9u )
		goto tr233;
	if ( (*p) < 192u ) {
		if ( 32u <= (*p) && (*p) <= 126u )
			goto st256;
	} else if ( (*p) > 223u ) {
		if ( (*p) > 239u ) {
			if ( 240u <= (*p) && (*p) <= 247u )
				goto st121;
		} else if ( (*p) >= 224u )
			goto st120;
	} else
		goto st119;
	goto st0;
st256:
	if ( ++p == pe )
		goto _test_eof256;
case 256:
	if ( (*p) == 9u )
		goto tr233;
	if ( (*p) < 192u ) {
		if ( 32u <= (*p) && (*p) <= 126u )
			goto st257;
	} else if ( (*p) > 223u ) {
		if ( (*p) > 239u ) {
			if ( 240u <= (*p) && (*p) <= 247u )
				goto st118;
		} else if ( (*p) >= 224u )
			goto st117;
	} else
		goto st116;
	goto st0;
st257:
	if ( ++p == pe )
		goto _test_eof257;
case 257:
	if ( (*p) == 9u )
		goto tr233;
	if ( (*p) < 192u ) {
		if ( 32u <= (*p) && (*p) <= 126u )
			goto st258;
	} else if ( (*p) > 223u ) {
		if ( (*p) > 239u ) {
			if ( 240u <= (*p) && (*p) <= 247u )
				goto st115;
		} else if ( (*p) >= 224u )
			goto st114;
	} else
		goto st113;
	goto st0;
st258:
	if ( ++p == pe )
		goto _test_eof258;
case 258:
	if ( (*p) == 9u )
		goto tr233;
	if ( (*p) < 192u ) {
		if ( 32u <= (*p) && (*p) <= 126u )
			goto st259;
	} else if ( (*p) > 223u ) {
		if ( (*p) > 239u ) {
			if ( 240u <= (*p) && (*p) <= 247u )
				goto st112;
		} else if ( (*p) >= 224u )
			goto st111;
	} else
		goto st110;
	goto st0;
st259:
	if ( ++p == pe )
		goto _test_eof259;
case 259:
	if ( (*p) == 9u )
		goto tr233;
	if ( (*p) < 192u ) {
		if ( 32u <= (*p) && (*p) <= 126u )
			goto st260;
	} else if ( (*p) > 223u ) {
		if ( (*p) > 239u ) {
			if ( 240u <= (*p) && (*p) <= 247u )
				goto st109;
		} else if ( (*p) >= 224u )
			goto st108;
	} else
		goto st107;
	goto st0;
st260:
	if ( ++p == pe )
		goto _test_eof260;
case 260:
	if ( (*p) == 9u )
		goto tr233;
	if ( (*p) < 192u ) {
		if ( 32u <= (*p) && (*p) <= 126u )
			goto st261;
	} else if ( (*p) > 223u ) {
		if ( (*p) > 239u ) {
			if ( 240u <= (*p) && (*p) <= 247u )
				goto st106;
		} else if ( (*p) >= 224u )
			goto st105;
	} else
		goto st104;
	goto st0;
st261:
	if ( ++p == pe )
		goto _test_eof261;
case 261:
	if ( (*p) == 9u )
		goto tr233;
	if ( (*p) < 192u ) {
		if ( 32u <= (*p) && (*p) <= 126u )
			goto st262;
	} else if ( (*p) > 223u ) {
		if ( (*p) > 239u ) {
			if ( 240u <= (*p) && (*p) <= 247u )
				goto st103;
		} else if ( (*p) >= 224u )
			goto st102;
	} else
		goto st101;
	goto st0;
st262:
	if ( ++p == pe )
		goto _test_eof262;
case 262:
	if ( (*p) == 9u )
		goto tr233;
	if ( (*p) < 192u ) {
		if ( 32u <= (*p) && (*p) <= 126u )
			goto st263;
	} else if ( (*p) > 223u ) {
		if ( (*p) > 239u ) {
			if ( 240u <= (*p) && (*p) <= 247u )
				goto st100;
		} else if ( (*p) >= 224u )
			goto st99;
	} else
		goto st98;
	goto st0;
st263:
	if ( ++p == pe )
		goto _test_eof263;
case 263:
	if ( (*p) == 9u )
		goto tr233;
	if ( (*p) < 192u ) {
		if ( 32u <= (*p) && (*p) <= 126u )
			goto st264;
	} else if ( (*p) > 223u ) {
		if ( (*p) > 239u ) {
			if ( 240u <= (*p) && (*p) <= 247u )
				goto st97;
		} else if ( (*p) >= 224u )
			goto st96;
	} else
		goto st95;
	goto st0;
st264:
	if ( ++p == pe )
		goto _test_eof264;
case 264:
	if ( (*p) == 9u )
		goto tr233;
	if ( (*p) < 192u ) {
		if ( 32u <= (*p) && (*p) <= 126u )
			goto st265;
	} else if ( (*p) > 223u ) {
		if ( (*p) > 239u ) {
			if ( 240u <= (*p) && (*p) <= 247u )
				goto st94;
		} else if ( (*p) >= 224u )
			goto st93;
	} else
		goto st92;
	goto st0;
st265:
	if ( ++p == pe )
		goto _test_eof265;
case 265:
	if ( (*p) == 9u )
		goto tr233;
	if ( (*p) < 192u ) {
		if ( 32u <= (*p) && (*p) <= 126u )
			goto st266;
	} else if ( (*p) > 223u ) {
		if ( (*p) > 239u ) {
			if ( 240u <= (*p) && (*p) <= 247u )
				goto st91;
		} else if ( (*p) >= 224u )
			goto st90;
	} else
		goto st89;
	goto st0;
st266:
	if ( ++p == pe )
		goto _test_eof266;
case 266:
	if ( (*p) == 9u )
		goto tr233;
	if ( (*p) < 192u ) {
		if ( 32u <= (*p) && (*p) <= 126u )
			goto st267;
	} else if ( (*p) > 223u ) {
		if ( (*p) > 239u ) {
			if ( 240u <= (*p) && (*p) <= 247u )
				goto st88;
		} else if ( (*p) >= 224u )
			goto st87;
	} else
		goto st86;
	goto st0;
st267:
	if ( ++p == pe )
		goto _test_eof267;
case 267:
	if ( (*p) == 9u )
		goto tr233;
	if ( (*p) < 192u ) {
		if ( 32u <= (*p) && (*p) <= 126u )
			goto st268;
	} else if ( (*p) > 223u ) {
		if ( (*p) > 239u ) {
			if ( 240u <= (*p) && (*p) <= 247u )
				goto st85;
		} else if ( (*p) >= 224u )
			goto st84;
	} else
		goto st83;
	goto st0;
st268:
	if ( ++p == pe )
		goto _test_eof268;
case 268:
	if ( (*p) == 9u )
		goto tr233;
	if ( (*p) < 192u ) {
		if ( 32u <= (*p) && (*p) <= 126u )
			goto st269;
	} else if ( (*p) > 223u ) {
		if ( (*p) > 239u ) {
			if ( 240u <= (*p) && (*p) <= 247u )
				goto st82;
		} else if ( (*p) >= 224u )
			goto st81;
	} else
		goto st80;
	goto st0;
st269:
	if ( ++p == pe )
		goto _test_eof269;
case 269:
	if ( (*p) == 9u )
		goto tr233;
	if ( (*p) < 192u ) {
		if ( 32u <= (*p) && (*p) <= 126u )
			goto st270;
	} else if ( (*p) > 223u ) {
		if ( (*p) > 239u ) {
			if ( 240u <= (*p) && (*p) <= 247u )
				goto st79;
		} else if ( (*p) >= 224u )
			goto st78;
	} else
		goto st77;
	goto st0;
st270:
	if ( ++p == pe )
		goto _test_eof270;
case 270:
	if ( (*p) == 9u )
		goto tr233;
	if ( (*p) < 192u ) {
		if ( 32u <= (*p) && (*p) <= 126u )
			goto st271;
	} else if ( (*p) > 223u ) {
		if ( (*p) > 239u ) {
			if ( 240u <= (*p) && (*p) <= 247u )
				goto st76;
		} else if ( (*p) >= 224u )
			goto st75;
	} else
		goto st74;
	goto st0;
st271:
	if ( ++p == pe )
		goto _test_eof271;
case 271:
	if ( (*p) == 9u )
		goto tr233;
	if ( (*p) < 192u ) {
		if ( 32u <= (*p) && (*p) <= 126u )
			goto st272;
	} else if ( (*p) > 223u ) {
		if ( (*p) > 239u ) {
			if ( 240u <= (*p) && (*p) <= 247u )
				goto st73;
		} else if ( (*p) >= 224u )
			goto st72;
	} else
		goto st71;
	goto st0;
st272:
	if ( ++p == pe )
		goto _test_eof272;
case 272:
	if ( (*p) == 9u )
		goto tr233;
	if ( (*p) < 192u ) {
		if ( 32u <= (*p) && (*p) <= 126u )
			goto st273;
	} else if ( (*p) > 223u ) {
		if ( (*p) > 239u ) {
			if ( 240u <= (*p) && (*p) <= 247u )
				goto st70;
		} else if ( (*p) >= 224u )
			goto st69;
	} else
		goto st68;
	goto st0;
st273:
	if ( ++p == pe )
		goto _test_eof273;
case 273:
	if ( (*p) == 9u )
		goto tr233;
	if ( (*p) < 192u ) {
		if ( 32u <= (*p) && (*p) <= 126u )
			goto st274;
	} else if ( (*p) > 223u ) {
		if ( (*p) > 239u ) {
			if ( 240u <= (*p) && (*p) <= 247u )
				goto st67;
		} else if ( (*p) >= 224u )
			goto st66;
	} else
		goto st65;
	goto st0;
st274:
	if ( ++p == pe )
		goto _test_eof274;
case 274:
	if ( (*p) == 9u )
		goto tr233;
	if ( (*p) < 192u ) {
		if ( 32u <= (*p) && (*p) <= 126u )
			goto st275;
	} else if ( (*p) > 223u ) {
		if ( (*p) > 239u ) {
			if ( 240u <= (*p) && (*p) <= 247u )
				goto st64;
		} else if ( (*p) >= 224u )
			goto st63;
	} else
		goto st62;
	goto st0;
st275:
	if ( ++p == pe )
		goto _test_eof275;
case 275:
	if ( (*p) == 9u )
		goto tr233;
	if ( (*p) < 192u ) {
		if ( 32u <= (*p) && (*p) <= 126u )
			goto st276;
	} else if ( (*p) > 223u ) {
		if ( (*p) > 239u ) {
			if ( 240u <= (*p) && (*p) <= 247u )
				goto st61;
		} else if ( (*p) >= 224u )
			goto st60;
	} else
		goto st59;
	goto st0;
st276:
	if ( ++p == pe )
		goto _test_eof276;
case 276:
	if ( (*p) == 9u )
		goto tr233;
	if ( (*p) < 192u ) {
		if ( 32u <= (*p) && (*p) <= 126u )
			goto st277;
	} else if ( (*p) > 223u ) {
		if ( (*p) > 239u ) {
			if ( 240u <= (*p) && (*p) <= 247u )
				goto st58;
		} else if ( (*p) >= 224u )
			goto st57;
	} else
		goto st56;
	goto st0;
st277:
	if ( ++p == pe )
		goto _test_eof277;
case 277:
	if ( (*p) == 9u )
		goto tr233;
	if ( (*p) < 192u ) {
		if ( 32u <= (*p) && (*p) <= 126u )
			goto st278;
	} else if ( (*p) > 223u ) {
		if ( (*p) > 239u ) {
			if ( 240u <= (*p) && (*p) <= 247u )
				goto st55;
		} else if ( (*p) >= 224u )
			goto st54;
	} else
		goto st53;
	goto st0;
st278:
	if ( ++p == pe )
		goto _test_eof278;
case 278:
	if ( (*p) == 9u )
		goto tr233;
	if ( (*p) < 192u ) {
		if ( 32u <= (*p) && (*p) <= 126u )
			goto st279;
	} else if ( (*p) > 223u ) {
		if ( (*p) > 239u ) {
			if ( 240u <= (*p) && (*p) <= 247u )
				goto st52;
		} else if ( (*p) >= 224u )
			goto st51;
	} else
		goto st50;
	goto st0;
st279:
	if ( ++p == pe )
		goto _test_eof279;
case 279:
	if ( (*p) == 9u )
		goto tr233;
	if ( (*p) < 192u ) {
		if ( 32u <= (*p) && (*p) <= 126u )
			goto st280;
	} else if ( (*p) > 223u ) {
		if ( (*p) > 239u ) {
			if ( 240u <= (*p) && (*p) <= 247u )
				goto st49;
		} else if ( (*p) >= 224u )
			goto st48;
	} else
		goto st47;
	goto st0;
st280:
	if ( ++p == pe )
		goto _test_eof280;
case 280:
	if ( (*p) == 9u )
		goto tr233;
	if ( (*p) < 192u ) {
		if ( 32u <= (*p) && (*p) <= 126u )
			goto st281;
	} else if ( (*p) > 223u ) {
		if ( (*p) > 239u ) {
			if ( 240u <= (*p) && (*p) <= 247u )
				goto st46;
		} else if ( (*p) >= 224u )
			goto st45;
	} else
		goto st44;
	goto st0;
st281:
	if ( ++p == pe )
		goto _test_eof281;
case 281:
	if ( (*p) == 9u )
		goto tr233;
	if ( (*p) < 192u ) {
		if ( 32u <= (*p) && (*p) <= 126u )
			goto st282;
	} else if ( (*p) > 223u ) {
		if ( (*p) > 239u ) {
			if ( 240u <= (*p) && (*p) <= 247u )
				goto st43;
		} else if ( (*p) >= 224u )
			goto st42;
	} else
		goto st41;
	goto st0;
st282:
	if ( ++p == pe )
		goto _test_eof282;
case 282:
	if ( (*p) == 9u )
		goto tr233;
	if ( (*p) < 192u ) {
		if ( 32u <= (*p) && (*p) <= 126u )
			goto st283;
	} else if ( (*p) > 223u ) {
		if ( (*p) > 239u ) {
			if ( 240u <= (*p) && (*p) <= 247u )
				goto st40;
		} else if ( (*p) >= 224u )
			goto st39;
	} else
		goto st38;
	goto st0;
st283:
	if ( ++p == pe )
		goto _test_eof283;
case 283:
	if ( (*p) == 9u )
		goto tr233;
	if ( (*p) < 192u ) {
		if ( 32u <= (*p) && (*p) <= 126u )
			goto st284;
	} else if ( (*p) > 223u ) {
		if ( (*p) > 239u ) {
			if ( 240u <= (*p) && (*p) <= 247u )
				goto st37;
		} else if ( (*p) >= 224u )
			goto st36;
	} else
		goto st35;
	goto st0;
st284:
	if ( ++p == pe )
		goto _test_eof284;
case 284:
	if ( (*p) == 9u )
		goto tr233;
	if ( (*p) < 192u ) {
		if ( 32u <= (*p) && (*p) <= 126u )
			goto st285;
	} else if ( (*p) > 223u ) {
		if ( (*p) > 239u ) {
			if ( 240u <= (*p) && (*p) <= 247u )
				goto st34;
		} else if ( (*p) >= 224u )
			goto st33;
	} else
		goto st32;
	goto st0;
st285:
	if ( ++p == pe )
		goto _test_eof285;
case 285:
	if ( (*p) == 9u )
		goto tr233;
	if ( (*p) < 192u ) {
		if ( 32u <= (*p) && (*p) <= 126u )
			goto st286;
	} else if ( (*p) > 223u ) {
		if ( (*p) > 239u ) {
			if ( 240u <= (*p) && (*p) <= 247u )
				goto st31;
		} else if ( (*p) >= 224u )
			goto st30;
	} else
		goto st29;
	goto st0;
st286:
	if ( ++p == pe )
		goto _test_eof286;
case 286:
	if ( (*p) == 9u )
		goto tr233;
	if ( (*p) < 192u ) {
		if ( 32u <= (*p) && (*p) <= 126u )
			goto st287;
	} else if ( (*p) > 223u ) {
		if ( (*p) > 239u ) {
			if ( 240u <= (*p) && (*p) <= 247u )
				goto st28;
		} else if ( (*p) >= 224u )
			goto st27;
	} else
		goto st26;
	goto st0;
st287:
	if ( ++p == pe )
		goto _test_eof287;
case 287:
	if ( (*p) == 9u )
		goto tr233;
	if ( (*p) < 192u ) {
		if ( 32u <= (*p) && (*p) <= 126u )
			goto st288;
	} else if ( (*p) > 223u ) {
		if ( (*p) > 239u ) {
			if ( 240u <= (*p) && (*p) <= 247u )
				goto st25;
		} else if ( (*p) >= 224u )
			goto st24;
	} else
		goto st23;
	goto st0;
st288:
	if ( ++p == pe )
		goto _test_eof288;
case 288:
	if ( (*p) == 9u )
		goto tr233;
	if ( (*p) < 192u ) {
		if ( 32u <= (*p) && (*p) <= 126u )
			goto st289;
	} else if ( (*p) > 223u ) {
		if ( (*p) > 239u ) {
			if ( 240u <= (*p) && (*p) <= 247u )
				goto st22;
		} else if ( (*p) >= 224u )
			goto st21;
	} else
		goto st20;
	goto st0;
st289:
	if ( ++p == pe )
		goto _test_eof289;
case 289:
	if ( (*p) == 9u )
		goto tr233;
	if ( (*p) < 192u ) {
		if ( 32u <= (*p) && (*p) <= 126u )
			goto st290;
	} else if ( (*p) > 223u ) {
		if ( (*p) > 239u ) {
			if ( 240u <= (*p) && (*p) <= 247u )
				goto st19;
		} else if ( (*p) >= 224u )
			goto st18;
	} else
		goto st17;
	goto st0;
st290:
	if ( ++p == pe )
		goto _test_eof290;
case 290:
	if ( (*p) == 9u )
		goto tr233;
	if ( (*p) < 192u ) {
		if ( 32u <= (*p) && (*p) <= 126u )
			goto st291;
	} else if ( (*p) > 223u ) {
		if ( (*p) > 239u ) {
			if ( 240u <= (*p) && (*p) <= 247u )
				goto st16;
		} else if ( (*p) >= 224u )
			goto st15;
	} else
		goto st14;
	goto st0;
st291:
	if ( ++p == pe )
		goto _test_eof291;
case 291:
	if ( (*p) == 9u )
		goto tr233;
	goto st0;
st14:
	if ( ++p == pe )
		goto _test_eof14;
case 14:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st291;
	goto st0;
st15:
	if ( ++p == pe )
		goto _test_eof15;
case 15:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st14;
	goto st0;
st16:
	if ( ++p == pe )
		goto _test_eof16;
case 16:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st15;
	goto st0;
st17:
	if ( ++p == pe )
		goto _test_eof17;
case 17:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st290;
	goto st0;
st18:
	if ( ++p == pe )
		goto _test_eof18;
case 18:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st17;
	goto st0;
st19:
	if ( ++p == pe )
		goto _test_eof19;
case 19:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st18;
	goto st0;
st20:
	if ( ++p == pe )
		goto _test_eof20;
case 20:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st289;
	goto st0;
st21:
	if ( ++p == pe )
		goto _test_eof21;
case 21:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st20;
	goto st0;
st22:
	if ( ++p == pe )
		goto _test_eof22;
case 22:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st21;
	goto st0;
st23:
	if ( ++p == pe )
		goto _test_eof23;
case 23:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st288;
	goto st0;
st24:
	if ( ++p == pe )
		goto _test_eof24;
case 24:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st23;
	goto st0;
st25:
	if ( ++p == pe )
		goto _test_eof25;
case 25:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st24;
	goto st0;
st26:
	if ( ++p == pe )
		goto _test_eof26;
case 26:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st287;
	goto st0;
st27:
	if ( ++p == pe )
		goto _test_eof27;
case 27:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st26;
	goto st0;
st28:
	if ( ++p == pe )
		goto _test_eof28;
case 28:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st27;
	goto st0;
st29:
	if ( ++p == pe )
		goto _test_eof29;
case 29:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st286;
	goto st0;
st30:
	if ( ++p == pe )
		goto _test_eof30;
case 30:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st29;
	goto st0;
st31:
	if ( ++p == pe )
		goto _test_eof31;
case 31:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st30;
	goto st0;
st32:
	if ( ++p == pe )
		goto _test_eof32;
case 32:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st285;
	goto st0;
st33:
	if ( ++p == pe )
		goto _test_eof33;
case 33:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st32;
	goto st0;
st34:
	if ( ++p == pe )
		goto _test_eof34;
case 34:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st33;
	goto st0;
st35:
	if ( ++p == pe )
		goto _test_eof35;
case 35:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st284;
	goto st0;
st36:
	if ( ++p == pe )
		goto _test_eof36;
case 36:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st35;
	goto st0;
st37:
	if ( ++p == pe )
		goto _test_eof37;
case 37:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st36;
	goto st0;
st38:
	if ( ++p == pe )
		goto _test_eof38;
case 38:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st283;
	goto st0;
st39:
	if ( ++p == pe )
		goto _test_eof39;
case 39:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st38;
	goto st0;
st40:
	if ( ++p == pe )
		goto _test_eof40;
case 40:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st39;
	goto st0;
st41:
	if ( ++p == pe )
		goto _test_eof41;
case 41:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st282;
	goto st0;
st42:
	if ( ++p == pe )
		goto _test_eof42;
case 42:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st41;
	goto st0;
st43:
	if ( ++p == pe )
		goto _test_eof43;
case 43:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st42;
	goto st0;
st44:
	if ( ++p == pe )
		goto _test_eof44;
case 44:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st281;
	goto st0;
st45:
	if ( ++p == pe )
		goto _test_eof45;
case 45:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st44;
	goto st0;
st46:
	if ( ++p == pe )
		goto _test_eof46;
case 46:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st45;
	goto st0;
st47:
	if ( ++p == pe )
		goto _test_eof47;
case 47:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st280;
	goto st0;
st48:
	if ( ++p == pe )
		goto _test_eof48;
case 48:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st47;
	goto st0;
st49:
	if ( ++p == pe )
		goto _test_eof49;
case 49:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st48;
	goto st0;
st50:
	if ( ++p == pe )
		goto _test_eof50;
case 50:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st279;
	goto st0;
st51:
	if ( ++p == pe )
		goto _test_eof51;
case 51:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st50;
	goto st0;
st52:
	if ( ++p == pe )
		goto _test_eof52;
case 52:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st51;
	goto st0;
st53:
	if ( ++p == pe )
		goto _test_eof53;
case 53:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st278;
	goto st0;
st54:
	if ( ++p == pe )
		goto _test_eof54;
case 54:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st53;
	goto st0;
st55:
	if ( ++p == pe )
		goto _test_eof55;
case 55:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st54;
	goto st0;
st56:
	if ( ++p == pe )
		goto _test_eof56;
case 56:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st277;
	goto st0;
st57:
	if ( ++p == pe )
		goto _test_eof57;
case 57:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st56;
	goto st0;
st58:
	if ( ++p == pe )
		goto _test_eof58;
case 58:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st57;
	goto st0;
st59:
	if ( ++p == pe )
		goto _test_eof59;
case 59:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st276;
	goto st0;
st60:
	if ( ++p == pe )
		goto _test_eof60;
case 60:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st59;
	goto st0;
st61:
	if ( ++p == pe )
		goto _test_eof61;
case 61:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st60;
	goto st0;
st62:
	if ( ++p == pe )
		goto _test_eof62;
case 62:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st275;
	goto st0;
st63:
	if ( ++p == pe )
		goto _test_eof63;
case 63:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st62;
	goto st0;
st64:
	if ( ++p == pe )
		goto _test_eof64;
case 64:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st63;
	goto st0;
st65:
	if ( ++p == pe )
		goto _test_eof65;
case 65:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st274;
	goto st0;
st66:
	if ( ++p == pe )
		goto _test_eof66;
case 66:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st65;
	goto st0;
st67:
	if ( ++p == pe )
		goto _test_eof67;
case 67:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st66;
	goto st0;
st68:
	if ( ++p == pe )
		goto _test_eof68;
case 68:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st273;
	goto st0;
st69:
	if ( ++p == pe )
		goto _test_eof69;
case 69:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st68;
	goto st0;
st70:
	if ( ++p == pe )
		goto _test_eof70;
case 70:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st69;
	goto st0;
st71:
	if ( ++p == pe )
		goto _test_eof71;
case 71:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st272;
	goto st0;
st72:
	if ( ++p == pe )
		goto _test_eof72;
case 72:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st71;
	goto st0;
st73:
	if ( ++p == pe )
		goto _test_eof73;
case 73:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st72;
	goto st0;
st74:
	if ( ++p == pe )
		goto _test_eof74;
case 74:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st271;
	goto st0;
st75:
	if ( ++p == pe )
		goto _test_eof75;
case 75:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st74;
	goto st0;
st76:
	if ( ++p == pe )
		goto _test_eof76;
case 76:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st75;
	goto st0;
st77:
	if ( ++p == pe )
		goto _test_eof77;
case 77:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st270;
	goto st0;
st78:
	if ( ++p == pe )
		goto _test_eof78;
case 78:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st77;
	goto st0;
st79:
	if ( ++p == pe )
		goto _test_eof79;
case 79:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st78;
	goto st0;
st80:
	if ( ++p == pe )
		goto _test_eof80;
case 80:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st269;
	goto st0;
st81:
	if ( ++p == pe )
		goto _test_eof81;
case 81:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st80;
	goto st0;
st82:
	if ( ++p == pe )
		goto _test_eof82;
case 82:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st81;
	goto st0;
st83:
	if ( ++p == pe )
		goto _test_eof83;
case 83:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st268;
	goto st0;
st84:
	if ( ++p == pe )
		goto _test_eof84;
case 84:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st83;
	goto st0;
st85:
	if ( ++p == pe )
		goto _test_eof85;
case 85:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st84;
	goto st0;
st86:
	if ( ++p == pe )
		goto _test_eof86;
case 86:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st267;
	goto st0;
st87:
	if ( ++p == pe )
		goto _test_eof87;
case 87:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st86;
	goto st0;
st88:
	if ( ++p == pe )
		goto _test_eof88;
case 88:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st87;
	goto st0;
st89:
	if ( ++p == pe )
		goto _test_eof89;
case 89:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st266;
	goto st0;
st90:
	if ( ++p == pe )
		goto _test_eof90;
case 90:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st89;
	goto st0;
st91:
	if ( ++p == pe )
		goto _test_eof91;
case 91:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st90;
	goto st0;
st92:
	if ( ++p == pe )
		goto _test_eof92;
case 92:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st265;
	goto st0;
st93:
	if ( ++p == pe )
		goto _test_eof93;
case 93:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st92;
	goto st0;
st94:
	if ( ++p == pe )
		goto _test_eof94;
case 94:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st93;
	goto st0;
st95:
	if ( ++p == pe )
		goto _test_eof95;
case 95:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st264;
	goto st0;
st96:
	if ( ++p == pe )
		goto _test_eof96;
case 96:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st95;
	goto st0;
st97:
	if ( ++p == pe )
		goto _test_eof97;
case 97:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st96;
	goto st0;
st98:
	if ( ++p == pe )
		goto _test_eof98;
case 98:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st263;
	goto st0;
st99:
	if ( ++p == pe )
		goto _test_eof99;
case 99:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st98;
	goto st0;
st100:
	if ( ++p == pe )
		goto _test_eof100;
case 100:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st99;
	goto st0;
st101:
	if ( ++p == pe )
		goto _test_eof101;
case 101:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st262;
	goto st0;
st102:
	if ( ++p == pe )
		goto _test_eof102;
case 102:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st101;
	goto st0;
st103:
	if ( ++p == pe )
		goto _test_eof103;
case 103:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st102;
	goto st0;
st104:
	if ( ++p == pe )
		goto _test_eof104;
case 104:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st261;
	goto st0;
st105:
	if ( ++p == pe )
		goto _test_eof105;
case 105:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st104;
	goto st0;
st106:
	if ( ++p == pe )
		goto _test_eof106;
case 106:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st105;
	goto st0;
st107:
	if ( ++p == pe )
		goto _test_eof107;
case 107:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st260;
	goto st0;
st108:
	if ( ++p == pe )
		goto _test_eof108;
case 108:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st107;
	goto st0;
st109:
	if ( ++p == pe )
		goto _test_eof109;
case 109:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st108;
	goto st0;
st110:
	if ( ++p == pe )
		goto _test_eof110;
case 110:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st259;
	goto st0;
st111:
	if ( ++p == pe )
		goto _test_eof111;
case 111:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st110;
	goto st0;
st112:
	if ( ++p == pe )
		goto _test_eof112;
case 112:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st111;
	goto st0;
st113:
	if ( ++p == pe )
		goto _test_eof113;
case 113:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st258;
	goto st0;
st114:
	if ( ++p == pe )
		goto _test_eof114;
case 114:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st113;
	goto st0;
st115:
	if ( ++p == pe )
		goto _test_eof115;
case 115:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st114;
	goto st0;
st116:
	if ( ++p == pe )
		goto _test_eof116;
case 116:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st257;
	goto st0;
st117:
	if ( ++p == pe )
		goto _test_eof117;
case 117:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st116;
	goto st0;
st118:
	if ( ++p == pe )
		goto _test_eof118;
case 118:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st117;
	goto st0;
st119:
	if ( ++p == pe )
		goto _test_eof119;
case 119:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st256;
	goto st0;
st120:
	if ( ++p == pe )
		goto _test_eof120;
case 120:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st119;
	goto st0;
st121:
	if ( ++p == pe )
		goto _test_eof121;
case 121:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st120;
	goto st0;
st122:
	if ( ++p == pe )
		goto _test_eof122;
case 122:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st255;
	goto st0;
st123:
	if ( ++p == pe )
		goto _test_eof123;
case 123:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st122;
	goto st0;
st124:
	if ( ++p == pe )
		goto _test_eof124;
case 124:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st123;
	goto st0;
st125:
	if ( ++p == pe )
		goto _test_eof125;
case 125:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st254;
	goto st0;
st126:
	if ( ++p == pe )
		goto _test_eof126;
case 126:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st125;
	goto st0;
st127:
	if ( ++p == pe )
		goto _test_eof127;
case 127:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st126;
	goto st0;
st128:
	if ( ++p == pe )
		goto _test_eof128;
case 128:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st253;
	goto st0;
st129:
	if ( ++p == pe )
		goto _test_eof129;
case 129:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st128;
	goto st0;
st130:
	if ( ++p == pe )
		goto _test_eof130;
case 130:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st129;
	goto st0;
st131:
	if ( ++p == pe )
		goto _test_eof131;
case 131:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st252;
	goto st0;
st132:
	if ( ++p == pe )
		goto _test_eof132;
case 132:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st131;
	goto st0;
st133:
	if ( ++p == pe )
		goto _test_eof133;
case 133:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st132;
	goto st0;
st134:
	if ( ++p == pe )
		goto _test_eof134;
case 134:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st251;
	goto st0;
st135:
	if ( ++p == pe )
		goto _test_eof135;
case 135:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st134;
	goto st0;
st136:
	if ( ++p == pe )
		goto _test_eof136;
case 136:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st135;
	goto st0;
st137:
	if ( ++p == pe )
		goto _test_eof137;
case 137:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st250;
	goto st0;
st138:
	if ( ++p == pe )
		goto _test_eof138;
case 138:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st137;
	goto st0;
st139:
	if ( ++p == pe )
		goto _test_eof139;
case 139:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st138;
	goto st0;
st140:
	if ( ++p == pe )
		goto _test_eof140;
case 140:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st249;
	goto st0;
st141:
	if ( ++p == pe )
		goto _test_eof141;
case 141:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st140;
	goto st0;
st142:
	if ( ++p == pe )
		goto _test_eof142;
case 142:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st141;
	goto st0;
st143:
	if ( ++p == pe )
		goto _test_eof143;
case 143:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st248;
	goto st0;
st144:
	if ( ++p == pe )
		goto _test_eof144;
case 144:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st143;
	goto st0;
st145:
	if ( ++p == pe )
		goto _test_eof145;
case 145:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st144;
	goto st0;
st146:
	if ( ++p == pe )
		goto _test_eof146;
case 146:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st247;
	goto st0;
st147:
	if ( ++p == pe )
		goto _test_eof147;
case 147:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st146;
	goto st0;
st148:
	if ( ++p == pe )
		goto _test_eof148;
case 148:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st147;
	goto st0;
st149:
	if ( ++p == pe )
		goto _test_eof149;
case 149:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st246;
	goto st0;
st150:
	if ( ++p == pe )
		goto _test_eof150;
case 150:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st149;
	goto st0;
st151:
	if ( ++p == pe )
		goto _test_eof151;
case 151:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st150;
	goto st0;
st152:
	if ( ++p == pe )
		goto _test_eof152;
case 152:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st245;
	goto st0;
st153:
	if ( ++p == pe )
		goto _test_eof153;
case 153:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st152;
	goto st0;
st154:
	if ( ++p == pe )
		goto _test_eof154;
case 154:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st153;
	goto st0;
st155:
	if ( ++p == pe )
		goto _test_eof155;
case 155:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st244;
	goto st0;
st156:
	if ( ++p == pe )
		goto _test_eof156;
case 156:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st155;
	goto st0;
st157:
	if ( ++p == pe )
		goto _test_eof157;
case 157:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st156;
	goto st0;
st158:
	if ( ++p == pe )
		goto _test_eof158;
case 158:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st243;
	goto st0;
st159:
	if ( ++p == pe )
		goto _test_eof159;
case 159:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st158;
	goto st0;
st160:
	if ( ++p == pe )
		goto _test_eof160;
case 160:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st159;
	goto st0;
st161:
	if ( ++p == pe )
		goto _test_eof161;
case 161:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st242;
	goto st0;
st162:
	if ( ++p == pe )
		goto _test_eof162;
case 162:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st161;
	goto st0;
st163:
	if ( ++p == pe )
		goto _test_eof163;
case 163:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st162;
	goto st0;
st164:
	if ( ++p == pe )
		goto _test_eof164;
case 164:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st241;
	goto st0;
st165:
	if ( ++p == pe )
		goto _test_eof165;
case 165:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st164;
	goto st0;
st166:
	if ( ++p == pe )
		goto _test_eof166;
case 166:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st165;
	goto st0;
st167:
	if ( ++p == pe )
		goto _test_eof167;
case 167:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st240;
	goto st0;
st168:
	if ( ++p == pe )
		goto _test_eof168;
case 168:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st167;
	goto st0;
st169:
	if ( ++p == pe )
		goto _test_eof169;
case 169:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st168;
	goto st0;
st170:
	if ( ++p == pe )
		goto _test_eof170;
case 170:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st239;
	goto st0;
st171:
	if ( ++p == pe )
		goto _test_eof171;
case 171:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st170;
	goto st0;
st172:
	if ( ++p == pe )
		goto _test_eof172;
case 172:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st171;
	goto st0;
st173:
	if ( ++p == pe )
		goto _test_eof173;
case 173:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st238;
	goto st0;
st174:
	if ( ++p == pe )
		goto _test_eof174;
case 174:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st173;
	goto st0;
st175:
	if ( ++p == pe )
		goto _test_eof175;
case 175:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st174;
	goto st0;
st176:
	if ( ++p == pe )
		goto _test_eof176;
case 176:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st237;
	goto st0;
st177:
	if ( ++p == pe )
		goto _test_eof177;
case 177:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st176;
	goto st0;
st178:
	if ( ++p == pe )
		goto _test_eof178;
case 178:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st177;
	goto st0;
st179:
	if ( ++p == pe )
		goto _test_eof179;
case 179:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st236;
	goto st0;
st180:
	if ( ++p == pe )
		goto _test_eof180;
case 180:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st179;
	goto st0;
st181:
	if ( ++p == pe )
		goto _test_eof181;
case 181:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st180;
	goto st0;
st182:
	if ( ++p == pe )
		goto _test_eof182;
case 182:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st235;
	goto st0;
st183:
	if ( ++p == pe )
		goto _test_eof183;
case 183:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st182;
	goto st0;
st184:
	if ( ++p == pe )
		goto _test_eof184;
case 184:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st183;
	goto st0;
st185:
	if ( ++p == pe )
		goto _test_eof185;
case 185:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st234;
	goto st0;
st186:
	if ( ++p == pe )
		goto _test_eof186;
case 186:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st185;
	goto st0;
st187:
	if ( ++p == pe )
		goto _test_eof187;
case 187:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st186;
	goto st0;
st188:
	if ( ++p == pe )
		goto _test_eof188;
case 188:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st233;
	goto st0;
st189:
	if ( ++p == pe )
		goto _test_eof189;
case 189:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st188;
	goto st0;
st190:
	if ( ++p == pe )
		goto _test_eof190;
case 190:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st189;
	goto st0;
st191:
	if ( ++p == pe )
		goto _test_eof191;
case 191:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st232;
	goto st0;
st192:
	if ( ++p == pe )
		goto _test_eof192;
case 192:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st191;
	goto st0;
st193:
	if ( ++p == pe )
		goto _test_eof193;
case 193:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st192;
	goto st0;
st194:
	if ( ++p == pe )
		goto _test_eof194;
case 194:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st231;
	goto st0;
st195:
	if ( ++p == pe )
		goto _test_eof195;
case 195:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st194;
	goto st0;
st196:
	if ( ++p == pe )
		goto _test_eof196;
case 196:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st195;
	goto st0;
st197:
	if ( ++p == pe )
		goto _test_eof197;
case 197:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st230;
	goto st0;
st198:
	if ( ++p == pe )
		goto _test_eof198;
case 198:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st197;
	goto st0;
st199:
	if ( ++p == pe )
		goto _test_eof199;
case 199:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st198;
	goto st0;
st200:
	if ( ++p == pe )
		goto _test_eof200;
case 200:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st229;
	goto st0;
st201:
	if ( ++p == pe )
		goto _test_eof201;
case 201:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st200;
	goto st0;
st202:
	if ( ++p == pe )
		goto _test_eof202;
case 202:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st201;
	goto st0;
tr35:
#line 111 "src/readstat_sav_parse.rl"
	{ str_start = p; }
	goto st203;
st203:
	if ( ++p == pe )
		goto _test_eof203;
case 203:
#line 2893 "src/readstat_sav_parse.c"
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st227;
	goto st0;
tr36:
#line 111 "src/readstat_sav_parse.rl"
	{ str_start = p; }
	goto st204;
st204:
	if ( ++p == pe )
		goto _test_eof204;
case 204:
#line 2905 "src/readstat_sav_parse.c"
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st203;
	goto st0;
tr37:
#line 111 "src/readstat_sav_parse.rl"
	{ str_start = p; }
	goto st205;
st205:
	if ( ++p == pe )
		goto _test_eof205;
case 205:
#line 2917 "src/readstat_sav_parse.c"
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st204;
	goto st0;
st206:
	if ( ++p == pe )
		goto _test_eof206;
case 206:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st9;
	goto st0;
st207:
	if ( ++p == pe )
		goto _test_eof207;
case 207:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st206;
	goto st0;
st208:
	if ( ++p == pe )
		goto _test_eof208;
case 208:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st207;
	goto st0;
st209:
	if ( ++p == pe )
		goto _test_eof209;
case 209:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st8;
	goto st0;
st210:
	if ( ++p == pe )
		goto _test_eof210;
case 210:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st209;
	goto st0;
st211:
	if ( ++p == pe )
		goto _test_eof211;
case 211:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st210;
	goto st0;
st212:
	if ( ++p == pe )
		goto _test_eof212;
case 212:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st7;
	goto st0;
st213:
	if ( ++p == pe )
		goto _test_eof213;
case 213:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st212;
	goto st0;
st214:
	if ( ++p == pe )
		goto _test_eof214;
case 214:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st213;
	goto st0;
st215:
	if ( ++p == pe )
		goto _test_eof215;
case 215:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st6;
	goto st0;
st216:
	if ( ++p == pe )
		goto _test_eof216;
case 216:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st215;
	goto st0;
st217:
	if ( ++p == pe )
		goto _test_eof217;
case 217:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st216;
	goto st0;
st218:
	if ( ++p == pe )
		goto _test_eof218;
case 218:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st5;
	goto st0;
st219:
	if ( ++p == pe )
		goto _test_eof219;
case 219:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st218;
	goto st0;
st220:
	if ( ++p == pe )
		goto _test_eof220;
case 220:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st219;
	goto st0;
st221:
	if ( ++p == pe )
		goto _test_eof221;
case 221:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st4;
	goto st0;
st222:
	if ( ++p == pe )
		goto _test_eof222;
case 222:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st221;
	goto st0;
st223:
	if ( ++p == pe )
		goto _test_eof223;
case 223:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st222;
	goto st0;
st224:
	if ( ++p == pe )
		goto _test_eof224;
case 224:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st3;
	goto st0;
st225:
	if ( ++p == pe )
		goto _test_eof225;
case 225:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st224;
	goto st0;
st226:
	if ( ++p == pe )
		goto _test_eof226;
case 226:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st225;
	goto st0;
	}
	_test_eof2: cs = 2; goto _test_eof; 
	_test_eof3: cs = 3; goto _test_eof; 
	_test_eof4: cs = 4; goto _test_eof; 
	_test_eof5: cs = 5; goto _test_eof; 
	_test_eof6: cs = 6; goto _test_eof; 
	_test_eof7: cs = 7; goto _test_eof; 
	_test_eof8: cs = 8; goto _test_eof; 
	_test_eof9: cs = 9; goto _test_eof; 
	_test_eof10: cs = 10; goto _test_eof; 
	_test_eof227: cs = 227; goto _test_eof; 
	_test_eof228: cs = 228; goto _test_eof; 
	_test_eof11: cs = 11; goto _test_eof; 
	_test_eof12: cs = 12; goto _test_eof; 
	_test_eof13: cs = 13; goto _test_eof; 
	_test_eof229: cs = 229; goto _test_eof; 
	_test_eof230: cs = 230; goto _test_eof; 
	_test_eof231: cs = 231; goto _test_eof; 
	_test_eof232: cs = 232; goto _test_eof; 
	_test_eof233: cs = 233; goto _test_eof; 
	_test_eof234: cs = 234; goto _test_eof; 
	_test_eof235: cs = 235; goto _test_eof; 
	_test_eof236: cs = 236; goto _test_eof; 
	_test_eof237: cs = 237; goto _test_eof; 
	_test_eof238: cs = 238; goto _test_eof; 
	_test_eof239: cs = 239; goto _test_eof; 
	_test_eof240: cs = 240; goto _test_eof; 
	_test_eof241: cs = 241; goto _test_eof; 
	_test_eof242: cs = 242; goto _test_eof; 
	_test_eof243: cs = 243; goto _test_eof; 
	_test_eof244: cs = 244; goto _test_eof; 
	_test_eof245: cs = 245; goto _test_eof; 
	_test_eof246: cs = 246; goto _test_eof; 
	_test_eof247: cs = 247; goto _test_eof; 
	_test_eof248: cs = 248; goto _test_eof; 
	_test_eof249: cs = 249; goto _test_eof; 
	_test_eof250: cs = 250; goto _test_eof; 
	_test_eof251: cs = 251; goto _test_eof; 
	_test_eof252: cs = 252; goto _test_eof; 
	_test_eof253: cs = 253; goto _test_eof; 
	_test_eof254: cs = 254; goto _test_eof; 
	_test_eof255: cs = 255; goto _test_eof; 
	_test_eof256: cs = 256; goto _test_eof; 
	_test_eof257: cs = 257; goto _test_eof; 
	_test_eof258: cs = 258; goto _test_eof; 
	_test_eof259: cs = 259; goto _test_eof; 
	_test_eof260: cs = 260; goto _test_eof; 
	_test_eof261: cs = 261; goto _test_eof; 
	_test_eof262: cs = 262; goto _test_eof; 
	_test_eof263: cs = 263; goto _test_eof; 
	_test_eof264: cs = 264; goto _test_eof; 
	_test_eof265: cs = 265; goto _test_eof; 
	_test_eof266: cs = 266; goto _test_eof; 
	_test_eof267: cs = 267; goto _test_eof; 
	_test_eof268: cs = 268; goto _test_eof; 
	_test_eof269: cs = 269; goto _test_eof; 
	_test_eof270: cs = 270; goto _test_eof; 
	_test_eof271: cs = 271; goto _test_eof; 
	_test_eof272: cs = 272; goto _test_eof; 
	_test_eof273: cs = 273; goto _test_eof; 
	_test_eof274: cs = 274; goto _test_eof; 
	_test_eof275: cs = 275; goto _test_eof; 
	_test_eof276: cs = 276; goto _test_eof; 
	_test_eof277: cs = 277; goto _test_eof; 
	_test_eof278: cs = 278; goto _test_eof; 
	_test_eof279: cs = 279; goto _test_eof; 
	_test_eof280: cs = 280; goto _test_eof; 
	_test_eof281: cs = 281; goto _test_eof; 
	_test_eof282: cs = 282; goto _test_eof; 
	_test_eof283: cs = 283; goto _test_eof; 
	_test_eof284: cs = 284; goto _test_eof; 
	_test_eof285: cs = 285; goto _test_eof; 
	_test_eof286: cs = 286; goto _test_eof; 
	_test_eof287: cs = 287; goto _test_eof; 
	_test_eof288: cs = 288; goto _test_eof; 
	_test_eof289: cs = 289; goto _test_eof; 
	_test_eof290: cs = 290; goto _test_eof; 
	_test_eof291: cs = 291; goto _test_eof; 
	_test_eof14: cs = 14; goto _test_eof; 
	_test_eof15: cs = 15; goto _test_eof; 
	_test_eof16: cs = 16; goto _test_eof; 
	_test_eof17: cs = 17; goto _test_eof; 
	_test_eof18: cs = 18; goto _test_eof; 
	_test_eof19: cs = 19; goto _test_eof; 
	_test_eof20: cs = 20; goto _test_eof; 
	_test_eof21: cs = 21; goto _test_eof; 
	_test_eof22: cs = 22; goto _test_eof; 
	_test_eof23: cs = 23; goto _test_eof; 
	_test_eof24: cs = 24; goto _test_eof; 
	_test_eof25: cs = 25; goto _test_eof; 
	_test_eof26: cs = 26; goto _test_eof; 
	_test_eof27: cs = 27; goto _test_eof; 
	_test_eof28: cs = 28; goto _test_eof; 
	_test_eof29: cs = 29; goto _test_eof; 
	_test_eof30: cs = 30; goto _test_eof; 
	_test_eof31: cs = 31; goto _test_eof; 
	_test_eof32: cs = 32; goto _test_eof; 
	_test_eof33: cs = 33; goto _test_eof; 
	_test_eof34: cs = 34; goto _test_eof; 
	_test_eof35: cs = 35; goto _test_eof; 
	_test_eof36: cs = 36; goto _test_eof; 
	_test_eof37: cs = 37; goto _test_eof; 
	_test_eof38: cs = 38; goto _test_eof; 
	_test_eof39: cs = 39; goto _test_eof; 
	_test_eof40: cs = 40; goto _test_eof; 
	_test_eof41: cs = 41; goto _test_eof; 
	_test_eof42: cs = 42; goto _test_eof; 
	_test_eof43: cs = 43; goto _test_eof; 
	_test_eof44: cs = 44; goto _test_eof; 
	_test_eof45: cs = 45; goto _test_eof; 
	_test_eof46: cs = 46; goto _test_eof; 
	_test_eof47: cs = 47; goto _test_eof; 
	_test_eof48: cs = 48; goto _test_eof; 
	_test_eof49: cs = 49; goto _test_eof; 
	_test_eof50: cs = 50; goto _test_eof; 
	_test_eof51: cs = 51; goto _test_eof; 
	_test_eof52: cs = 52; goto _test_eof; 
	_test_eof53: cs = 53; goto _test_eof; 
	_test_eof54: cs = 54; goto _test_eof; 
	_test_eof55: cs = 55; goto _test_eof; 
	_test_eof56: cs = 56; goto _test_eof; 
	_test_eof57: cs = 57; goto _test_eof; 
	_test_eof58: cs = 58; goto _test_eof; 
	_test_eof59: cs = 59; goto _test_eof; 
	_test_eof60: cs = 60; goto _test_eof; 
	_test_eof61: cs = 61; goto _test_eof; 
	_test_eof62: cs = 62; goto _test_eof; 
	_test_eof63: cs = 63; goto _test_eof; 
	_test_eof64: cs = 64; goto _test_eof; 
	_test_eof65: cs = 65; goto _test_eof; 
	_test_eof66: cs = 66; goto _test_eof; 
	_test_eof67: cs = 67; goto _test_eof; 
	_test_eof68: cs = 68; goto _test_eof; 
	_test_eof69: cs = 69; goto _test_eof; 
	_test_eof70: cs = 70; goto _test_eof; 
	_test_eof71: cs = 71; goto _test_eof; 
	_test_eof72: cs = 72; goto _test_eof; 
	_test_eof73: cs = 73; goto _test_eof; 
	_test_eof74: cs = 74; goto _test_eof; 
	_test_eof75: cs = 75; goto _test_eof; 
	_test_eof76: cs = 76; goto _test_eof; 
	_test_eof77: cs = 77; goto _test_eof; 
	_test_eof78: cs = 78; goto _test_eof; 
	_test_eof79: cs = 79; goto _test_eof; 
	_test_eof80: cs = 80; goto _test_eof; 
	_test_eof81: cs = 81; goto _test_eof; 
	_test_eof82: cs = 82; goto _test_eof; 
	_test_eof83: cs = 83; goto _test_eof; 
	_test_eof84: cs = 84; goto _test_eof; 
	_test_eof85: cs = 85; goto _test_eof; 
	_test_eof86: cs = 86; goto _test_eof; 
	_test_eof87: cs = 87; goto _test_eof; 
	_test_eof88: cs = 88; goto _test_eof; 
	_test_eof89: cs = 89; goto _test_eof; 
	_test_eof90: cs = 90; goto _test_eof; 
	_test_eof91: cs = 91; goto _test_eof; 
	_test_eof92: cs = 92; goto _test_eof; 
	_test_eof93: cs = 93; goto _test_eof; 
	_test_eof94: cs = 94; goto _test_eof; 
	_test_eof95: cs = 95; goto _test_eof; 
	_test_eof96: cs = 96; goto _test_eof; 
	_test_eof97: cs = 97; goto _test_eof; 
	_test_eof98: cs = 98; goto _test_eof; 
	_test_eof99: cs = 99; goto _test_eof; 
	_test_eof100: cs = 100; goto _test_eof; 
	_test_eof101: cs = 101; goto _test_eof; 
	_test_eof102: cs = 102; goto _test_eof; 
	_test_eof103: cs = 103; goto _test_eof; 
	_test_eof104: cs = 104; goto _test_eof; 
	_test_eof105: cs = 105; goto _test_eof; 
	_test_eof106: cs = 106; goto _test_eof; 
	_test_eof107: cs = 107; goto _test_eof; 
	_test_eof108: cs = 108; goto _test_eof; 
	_test_eof109: cs = 109; goto _test_eof; 
	_test_eof110: cs = 110; goto _test_eof; 
	_test_eof111: cs = 111; goto _test_eof; 
	_test_eof112: cs = 112; goto _test_eof; 
	_test_eof113: cs = 113; goto _test_eof; 
	_test_eof114: cs = 114; goto _test_eof; 
	_test_eof115: cs = 115; goto _test_eof; 
	_test_eof116: cs = 116; goto _test_eof; 
	_test_eof117: cs = 117; goto _test_eof; 
	_test_eof118: cs = 118; goto _test_eof; 
	_test_eof119: cs = 119; goto _test_eof; 
	_test_eof120: cs = 120; goto _test_eof; 
	_test_eof121: cs = 121; goto _test_eof; 
	_test_eof122: cs = 122; goto _test_eof; 
	_test_eof123: cs = 123; goto _test_eof; 
	_test_eof124: cs = 124; goto _test_eof; 
	_test_eof125: cs = 125; goto _test_eof; 
	_test_eof126: cs = 126; goto _test_eof; 
	_test_eof127: cs = 127; goto _test_eof; 
	_test_eof128: cs = 128; goto _test_eof; 
	_test_eof129: cs = 129; goto _test_eof; 
	_test_eof130: cs = 130; goto _test_eof; 
	_test_eof131: cs = 131; goto _test_eof; 
	_test_eof132: cs = 132; goto _test_eof; 
	_test_eof133: cs = 133; goto _test_eof; 
	_test_eof134: cs = 134; goto _test_eof; 
	_test_eof135: cs = 135; goto _test_eof; 
	_test_eof136: cs = 136; goto _test_eof; 
	_test_eof137: cs = 137; goto _test_eof; 
	_test_eof138: cs = 138; goto _test_eof; 
	_test_eof139: cs = 139; goto _test_eof; 
	_test_eof140: cs = 140; goto _test_eof; 
	_test_eof141: cs = 141; goto _test_eof; 
	_test_eof142: cs = 142; goto _test_eof; 
	_test_eof143: cs = 143; goto _test_eof; 
	_test_eof144: cs = 144; goto _test_eof; 
	_test_eof145: cs = 145; goto _test_eof; 
	_test_eof146: cs = 146; goto _test_eof; 
	_test_eof147: cs = 147; goto _test_eof; 
	_test_eof148: cs = 148; goto _test_eof; 
	_test_eof149: cs = 149; goto _test_eof; 
	_test_eof150: cs = 150; goto _test_eof; 
	_test_eof151: cs = 151; goto _test_eof; 
	_test_eof152: cs = 152; goto _test_eof; 
	_test_eof153: cs = 153; goto _test_eof; 
	_test_eof154: cs = 154; goto _test_eof; 
	_test_eof155: cs = 155; goto _test_eof; 
	_test_eof156: cs = 156; goto _test_eof; 
	_test_eof157: cs = 157; goto _test_eof; 
	_test_eof158: cs = 158; goto _test_eof; 
	_test_eof159: cs = 159; goto _test_eof; 
	_test_eof160: cs = 160; goto _test_eof; 
	_test_eof161: cs = 161; goto _test_eof; 
	_test_eof162: cs = 162; goto _test_eof; 
	_test_eof163: cs = 163; goto _test_eof; 
	_test_eof164: cs = 164; goto _test_eof; 
	_test_eof165: cs = 165; goto _test_eof; 
	_test_eof166: cs = 166; goto _test_eof; 
	_test_eof167: cs = 167; goto _test_eof; 
	_test_eof168: cs = 168; goto _test_eof; 
	_test_eof169: cs = 169; goto _test_eof; 
	_test_eof170: cs = 170; goto _test_eof; 
	_test_eof171: cs = 171; goto _test_eof; 
	_test_eof172: cs = 172; goto _test_eof; 
	_test_eof173: cs = 173; goto _test_eof; 
	_test_eof174: cs = 174; goto _test_eof; 
	_test_eof175: cs = 175; goto _test_eof; 
	_test_eof176: cs = 176; goto _test_eof; 
	_test_eof177: cs = 177; goto _test_eof; 
	_test_eof178: cs = 178; goto _test_eof; 
	_test_eof179: cs = 179; goto _test_eof; 
	_test_eof180: cs = 180; goto _test_eof; 
	_test_eof181: cs = 181; goto _test_eof; 
	_test_eof182: cs = 182; goto _test_eof; 
	_test_eof183: cs = 183; goto _test_eof; 
	_test_eof184: cs = 184; goto _test_eof; 
	_test_eof185: cs = 185; goto _test_eof; 
	_test_eof186: cs = 186; goto _test_eof; 
	_test_eof187: cs = 187; goto _test_eof; 
	_test_eof188: cs = 188; goto _test_eof; 
	_test_eof189: cs = 189; goto _test_eof; 
	_test_eof190: cs = 190; goto _test_eof; 
	_test_eof191: cs = 191; goto _test_eof; 
	_test_eof192: cs = 192; goto _test_eof; 
	_test_eof193: cs = 193; goto _test_eof; 
	_test_eof194: cs = 194; goto _test_eof; 
	_test_eof195: cs = 195; goto _test_eof; 
	_test_eof196: cs = 196; goto _test_eof; 
	_test_eof197: cs = 197; goto _test_eof; 
	_test_eof198: cs = 198; goto _test_eof; 
	_test_eof199: cs = 199; goto _test_eof; 
	_test_eof200: cs = 200; goto _test_eof; 
	_test_eof201: cs = 201; goto _test_eof; 
	_test_eof202: cs = 202; goto _test_eof; 
	_test_eof203: cs = 203; goto _test_eof; 
	_test_eof204: cs = 204; goto _test_eof; 
	_test_eof205: cs = 205; goto _test_eof; 
	_test_eof206: cs = 206; goto _test_eof; 
	_test_eof207: cs = 207; goto _test_eof; 
	_test_eof208: cs = 208; goto _test_eof; 
	_test_eof209: cs = 209; goto _test_eof; 
	_test_eof210: cs = 210; goto _test_eof; 
	_test_eof211: cs = 211; goto _test_eof; 
	_test_eof212: cs = 212; goto _test_eof; 
	_test_eof213: cs = 213; goto _test_eof; 
	_test_eof214: cs = 214; goto _test_eof; 
	_test_eof215: cs = 215; goto _test_eof; 
	_test_eof216: cs = 216; goto _test_eof; 
	_test_eof217: cs = 217; goto _test_eof; 
	_test_eof218: cs = 218; goto _test_eof; 
	_test_eof219: cs = 219; goto _test_eof; 
	_test_eof220: cs = 220; goto _test_eof; 
	_test_eof221: cs = 221; goto _test_eof; 
	_test_eof222: cs = 222; goto _test_eof; 
	_test_eof223: cs = 223; goto _test_eof; 
	_test_eof224: cs = 224; goto _test_eof; 
	_test_eof225: cs = 225; goto _test_eof; 
	_test_eof226: cs = 226; goto _test_eof; 

	_test_eof: {}
	if ( p == eof )
	{
	switch ( cs ) {
	case 227: 
	case 229: 
	case 230: 
	case 231: 
	case 232: 
	case 233: 
	case 234: 
	case 235: 
	case 236: 
	case 237: 
	case 238: 
	case 239: 
	case 240: 
	case 241: 
	case 242: 
	case 243: 
	case 244: 
	case 245: 
	case 246: 
	case 247: 
	case 248: 
	case 249: 
	case 250: 
	case 251: 
	case 252: 
	case 253: 
	case 254: 
	case 255: 
	case 256: 
	case 257: 
	case 258: 
	case 259: 
	case 260: 
	case 261: 
	case 262: 
	case 263: 
	case 264: 
	case 265: 
	case 266: 
	case 267: 
	case 268: 
	case 269: 
	case 270: 
	case 271: 
	case 272: 
	case 273: 
	case 274: 
	case 275: 
	case 276: 
	case 277: 
	case 278: 
	case 279: 
	case 280: 
	case 281: 
	case 282: 
	case 283: 
	case 284: 
	case 285: 
	case 286: 
	case 287: 
	case 288: 
	case 289: 
	case 290: 
	case 291: 
#line 111 "src/readstat_sav_parse.rl"
	{ str_len = p - str_start; }
#line 98 "src/readstat_sav_parse.rl"
	{
            memcpy(temp_val, str_start, str_len);
            temp_val[str_len] = '\0';
        }
#line 82 "src/readstat_sav_parse.rl"
	{
            varlookup_t *found = bsearch(temp_key, table, var_count, sizeof(varlookup_t), &compare_key_varlookup);
            if (found) {
                memcpy(ctx->varinfo[found->index].longname, temp_val, str_len);
                ctx->varinfo[found->index].longname[str_len] = '\0';
            } else if (ctx->error_handler) {
                snprintf(error_buf, sizeof(error_buf), "Failed to find %s\n", temp_key);
                ctx->error_handler(error_buf, ctx->user_ctx);
            }
        }
	break;
#line 3447 "src/readstat_sav_parse.c"
	}
	}

	_out: {}
	}

#line 119 "src/readstat_sav_parse.rl"


    if (cs < 227|| p != pe) {
        if (ctx->error_handler) {
            snprintf(error_buf, sizeof(error_buf), "Error parsing string \"%s\" around byte #%ld/%d, character %c\n", 
                    (char *)data, (long)(p - c_data), count, *p);
            ctx->error_handler(error_buf, ctx->user_ctx);
        }
        retval = READSTAT_ERROR_PARSE;
    }
    
    if (table)
        free(table);
    if (output_buffer)
        free(output_buffer);

    /* suppress warning */
    (void)sav_long_variable_parse_en_main;

    return retval;
}


#line 3478 "src/readstat_sav_parse.c"
static const int sav_very_long_string_parse_start = 1;

static const int sav_very_long_string_parse_en_main = 1;


#line 145 "src/readstat_sav_parse.rl"


readstat_error_t sav_parse_very_long_string_record(void *data, int count, sav_ctx_t *ctx) {
    unsigned char *c_data = (unsigned char *)data;
    int var_count = ctx->var_index;
    varlookup_t *table = malloc(var_count * sizeof(varlookup_t));
    readstat_error_t retval = READSTAT_OK;

    char temp_key[8*4+1];
    int temp_val;
    unsigned char *str_start = NULL;
    size_t str_len = 0;

    size_t error_buf_len = 1024 + count;
    char *error_buf = malloc(error_buf_len);
    unsigned char *p = NULL;
    unsigned char *pe = NULL;

    unsigned char *output_buffer = NULL;

    build_lookup_table(table, var_count, ctx);

    if (ctx->converter) {
        size_t input_len = count;
        size_t output_len = input_len * 4;

        pe = p = output_buffer = malloc(output_len);

        size_t status = iconv(ctx->converter, 
                (readstat_iconv_inbuf_t)&data, &input_len,
                (char **)&pe, &output_len);
        if (status == (size_t)-1) {
            free(output_buffer);
            return READSTAT_ERROR_PARSE;
        }
    } else {
        p = c_data;
        pe = c_data + count;
    }
    
    int cs;
    
    
#line 3528 "src/readstat_sav_parse.c"
	{
	cs = sav_very_long_string_parse_start;
	}

#line 3533 "src/readstat_sav_parse.c"
	{
	if ( p == pe )
		goto _test_eof;
	switch ( cs )
	{
case 1:
	if ( (*p) < 192u ) {
		if ( 64u <= (*p) && (*p) <= 90u )
			goto tr0;
	} else if ( (*p) > 223u ) {
		if ( (*p) > 239u ) {
			if ( 240u <= (*p) && (*p) <= 247u )
				goto tr4;
		} else if ( (*p) >= 224u )
			goto tr3;
	} else
		goto tr2;
	goto st0;
st0:
cs = 0;
	goto _out;
tr0:
#line 212 "src/readstat_sav_parse.rl"
	{ str_start = p; }
	goto st2;
st2:
	if ( ++p == pe )
		goto _test_eof2;
case 2:
#line 3563 "src/readstat_sav_parse.c"
	switch( (*p) ) {
		case 46u: goto st3;
		case 61u: goto tr6;
		case 95u: goto st3;
	}
	if ( (*p) < 64u ) {
		if ( (*p) > 36u ) {
			if ( 48u <= (*p) && (*p) <= 57u )
				goto st3;
		} else if ( (*p) >= 35u )
			goto st3;
	} else if ( (*p) > 90u ) {
		if ( (*p) < 224u ) {
			if ( 192u <= (*p) && (*p) <= 223u )
				goto st33;
		} else if ( (*p) > 239u ) {
			if ( 240u <= (*p) && (*p) <= 247u )
				goto st35;
		} else
			goto st34;
	} else
		goto st3;
	goto st0;
st3:
	if ( ++p == pe )
		goto _test_eof3;
case 3:
	switch( (*p) ) {
		case 46u: goto st4;
		case 61u: goto tr6;
		case 95u: goto st4;
	}
	if ( (*p) < 64u ) {
		if ( (*p) > 36u ) {
			if ( 48u <= (*p) && (*p) <= 57u )
				goto st4;
		} else if ( (*p) >= 35u )
			goto st4;
	} else if ( (*p) > 90u ) {
		if ( (*p) < 224u ) {
			if ( 192u <= (*p) && (*p) <= 223u )
				goto st30;
		} else if ( (*p) > 239u ) {
			if ( 240u <= (*p) && (*p) <= 247u )
				goto st32;
		} else
			goto st31;
	} else
		goto st4;
	goto st0;
st4:
	if ( ++p == pe )
		goto _test_eof4;
case 4:
	switch( (*p) ) {
		case 46u: goto st5;
		case 61u: goto tr6;
		case 95u: goto st5;
	}
	if ( (*p) < 64u ) {
		if ( (*p) > 36u ) {
			if ( 48u <= (*p) && (*p) <= 57u )
				goto st5;
		} else if ( (*p) >= 35u )
			goto st5;
	} else if ( (*p) > 90u ) {
		if ( (*p) < 224u ) {
			if ( 192u <= (*p) && (*p) <= 223u )
				goto st27;
		} else if ( (*p) > 239u ) {
			if ( 240u <= (*p) && (*p) <= 247u )
				goto st29;
		} else
			goto st28;
	} else
		goto st5;
	goto st0;
st5:
	if ( ++p == pe )
		goto _test_eof5;
case 5:
	switch( (*p) ) {
		case 46u: goto st6;
		case 61u: goto tr6;
		case 95u: goto st6;
	}
	if ( (*p) < 64u ) {
		if ( (*p) > 36u ) {
			if ( 48u <= (*p) && (*p) <= 57u )
				goto st6;
		} else if ( (*p) >= 35u )
			goto st6;
	} else if ( (*p) > 90u ) {
		if ( (*p) < 224u ) {
			if ( 192u <= (*p) && (*p) <= 223u )
				goto st24;
		} else if ( (*p) > 239u ) {
			if ( 240u <= (*p) && (*p) <= 247u )
				goto st26;
		} else
			goto st25;
	} else
		goto st6;
	goto st0;
st6:
	if ( ++p == pe )
		goto _test_eof6;
case 6:
	switch( (*p) ) {
		case 46u: goto st7;
		case 61u: goto tr6;
		case 95u: goto st7;
	}
	if ( (*p) < 64u ) {
		if ( (*p) > 36u ) {
			if ( 48u <= (*p) && (*p) <= 57u )
				goto st7;
		} else if ( (*p) >= 35u )
			goto st7;
	} else if ( (*p) > 90u ) {
		if ( (*p) < 224u ) {
			if ( 192u <= (*p) && (*p) <= 223u )
				goto st21;
		} else if ( (*p) > 239u ) {
			if ( 240u <= (*p) && (*p) <= 247u )
				goto st23;
		} else
			goto st22;
	} else
		goto st7;
	goto st0;
st7:
	if ( ++p == pe )
		goto _test_eof7;
case 7:
	switch( (*p) ) {
		case 46u: goto st8;
		case 61u: goto tr6;
		case 95u: goto st8;
	}
	if ( (*p) < 64u ) {
		if ( (*p) > 36u ) {
			if ( 48u <= (*p) && (*p) <= 57u )
				goto st8;
		} else if ( (*p) >= 35u )
			goto st8;
	} else if ( (*p) > 90u ) {
		if ( (*p) < 224u ) {
			if ( 192u <= (*p) && (*p) <= 223u )
				goto st18;
		} else if ( (*p) > 239u ) {
			if ( 240u <= (*p) && (*p) <= 247u )
				goto st20;
		} else
			goto st19;
	} else
		goto st8;
	goto st0;
st8:
	if ( ++p == pe )
		goto _test_eof8;
case 8:
	switch( (*p) ) {
		case 46u: goto st9;
		case 61u: goto tr6;
		case 95u: goto st9;
	}
	if ( (*p) < 64u ) {
		if ( (*p) > 36u ) {
			if ( 48u <= (*p) && (*p) <= 57u )
				goto st9;
		} else if ( (*p) >= 35u )
			goto st9;
	} else if ( (*p) > 90u ) {
		if ( (*p) < 224u ) {
			if ( 192u <= (*p) && (*p) <= 223u )
				goto st15;
		} else if ( (*p) > 239u ) {
			if ( 240u <= (*p) && (*p) <= 247u )
				goto st17;
		} else
			goto st16;
	} else
		goto st9;
	goto st0;
st9:
	if ( ++p == pe )
		goto _test_eof9;
case 9:
	if ( (*p) == 61u )
		goto tr6;
	goto st0;
tr6:
#line 212 "src/readstat_sav_parse.rl"
	{ str_len = p - str_start; }
#line 195 "src/readstat_sav_parse.rl"
	{
            memcpy(temp_key, str_start, str_len);
            temp_key[str_len] = '\0';
        }
	goto st10;
st10:
	if ( ++p == pe )
		goto _test_eof10;
case 10:
#line 3769 "src/readstat_sav_parse.c"
	if ( 48u <= (*p) && (*p) <= 57u )
		goto tr34;
	goto st0;
tr34:
#line 214 "src/readstat_sav_parse.rl"
	{ temp_val = 0; }
#line 200 "src/readstat_sav_parse.rl"
	{
            if ((*p) != '\0') { 
                temp_val = 10 * temp_val + ((*p) - '0'); 
            }
        }
	goto st11;
tr36:
#line 200 "src/readstat_sav_parse.rl"
	{
            if ((*p) != '\0') { 
                temp_val = 10 * temp_val + ((*p) - '0'); 
            }
        }
	goto st11;
st11:
	if ( ++p == pe )
		goto _test_eof11;
case 11:
#line 3795 "src/readstat_sav_parse.c"
	if ( (*p) == 0u )
		goto tr35;
	if ( 48u <= (*p) && (*p) <= 57u )
		goto tr36;
	goto st0;
tr35:
#line 188 "src/readstat_sav_parse.rl"
	{
            varlookup_t *found = bsearch(temp_key, table, var_count, sizeof(varlookup_t), &compare_key_varlookup);
            if (found) {
                ctx->varinfo[found->index].string_length = temp_val;
            }
        }
	goto st36;
st36:
	if ( ++p == pe )
		goto _test_eof36;
case 36:
#line 3814 "src/readstat_sav_parse.c"
	switch( (*p) ) {
		case 0u: goto st36;
		case 9u: goto st37;
	}
	goto st0;
st37:
	if ( ++p == pe )
		goto _test_eof37;
case 37:
	if ( (*p) < 192u ) {
		if ( 64u <= (*p) && (*p) <= 90u )
			goto tr0;
	} else if ( (*p) > 223u ) {
		if ( (*p) > 239u ) {
			if ( 240u <= (*p) && (*p) <= 247u )
				goto tr4;
		} else if ( (*p) >= 224u )
			goto tr3;
	} else
		goto tr2;
	goto st0;
tr2:
#line 212 "src/readstat_sav_parse.rl"
	{ str_start = p; }
	goto st12;
st12:
	if ( ++p == pe )
		goto _test_eof12;
case 12:
#line 3844 "src/readstat_sav_parse.c"
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st2;
	goto st0;
tr3:
#line 212 "src/readstat_sav_parse.rl"
	{ str_start = p; }
	goto st13;
st13:
	if ( ++p == pe )
		goto _test_eof13;
case 13:
#line 3856 "src/readstat_sav_parse.c"
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st12;
	goto st0;
tr4:
#line 212 "src/readstat_sav_parse.rl"
	{ str_start = p; }
	goto st14;
st14:
	if ( ++p == pe )
		goto _test_eof14;
case 14:
#line 3868 "src/readstat_sav_parse.c"
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st13;
	goto st0;
st15:
	if ( ++p == pe )
		goto _test_eof15;
case 15:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st9;
	goto st0;
st16:
	if ( ++p == pe )
		goto _test_eof16;
case 16:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st15;
	goto st0;
st17:
	if ( ++p == pe )
		goto _test_eof17;
case 17:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st16;
	goto st0;
st18:
	if ( ++p == pe )
		goto _test_eof18;
case 18:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st8;
	goto st0;
st19:
	if ( ++p == pe )
		goto _test_eof19;
case 19:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st18;
	goto st0;
st20:
	if ( ++p == pe )
		goto _test_eof20;
case 20:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st19;
	goto st0;
st21:
	if ( ++p == pe )
		goto _test_eof21;
case 21:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st7;
	goto st0;
st22:
	if ( ++p == pe )
		goto _test_eof22;
case 22:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st21;
	goto st0;
st23:
	if ( ++p == pe )
		goto _test_eof23;
case 23:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st22;
	goto st0;
st24:
	if ( ++p == pe )
		goto _test_eof24;
case 24:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st6;
	goto st0;
st25:
	if ( ++p == pe )
		goto _test_eof25;
case 25:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st24;
	goto st0;
st26:
	if ( ++p == pe )
		goto _test_eof26;
case 26:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st25;
	goto st0;
st27:
	if ( ++p == pe )
		goto _test_eof27;
case 27:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st5;
	goto st0;
st28:
	if ( ++p == pe )
		goto _test_eof28;
case 28:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st27;
	goto st0;
st29:
	if ( ++p == pe )
		goto _test_eof29;
case 29:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st28;
	goto st0;
st30:
	if ( ++p == pe )
		goto _test_eof30;
case 30:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st4;
	goto st0;
st31:
	if ( ++p == pe )
		goto _test_eof31;
case 31:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st30;
	goto st0;
st32:
	if ( ++p == pe )
		goto _test_eof32;
case 32:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st31;
	goto st0;
st33:
	if ( ++p == pe )
		goto _test_eof33;
case 33:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st3;
	goto st0;
st34:
	if ( ++p == pe )
		goto _test_eof34;
case 34:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st33;
	goto st0;
st35:
	if ( ++p == pe )
		goto _test_eof35;
case 35:
	if ( 128u <= (*p) && (*p) <= 191u )
		goto st34;
	goto st0;
	}
	_test_eof2: cs = 2; goto _test_eof; 
	_test_eof3: cs = 3; goto _test_eof; 
	_test_eof4: cs = 4; goto _test_eof; 
	_test_eof5: cs = 5; goto _test_eof; 
	_test_eof6: cs = 6; goto _test_eof; 
	_test_eof7: cs = 7; goto _test_eof; 
	_test_eof8: cs = 8; goto _test_eof; 
	_test_eof9: cs = 9; goto _test_eof; 
	_test_eof10: cs = 10; goto _test_eof; 
	_test_eof11: cs = 11; goto _test_eof; 
	_test_eof36: cs = 36; goto _test_eof; 
	_test_eof37: cs = 37; goto _test_eof; 
	_test_eof12: cs = 12; goto _test_eof; 
	_test_eof13: cs = 13; goto _test_eof; 
	_test_eof14: cs = 14; goto _test_eof; 
	_test_eof15: cs = 15; goto _test_eof; 
	_test_eof16: cs = 16; goto _test_eof; 
	_test_eof17: cs = 17; goto _test_eof; 
	_test_eof18: cs = 18; goto _test_eof; 
	_test_eof19: cs = 19; goto _test_eof; 
	_test_eof20: cs = 20; goto _test_eof; 
	_test_eof21: cs = 21; goto _test_eof; 
	_test_eof22: cs = 22; goto _test_eof; 
	_test_eof23: cs = 23; goto _test_eof; 
	_test_eof24: cs = 24; goto _test_eof; 
	_test_eof25: cs = 25; goto _test_eof; 
	_test_eof26: cs = 26; goto _test_eof; 
	_test_eof27: cs = 27; goto _test_eof; 
	_test_eof28: cs = 28; goto _test_eof; 
	_test_eof29: cs = 29; goto _test_eof; 
	_test_eof30: cs = 30; goto _test_eof; 
	_test_eof31: cs = 31; goto _test_eof; 
	_test_eof32: cs = 32; goto _test_eof; 
	_test_eof33: cs = 33; goto _test_eof; 
	_test_eof34: cs = 34; goto _test_eof; 
	_test_eof35: cs = 35; goto _test_eof; 

	_test_eof: {}
	_out: {}
	}

#line 222 "src/readstat_sav_parse.rl"

    
    if (cs < 36 || p != pe) {
        if (ctx->error_handler) {
            snprintf(error_buf, error_buf_len, "Parsed %ld of %ld bytes\nRemaining bytes: %s\n", (long)(p - c_data), (long)(pe - c_data), p);
            ctx->error_handler(error_buf, ctx->user_ctx);
        }
        retval = READSTAT_ERROR_PARSE;
    }
    
    if (table)
        free(table);
    if (output_buffer)
        free(output_buffer);
    if (error_buf)
        free(error_buf);

    /* suppress warning */
    (void)sav_very_long_string_parse_en_main;

    return retval;
}
