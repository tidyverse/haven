
#line 1 "src/spss/readstat_sav_parse.rl"
#include <limits.h>
#include <stdlib.h>
#include "../readstat.h"
#include "../readstat_iconv.h"
#include "../readstat_malloc.h"

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

static int count_vars(sav_ctx_t *ctx) {
    int i;
    spss_varinfo_t *last_info = NULL;
    int var_count = 0;
    for (i=0; i<ctx->var_index; i++) {
        spss_varinfo_t *info = ctx->varinfo[i];
        if (last_info == NULL || strcmp(info->name, last_info->name) != 0) {
            var_count++;
        }
        last_info = info;
    }
    return var_count;
}

static varlookup_t *build_lookup_table(int var_count, sav_ctx_t *ctx) {
    varlookup_t *table = readstat_malloc(var_count * sizeof(varlookup_t));
    int offset = 0;
    int i;
    spss_varinfo_t *last_info = NULL;
    for (i=0; i<ctx->var_index; i++) {
        spss_varinfo_t *info = ctx->varinfo[i];

        if (last_info == NULL || strcmp(info->name, last_info->name) != 0) {
            varlookup_t *entry = &table[offset++];

            memcpy(entry->name, info->name, sizeof(info->name));
            entry->index = info->index;
        }
        last_info = info;
    }
    qsort(table, var_count, sizeof(varlookup_t), &compare_varlookups);
    return table;
}


#line 65 "src/spss/readstat_sav_parse.c"
static const char _sav_long_variable_parse_actions[] = {
	0, 1, 3, 1, 5, 2, 4, 1, 
	3, 6, 2, 0
};

static const short _sav_long_variable_parse_key_offsets[] = {
	0, 0, 8, 23, 38, 53, 68, 83, 
	98, 113, 114, 122, 124, 126, 128, 130, 
	132, 134, 136, 138, 140, 142, 144, 146, 
	148, 150, 152, 154, 156, 158, 160, 162, 
	164, 166, 168, 170, 172, 174, 176, 178, 
	180, 182, 184, 186, 188, 190, 192, 194, 
	196, 198, 200, 202, 204, 206, 208, 210, 
	212, 214, 216, 218, 220, 222, 224, 226, 
	228, 230, 232, 234, 236, 238, 240, 242, 
	244, 246, 248, 250, 252, 254, 256, 258, 
	260, 262, 264, 266, 268, 270, 272, 274, 
	276, 278, 280, 282, 284, 286, 288, 290, 
	292, 294, 296, 298, 300, 302, 304, 306, 
	308, 310, 312, 314, 316, 318, 320, 322, 
	324, 326, 328, 330, 332, 334, 336, 338, 
	340, 342, 344, 346, 348, 350, 352, 354, 
	356, 358, 360, 362, 364, 366, 368, 370, 
	372, 374, 376, 378, 380, 382, 384, 386, 
	388, 390, 392, 394, 396, 398, 400, 402, 
	404, 406, 408, 410, 412, 414, 416, 418, 
	420, 422, 424, 426, 428, 430, 432, 434, 
	436, 438, 440, 442, 444, 446, 448, 450, 
	452, 454, 456, 458, 460, 462, 464, 466, 
	468, 470, 472, 474, 476, 478, 480, 482, 
	484, 486, 488, 490, 492, 494, 496, 498, 
	500, 502, 504, 506, 508, 510, 512, 514, 
	516, 518, 520, 522, 524, 526, 528, 530, 
	532, 534, 536, 538, 540, 542, 544, 546, 
	548, 550, 552, 554, 563, 571, 580, 589, 
	598, 607, 616, 625, 634, 643, 652, 661, 
	670, 679, 688, 697, 706, 715, 724, 733, 
	742, 751, 760, 769, 778, 787, 796, 805, 
	814, 823, 832, 841, 850, 859, 868, 877, 
	886, 895, 904, 913, 922, 931, 940, 949, 
	958, 967, 976, 985, 994, 1003, 1012, 1021, 
	1030, 1039, 1048, 1057, 1066, 1075, 1084, 1093, 
	1102, 1111, 1120, 1129
};

static const unsigned char _sav_long_variable_parse_trans_keys[] = {
	64u, 90u, 192u, 223u, 224u, 239u, 240u, 247u, 
	46u, 61u, 95u, 35u, 36u, 48u, 57u, 64u, 
	90u, 192u, 223u, 224u, 239u, 240u, 247u, 46u, 
	61u, 95u, 35u, 36u, 48u, 57u, 64u, 90u, 
	192u, 223u, 224u, 239u, 240u, 247u, 46u, 61u, 
	95u, 35u, 36u, 48u, 57u, 64u, 90u, 192u, 
	223u, 224u, 239u, 240u, 247u, 46u, 61u, 95u, 
	35u, 36u, 48u, 57u, 64u, 90u, 192u, 223u, 
	224u, 239u, 240u, 247u, 46u, 61u, 95u, 35u, 
	36u, 48u, 57u, 64u, 90u, 192u, 223u, 224u, 
	239u, 240u, 247u, 46u, 61u, 95u, 35u, 36u, 
	48u, 57u, 64u, 90u, 192u, 223u, 224u, 239u, 
	240u, 247u, 46u, 61u, 95u, 35u, 36u, 48u, 
	57u, 64u, 90u, 192u, 223u, 224u, 239u, 240u, 
	247u, 61u, 32u, 126u, 192u, 223u, 224u, 239u, 
	240u, 247u, 128u, 191u, 128u, 191u, 128u, 191u, 
	128u, 191u, 128u, 191u, 128u, 191u, 128u, 191u, 
	128u, 191u, 128u, 191u, 128u, 191u, 128u, 191u, 
	128u, 191u, 128u, 191u, 128u, 191u, 128u, 191u, 
	128u, 191u, 128u, 191u, 128u, 191u, 128u, 191u, 
	128u, 191u, 128u, 191u, 128u, 191u, 128u, 191u, 
	128u, 191u, 128u, 191u, 128u, 191u, 128u, 191u, 
	128u, 191u, 128u, 191u, 128u, 191u, 128u, 191u, 
	128u, 191u, 128u, 191u, 128u, 191u, 128u, 191u, 
	128u, 191u, 128u, 191u, 128u, 191u, 128u, 191u, 
	128u, 191u, 128u, 191u, 128u, 191u, 128u, 191u, 
	128u, 191u, 128u, 191u, 128u, 191u, 128u, 191u, 
	128u, 191u, 128u, 191u, 128u, 191u, 128u, 191u, 
	128u, 191u, 128u, 191u, 128u, 191u, 128u, 191u, 
	128u, 191u, 128u, 191u, 128u, 191u, 128u, 191u, 
	128u, 191u, 128u, 191u, 128u, 191u, 128u, 191u, 
	128u, 191u, 128u, 191u, 128u, 191u, 128u, 191u, 
	128u, 191u, 128u, 191u, 128u, 191u, 128u, 191u, 
	128u, 191u, 128u, 191u, 128u, 191u, 128u, 191u, 
	128u, 191u, 128u, 191u, 128u, 191u, 128u, 191u, 
	128u, 191u, 128u, 191u, 128u, 191u, 128u, 191u, 
	128u, 191u, 128u, 191u, 128u, 191u, 128u, 191u, 
	128u, 191u, 128u, 191u, 128u, 191u, 128u, 191u, 
	128u, 191u, 128u, 191u, 128u, 191u, 128u, 191u, 
	128u, 191u, 128u, 191u, 128u, 191u, 128u, 191u, 
	128u, 191u, 128u, 191u, 128u, 191u, 128u, 191u, 
	128u, 191u, 128u, 191u, 128u, 191u, 128u, 191u, 
	128u, 191u, 128u, 191u, 128u, 191u, 128u, 191u, 
	128u, 191u, 128u, 191u, 128u, 191u, 128u, 191u, 
	128u, 191u, 128u, 191u, 128u, 191u, 128u, 191u, 
	128u, 191u, 128u, 191u, 128u, 191u, 128u, 191u, 
	128u, 191u, 128u, 191u, 128u, 191u, 128u, 191u, 
	128u, 191u, 128u, 191u, 128u, 191u, 128u, 191u, 
	128u, 191u, 128u, 191u, 128u, 191u, 128u, 191u, 
	128u, 191u, 128u, 191u, 128u, 191u, 128u, 191u, 
	128u, 191u, 128u, 191u, 128u, 191u, 128u, 191u, 
	128u, 191u, 128u, 191u, 128u, 191u, 128u, 191u, 
	128u, 191u, 128u, 191u, 128u, 191u, 128u, 191u, 
	128u, 191u, 128u, 191u, 128u, 191u, 128u, 191u, 
	128u, 191u, 128u, 191u, 128u, 191u, 128u, 191u, 
	128u, 191u, 128u, 191u, 128u, 191u, 128u, 191u, 
	128u, 191u, 128u, 191u, 128u, 191u, 128u, 191u, 
	128u, 191u, 128u, 191u, 128u, 191u, 128u, 191u, 
	128u, 191u, 128u, 191u, 128u, 191u, 128u, 191u, 
	128u, 191u, 128u, 191u, 128u, 191u, 128u, 191u, 
	128u, 191u, 128u, 191u, 128u, 191u, 128u, 191u, 
	128u, 191u, 128u, 191u, 128u, 191u, 128u, 191u, 
	128u, 191u, 128u, 191u, 128u, 191u, 128u, 191u, 
	128u, 191u, 128u, 191u, 128u, 191u, 128u, 191u, 
	128u, 191u, 128u, 191u, 128u, 191u, 128u, 191u, 
	128u, 191u, 128u, 191u, 128u, 191u, 128u, 191u, 
	128u, 191u, 128u, 191u, 128u, 191u, 128u, 191u, 
	128u, 191u, 128u, 191u, 128u, 191u, 128u, 191u, 
	128u, 191u, 128u, 191u, 128u, 191u, 128u, 191u, 
	128u, 191u, 9u, 32u, 126u, 192u, 223u, 224u, 
	239u, 240u, 247u, 64u, 90u, 192u, 223u, 224u, 
	239u, 240u, 247u, 9u, 32u, 126u, 192u, 223u, 
	224u, 239u, 240u, 247u, 9u, 32u, 126u, 192u, 
	223u, 224u, 239u, 240u, 247u, 9u, 32u, 126u, 
	192u, 223u, 224u, 239u, 240u, 247u, 9u, 32u, 
	126u, 192u, 223u, 224u, 239u, 240u, 247u, 9u, 
	32u, 126u, 192u, 223u, 224u, 239u, 240u, 247u, 
	9u, 32u, 126u, 192u, 223u, 224u, 239u, 240u, 
	247u, 9u, 32u, 126u, 192u, 223u, 224u, 239u, 
	240u, 247u, 9u, 32u, 126u, 192u, 223u, 224u, 
	239u, 240u, 247u, 9u, 32u, 126u, 192u, 223u, 
	224u, 239u, 240u, 247u, 9u, 32u, 126u, 192u, 
	223u, 224u, 239u, 240u, 247u, 9u, 32u, 126u, 
	192u, 223u, 224u, 239u, 240u, 247u, 9u, 32u, 
	126u, 192u, 223u, 224u, 239u, 240u, 247u, 9u, 
	32u, 126u, 192u, 223u, 224u, 239u, 240u, 247u, 
	9u, 32u, 126u, 192u, 223u, 224u, 239u, 240u, 
	247u, 9u, 32u, 126u, 192u, 223u, 224u, 239u, 
	240u, 247u, 9u, 32u, 126u, 192u, 223u, 224u, 
	239u, 240u, 247u, 9u, 32u, 126u, 192u, 223u, 
	224u, 239u, 240u, 247u, 9u, 32u, 126u, 192u, 
	223u, 224u, 239u, 240u, 247u, 9u, 32u, 126u, 
	192u, 223u, 224u, 239u, 240u, 247u, 9u, 32u, 
	126u, 192u, 223u, 224u, 239u, 240u, 247u, 9u, 
	32u, 126u, 192u, 223u, 224u, 239u, 240u, 247u, 
	9u, 32u, 126u, 192u, 223u, 224u, 239u, 240u, 
	247u, 9u, 32u, 126u, 192u, 223u, 224u, 239u, 
	240u, 247u, 9u, 32u, 126u, 192u, 223u, 224u, 
	239u, 240u, 247u, 9u, 32u, 126u, 192u, 223u, 
	224u, 239u, 240u, 247u, 9u, 32u, 126u, 192u, 
	223u, 224u, 239u, 240u, 247u, 9u, 32u, 126u, 
	192u, 223u, 224u, 239u, 240u, 247u, 9u, 32u, 
	126u, 192u, 223u, 224u, 239u, 240u, 247u, 9u, 
	32u, 126u, 192u, 223u, 224u, 239u, 240u, 247u, 
	9u, 32u, 126u, 192u, 223u, 224u, 239u, 240u, 
	247u, 9u, 32u, 126u, 192u, 223u, 224u, 239u, 
	240u, 247u, 9u, 32u, 126u, 192u, 223u, 224u, 
	239u, 240u, 247u, 9u, 32u, 126u, 192u, 223u, 
	224u, 239u, 240u, 247u, 9u, 32u, 126u, 192u, 
	223u, 224u, 239u, 240u, 247u, 9u, 32u, 126u, 
	192u, 223u, 224u, 239u, 240u, 247u, 9u, 32u, 
	126u, 192u, 223u, 224u, 239u, 240u, 247u, 9u, 
	32u, 126u, 192u, 223u, 224u, 239u, 240u, 247u, 
	9u, 32u, 126u, 192u, 223u, 224u, 239u, 240u, 
	247u, 9u, 32u, 126u, 192u, 223u, 224u, 239u, 
	240u, 247u, 9u, 32u, 126u, 192u, 223u, 224u, 
	239u, 240u, 247u, 9u, 32u, 126u, 192u, 223u, 
	224u, 239u, 240u, 247u, 9u, 32u, 126u, 192u, 
	223u, 224u, 239u, 240u, 247u, 9u, 32u, 126u, 
	192u, 223u, 224u, 239u, 240u, 247u, 9u, 32u, 
	126u, 192u, 223u, 224u, 239u, 240u, 247u, 9u, 
	32u, 126u, 192u, 223u, 224u, 239u, 240u, 247u, 
	9u, 32u, 126u, 192u, 223u, 224u, 239u, 240u, 
	247u, 9u, 32u, 126u, 192u, 223u, 224u, 239u, 
	240u, 247u, 9u, 32u, 126u, 192u, 223u, 224u, 
	239u, 240u, 247u, 9u, 32u, 126u, 192u, 223u, 
	224u, 239u, 240u, 247u, 9u, 32u, 126u, 192u, 
	223u, 224u, 239u, 240u, 247u, 9u, 32u, 126u, 
	192u, 223u, 224u, 239u, 240u, 247u, 9u, 32u, 
	126u, 192u, 223u, 224u, 239u, 240u, 247u, 9u, 
	32u, 126u, 192u, 223u, 224u, 239u, 240u, 247u, 
	9u, 32u, 126u, 192u, 223u, 224u, 239u, 240u, 
	247u, 9u, 32u, 126u, 192u, 223u, 224u, 239u, 
	240u, 247u, 9u, 32u, 126u, 192u, 223u, 224u, 
	239u, 240u, 247u, 9u, 32u, 126u, 192u, 223u, 
	224u, 239u, 240u, 247u, 9u, 32u, 126u, 192u, 
	223u, 224u, 239u, 240u, 247u, 9u, 32u, 126u, 
	192u, 223u, 224u, 239u, 240u, 247u, 9u, 32u, 
	126u, 192u, 223u, 224u, 239u, 240u, 247u, 9u, 
	32u, 126u, 192u, 223u, 224u, 239u, 240u, 247u, 
	9u, 32u, 126u, 192u, 223u, 224u, 239u, 240u, 
	247u, 9u, 0
};

static const char _sav_long_variable_parse_single_lengths[] = {
	0, 0, 3, 3, 3, 3, 3, 3, 
	3, 1, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 1, 0, 1, 1, 1, 
	1, 1, 1, 1, 1, 1, 1, 1, 
	1, 1, 1, 1, 1, 1, 1, 1, 
	1, 1, 1, 1, 1, 1, 1, 1, 
	1, 1, 1, 1, 1, 1, 1, 1, 
	1, 1, 1, 1, 1, 1, 1, 1, 
	1, 1, 1, 1, 1, 1, 1, 1, 
	1, 1, 1, 1, 1, 1, 1, 1, 
	1, 1, 1, 1
};

static const char _sav_long_variable_parse_range_lengths[] = {
	0, 4, 6, 6, 6, 6, 6, 6, 
	6, 0, 4, 1, 1, 1, 1, 1, 
	1, 1, 1, 1, 1, 1, 1, 1, 
	1, 1, 1, 1, 1, 1, 1, 1, 
	1, 1, 1, 1, 1, 1, 1, 1, 
	1, 1, 1, 1, 1, 1, 1, 1, 
	1, 1, 1, 1, 1, 1, 1, 1, 
	1, 1, 1, 1, 1, 1, 1, 1, 
	1, 1, 1, 1, 1, 1, 1, 1, 
	1, 1, 1, 1, 1, 1, 1, 1, 
	1, 1, 1, 1, 1, 1, 1, 1, 
	1, 1, 1, 1, 1, 1, 1, 1, 
	1, 1, 1, 1, 1, 1, 1, 1, 
	1, 1, 1, 1, 1, 1, 1, 1, 
	1, 1, 1, 1, 1, 1, 1, 1, 
	1, 1, 1, 1, 1, 1, 1, 1, 
	1, 1, 1, 1, 1, 1, 1, 1, 
	1, 1, 1, 1, 1, 1, 1, 1, 
	1, 1, 1, 1, 1, 1, 1, 1, 
	1, 1, 1, 1, 1, 1, 1, 1, 
	1, 1, 1, 1, 1, 1, 1, 1, 
	1, 1, 1, 1, 1, 1, 1, 1, 
	1, 1, 1, 1, 1, 1, 1, 1, 
	1, 1, 1, 1, 1, 1, 1, 1, 
	1, 1, 1, 1, 1, 1, 1, 1, 
	1, 1, 1, 1, 1, 1, 1, 1, 
	1, 1, 1, 1, 1, 1, 1, 1, 
	1, 1, 1, 1, 1, 1, 1, 1, 
	1, 1, 1, 4, 4, 4, 4, 4, 
	4, 4, 4, 4, 4, 4, 4, 4, 
	4, 4, 4, 4, 4, 4, 4, 4, 
	4, 4, 4, 4, 4, 4, 4, 4, 
	4, 4, 4, 4, 4, 4, 4, 4, 
	4, 4, 4, 4, 4, 4, 4, 4, 
	4, 4, 4, 4, 4, 4, 4, 4, 
	4, 4, 4, 4, 4, 4, 4, 4, 
	4, 4, 4, 0
};

static const short _sav_long_variable_parse_index_offsets[] = {
	0, 0, 5, 15, 25, 35, 45, 55, 
	65, 75, 77, 82, 84, 86, 88, 90, 
	92, 94, 96, 98, 100, 102, 104, 106, 
	108, 110, 112, 114, 116, 118, 120, 122, 
	124, 126, 128, 130, 132, 134, 136, 138, 
	140, 142, 144, 146, 148, 150, 152, 154, 
	156, 158, 160, 162, 164, 166, 168, 170, 
	172, 174, 176, 178, 180, 182, 184, 186, 
	188, 190, 192, 194, 196, 198, 200, 202, 
	204, 206, 208, 210, 212, 214, 216, 218, 
	220, 222, 224, 226, 228, 230, 232, 234, 
	236, 238, 240, 242, 244, 246, 248, 250, 
	252, 254, 256, 258, 260, 262, 264, 266, 
	268, 270, 272, 274, 276, 278, 280, 282, 
	284, 286, 288, 290, 292, 294, 296, 298, 
	300, 302, 304, 306, 308, 310, 312, 314, 
	316, 318, 320, 322, 324, 326, 328, 330, 
	332, 334, 336, 338, 340, 342, 344, 346, 
	348, 350, 352, 354, 356, 358, 360, 362, 
	364, 366, 368, 370, 372, 374, 376, 378, 
	380, 382, 384, 386, 388, 390, 392, 394, 
	396, 398, 400, 402, 404, 406, 408, 410, 
	412, 414, 416, 418, 420, 422, 424, 426, 
	428, 430, 432, 434, 436, 438, 440, 442, 
	444, 446, 448, 450, 452, 454, 456, 458, 
	460, 462, 464, 466, 468, 470, 472, 474, 
	476, 478, 480, 482, 484, 486, 488, 490, 
	492, 494, 496, 498, 500, 502, 504, 506, 
	508, 510, 512, 514, 520, 525, 531, 537, 
	543, 549, 555, 561, 567, 573, 579, 585, 
	591, 597, 603, 609, 615, 621, 627, 633, 
	639, 645, 651, 657, 663, 669, 675, 681, 
	687, 693, 699, 705, 711, 717, 723, 729, 
	735, 741, 747, 753, 759, 765, 771, 777, 
	783, 789, 795, 801, 807, 813, 819, 825, 
	831, 837, 843, 849, 855, 861, 867, 873, 
	879, 885, 891, 897
};

static const short _sav_long_variable_parse_indicies[] = {
	0, 2, 3, 4, 1, 5, 6, 5, 
	5, 5, 5, 7, 8, 9, 1, 10, 
	6, 10, 10, 10, 10, 11, 12, 13, 
	1, 14, 6, 14, 14, 14, 14, 15, 
	16, 17, 1, 18, 6, 18, 18, 18, 
	18, 19, 20, 21, 1, 22, 6, 22, 
	22, 22, 22, 23, 24, 25, 1, 26, 
	6, 26, 26, 26, 26, 27, 28, 29, 
	1, 30, 6, 30, 30, 30, 30, 31, 
	32, 33, 1, 6, 1, 34, 35, 36, 
	37, 1, 38, 1, 39, 1, 40, 1, 
	41, 1, 42, 1, 43, 1, 44, 1, 
	45, 1, 46, 1, 47, 1, 48, 1, 
	49, 1, 50, 1, 51, 1, 52, 1, 
	53, 1, 54, 1, 55, 1, 56, 1, 
	57, 1, 58, 1, 59, 1, 60, 1, 
	61, 1, 62, 1, 63, 1, 64, 1, 
	65, 1, 66, 1, 67, 1, 68, 1, 
	69, 1, 70, 1, 71, 1, 72, 1, 
	73, 1, 74, 1, 75, 1, 76, 1, 
	77, 1, 78, 1, 79, 1, 80, 1, 
	81, 1, 82, 1, 83, 1, 84, 1, 
	85, 1, 86, 1, 87, 1, 88, 1, 
	89, 1, 90, 1, 91, 1, 92, 1, 
	93, 1, 94, 1, 95, 1, 96, 1, 
	97, 1, 98, 1, 99, 1, 100, 1, 
	101, 1, 102, 1, 103, 1, 104, 1, 
	105, 1, 106, 1, 107, 1, 108, 1, 
	109, 1, 110, 1, 111, 1, 112, 1, 
	113, 1, 114, 1, 115, 1, 116, 1, 
	117, 1, 118, 1, 119, 1, 120, 1, 
	121, 1, 122, 1, 123, 1, 124, 1, 
	125, 1, 126, 1, 127, 1, 128, 1, 
	129, 1, 130, 1, 131, 1, 132, 1, 
	133, 1, 134, 1, 135, 1, 136, 1, 
	137, 1, 138, 1, 139, 1, 140, 1, 
	141, 1, 142, 1, 143, 1, 144, 1, 
	145, 1, 146, 1, 147, 1, 148, 1, 
	149, 1, 150, 1, 151, 1, 152, 1, 
	153, 1, 154, 1, 155, 1, 156, 1, 
	157, 1, 158, 1, 159, 1, 160, 1, 
	161, 1, 162, 1, 163, 1, 164, 1, 
	165, 1, 166, 1, 167, 1, 168, 1, 
	169, 1, 170, 1, 171, 1, 172, 1, 
	173, 1, 174, 1, 175, 1, 176, 1, 
	177, 1, 178, 1, 179, 1, 180, 1, 
	181, 1, 182, 1, 183, 1, 184, 1, 
	185, 1, 186, 1, 187, 1, 188, 1, 
	189, 1, 190, 1, 191, 1, 192, 1, 
	193, 1, 194, 1, 195, 1, 196, 1, 
	197, 1, 198, 1, 199, 1, 200, 1, 
	201, 1, 202, 1, 203, 1, 204, 1, 
	205, 1, 206, 1, 207, 1, 208, 1, 
	209, 1, 210, 1, 211, 1, 212, 1, 
	213, 1, 214, 1, 215, 1, 216, 1, 
	217, 1, 218, 1, 219, 1, 220, 1, 
	221, 1, 222, 1, 223, 1, 224, 1, 
	225, 1, 226, 1, 227, 1, 228, 1, 
	229, 1, 230, 1, 231, 1, 232, 1, 
	30, 1, 31, 1, 32, 1, 26, 1, 
	27, 1, 28, 1, 22, 1, 23, 1, 
	24, 1, 18, 1, 19, 1, 20, 1, 
	14, 1, 15, 1, 16, 1, 10, 1, 
	11, 1, 12, 1, 5, 1, 7, 1, 
	8, 1, 233, 227, 228, 229, 234, 1, 
	0, 2, 3, 4, 1, 233, 224, 225, 
	226, 235, 1, 233, 221, 222, 223, 236, 
	1, 233, 218, 219, 220, 237, 1, 233, 
	215, 216, 217, 238, 1, 233, 212, 213, 
	214, 239, 1, 233, 209, 210, 211, 240, 
	1, 233, 206, 207, 208, 241, 1, 233, 
	203, 204, 205, 242, 1, 233, 200, 201, 
	202, 243, 1, 233, 197, 198, 199, 244, 
	1, 233, 194, 195, 196, 245, 1, 233, 
	191, 192, 193, 246, 1, 233, 188, 189, 
	190, 247, 1, 233, 185, 186, 187, 248, 
	1, 233, 182, 183, 184, 249, 1, 233, 
	179, 180, 181, 250, 1, 233, 176, 177, 
	178, 251, 1, 233, 173, 174, 175, 252, 
	1, 233, 170, 171, 172, 253, 1, 233, 
	167, 168, 169, 254, 1, 233, 164, 165, 
	166, 255, 1, 233, 161, 162, 163, 256, 
	1, 233, 158, 159, 160, 257, 1, 233, 
	155, 156, 157, 258, 1, 233, 152, 153, 
	154, 259, 1, 233, 149, 150, 151, 260, 
	1, 233, 146, 147, 148, 261, 1, 233, 
	143, 144, 145, 262, 1, 233, 140, 141, 
	142, 263, 1, 233, 137, 138, 139, 264, 
	1, 233, 134, 135, 136, 265, 1, 233, 
	131, 132, 133, 266, 1, 233, 128, 129, 
	130, 267, 1, 233, 125, 126, 127, 268, 
	1, 233, 122, 123, 124, 269, 1, 233, 
	119, 120, 121, 270, 1, 233, 116, 117, 
	118, 271, 1, 233, 113, 114, 115, 272, 
	1, 233, 110, 111, 112, 273, 1, 233, 
	107, 108, 109, 274, 1, 233, 104, 105, 
	106, 275, 1, 233, 101, 102, 103, 276, 
	1, 233, 98, 99, 100, 277, 1, 233, 
	95, 96, 97, 278, 1, 233, 92, 93, 
	94, 279, 1, 233, 89, 90, 91, 280, 
	1, 233, 86, 87, 88, 281, 1, 233, 
	83, 84, 85, 282, 1, 233, 80, 81, 
	82, 283, 1, 233, 77, 78, 79, 284, 
	1, 233, 74, 75, 76, 285, 1, 233, 
	71, 72, 73, 286, 1, 233, 68, 69, 
	70, 287, 1, 233, 65, 66, 67, 288, 
	1, 233, 62, 63, 64, 289, 1, 233, 
	59, 60, 61, 290, 1, 233, 56, 57, 
	58, 291, 1, 233, 53, 54, 55, 292, 
	1, 233, 50, 51, 52, 293, 1, 233, 
	47, 48, 49, 294, 1, 233, 44, 45, 
	46, 295, 1, 233, 41, 42, 43, 296, 
	1, 233, 1, 0
};

static const short _sav_long_variable_parse_trans_targs[] = {
	2, 0, 11, 12, 13, 3, 10, 224, 
	225, 226, 4, 221, 222, 223, 5, 218, 
	219, 220, 6, 215, 216, 217, 7, 212, 
	213, 214, 8, 209, 210, 211, 9, 206, 
	207, 208, 227, 203, 204, 205, 2, 11, 
	12, 291, 14, 15, 290, 17, 18, 289, 
	20, 21, 288, 23, 24, 287, 26, 27, 
	286, 29, 30, 285, 32, 33, 284, 35, 
	36, 283, 38, 39, 282, 41, 42, 281, 
	44, 45, 280, 47, 48, 279, 50, 51, 
	278, 53, 54, 277, 56, 57, 276, 59, 
	60, 275, 62, 63, 274, 65, 66, 273, 
	68, 69, 272, 71, 72, 271, 74, 75, 
	270, 77, 78, 269, 80, 81, 268, 83, 
	84, 267, 86, 87, 266, 89, 90, 265, 
	92, 93, 264, 95, 96, 263, 98, 99, 
	262, 101, 102, 261, 104, 105, 260, 107, 
	108, 259, 110, 111, 258, 113, 114, 257, 
	116, 117, 256, 119, 120, 255, 122, 123, 
	254, 125, 126, 253, 128, 129, 252, 131, 
	132, 251, 134, 135, 250, 137, 138, 249, 
	140, 141, 248, 143, 144, 247, 146, 147, 
	246, 149, 150, 245, 152, 153, 244, 155, 
	156, 243, 158, 159, 242, 161, 162, 241, 
	164, 165, 240, 167, 168, 239, 170, 171, 
	238, 173, 174, 237, 176, 177, 236, 179, 
	180, 235, 182, 183, 234, 185, 186, 233, 
	188, 189, 232, 191, 192, 231, 194, 195, 
	230, 197, 198, 229, 200, 201, 227, 203, 
	204, 228, 202, 199, 196, 193, 190, 187, 
	184, 181, 178, 175, 172, 169, 166, 163, 
	160, 157, 154, 151, 148, 145, 142, 139, 
	136, 133, 130, 127, 124, 121, 118, 115, 
	112, 109, 106, 103, 100, 97, 94, 91, 
	88, 85, 82, 79, 76, 73, 70, 67, 
	64, 61, 58, 55, 52, 49, 46, 43, 
	40, 37, 34, 31, 28, 25, 22, 19, 
	16
};

static const char _sav_long_variable_parse_trans_actions[] = {
	1, 0, 1, 1, 1, 0, 5, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 3, 3, 3, 3, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 8, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0
};

static const char _sav_long_variable_parse_eof_actions[] = {
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 8, 0, 8, 8, 8, 
	8, 8, 8, 8, 8, 8, 8, 8, 
	8, 8, 8, 8, 8, 8, 8, 8, 
	8, 8, 8, 8, 8, 8, 8, 8, 
	8, 8, 8, 8, 8, 8, 8, 8, 
	8, 8, 8, 8, 8, 8, 8, 8, 
	8, 8, 8, 8, 8, 8, 8, 8, 
	8, 8, 8, 8, 8, 8, 8, 8, 
	8, 8, 8, 8
};

static const int sav_long_variable_parse_start = 1;

static const int sav_long_variable_parse_en_main = 1;


#line 65 "src/spss/readstat_sav_parse.rl"


readstat_error_t sav_parse_long_variable_names_record(void *data, int count, sav_ctx_t *ctx) {
    unsigned char *c_data = (unsigned char *)data;
    int var_count = count_vars(ctx);
    readstat_error_t retval = READSTAT_OK;

    char temp_key[4*8+1];
    char temp_val[4*64+1];
    unsigned char *str_start = NULL;
    size_t str_len = 0;
    
    char error_buf[8192];
    unsigned char *p = NULL;
    unsigned char *pe = NULL;
    unsigned char *output_buffer = NULL;

    varlookup_t *table = build_lookup_table(var_count, ctx);

    if (ctx->converter) {
        size_t input_len = count;
        size_t output_len = input_len * 4;
        pe = p = output_buffer = readstat_malloc(output_len);
        size_t status = iconv(ctx->converter, 
                (readstat_iconv_inbuf_t)&data, &input_len,
                (char **)&pe, &output_len);
        if (status == (size_t)-1) {
            free(table);
            free(output_buffer);
            return READSTAT_ERROR_PARSE;
        }
    } else {
        p = c_data;
        pe = c_data + count;
    }
    unsigned char *eof = pe;

    int cs;

    
#line 660 "src/spss/readstat_sav_parse.c"
	{
	cs = sav_long_variable_parse_start;
	}

#line 665 "src/spss/readstat_sav_parse.c"
	{
	int _klen;
	unsigned int _trans;
	const char *_acts;
	unsigned int _nacts;
	const unsigned char *_keys;

	if ( p == pe )
		goto _test_eof;
	if ( cs == 0 )
		goto _out;
_resume:
	_keys = _sav_long_variable_parse_trans_keys + _sav_long_variable_parse_key_offsets[cs];
	_trans = _sav_long_variable_parse_index_offsets[cs];

	_klen = _sav_long_variable_parse_single_lengths[cs];
	if ( _klen > 0 ) {
		const unsigned char *_lower = _keys;
		const unsigned char *_mid;
		const unsigned char *_upper = _keys + _klen - 1;
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

	_klen = _sav_long_variable_parse_range_lengths[cs];
	if ( _klen > 0 ) {
		const unsigned char *_lower = _keys;
		const unsigned char *_mid;
		const unsigned char *_upper = _keys + (_klen<<1) - 2;
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
	_trans = _sav_long_variable_parse_indicies[_trans];
	cs = _sav_long_variable_parse_trans_targs[_trans];

	if ( _sav_long_variable_parse_trans_actions[_trans] == 0 )
		goto _again;

	_acts = _sav_long_variable_parse_actions + _sav_long_variable_parse_trans_actions[_trans];
	_nacts = (unsigned int) *_acts++;
	while ( _nacts-- > 0 )
	{
		switch ( *_acts++ )
		{
	case 0:
#line 105 "src/spss/readstat_sav_parse.rl"
	{
            varlookup_t *found = bsearch(temp_key, table, var_count, sizeof(varlookup_t), &compare_key_varlookup);
            if (found) {
                spss_varinfo_t *info = ctx->varinfo[found->index];
                memcpy(info->longname, temp_val, str_len);
                info->longname[str_len] = '\0';
            } else if (ctx->handle.error) {
                snprintf(error_buf, sizeof(error_buf), "Failed to find %s", temp_key);
                ctx->handle.error(error_buf, ctx->user_ctx);
            }
        }
	break;
	case 1:
#line 117 "src/spss/readstat_sav_parse.rl"
	{
            memcpy(temp_key, str_start, str_len);
            temp_key[str_len] = '\0';
        }
	break;
	case 2:
#line 122 "src/spss/readstat_sav_parse.rl"
	{
            memcpy(temp_val, str_start, str_len);
            temp_val[str_len] = '\0';
        }
	break;
	case 3:
#line 133 "src/spss/readstat_sav_parse.rl"
	{ str_start = p; }
	break;
	case 4:
#line 133 "src/spss/readstat_sav_parse.rl"
	{ str_len = p - str_start; }
	break;
	case 5:
#line 135 "src/spss/readstat_sav_parse.rl"
	{ str_start = p; }
	break;
	case 6:
#line 135 "src/spss/readstat_sav_parse.rl"
	{ str_len = p - str_start; }
	break;
#line 783 "src/spss/readstat_sav_parse.c"
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
	const char *__acts = _sav_long_variable_parse_actions + _sav_long_variable_parse_eof_actions[cs];
	unsigned int __nacts = (unsigned int) *__acts++;
	while ( __nacts-- > 0 ) {
		switch ( *__acts++ ) {
	case 0:
#line 105 "src/spss/readstat_sav_parse.rl"
	{
            varlookup_t *found = bsearch(temp_key, table, var_count, sizeof(varlookup_t), &compare_key_varlookup);
            if (found) {
                spss_varinfo_t *info = ctx->varinfo[found->index];
                memcpy(info->longname, temp_val, str_len);
                info->longname[str_len] = '\0';
            } else if (ctx->handle.error) {
                snprintf(error_buf, sizeof(error_buf), "Failed to find %s", temp_key);
                ctx->handle.error(error_buf, ctx->user_ctx);
            }
        }
	break;
	case 2:
#line 122 "src/spss/readstat_sav_parse.rl"
	{
            memcpy(temp_val, str_start, str_len);
            temp_val[str_len] = '\0';
        }
	break;
	case 6:
#line 135 "src/spss/readstat_sav_parse.rl"
	{ str_len = p - str_start; }
	break;
#line 824 "src/spss/readstat_sav_parse.c"
		}
	}
	}

	_out: {}
	}

#line 143 "src/spss/readstat_sav_parse.rl"


    if (cs < 227|| p != pe) {
        if (ctx->handle.error) {
            snprintf(error_buf, sizeof(error_buf), "Error parsing string \"%.*s\" around byte #%ld/%d, character %c", 
                    count, (char *)data, (long)(p - c_data), count, *p);
            ctx->handle.error(error_buf, ctx->user_ctx);
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


#line 856 "src/spss/readstat_sav_parse.c"
static const char _sav_very_long_string_parse_actions[] = {
	0, 1, 0, 1, 2, 1, 3, 2, 
	4, 1, 2, 5, 2
};

static const unsigned char _sav_very_long_string_parse_key_offsets[] = {
	0, 0, 8, 23, 38, 53, 68, 83, 
	98, 113, 114, 116, 119, 121, 123, 125, 
	127, 129, 131, 133, 135, 137, 139, 141, 
	143, 145, 147, 149, 151, 153, 155, 157, 
	159, 161, 163, 165, 167, 169
};

static const unsigned char _sav_very_long_string_parse_trans_keys[] = {
	64u, 90u, 192u, 223u, 224u, 239u, 240u, 247u, 
	46u, 61u, 95u, 35u, 36u, 48u, 57u, 64u, 
	90u, 192u, 223u, 224u, 239u, 240u, 247u, 46u, 
	61u, 95u, 35u, 36u, 48u, 57u, 64u, 90u, 
	192u, 223u, 224u, 239u, 240u, 247u, 46u, 61u, 
	95u, 35u, 36u, 48u, 57u, 64u, 90u, 192u, 
	223u, 224u, 239u, 240u, 247u, 46u, 61u, 95u, 
	35u, 36u, 48u, 57u, 64u, 90u, 192u, 223u, 
	224u, 239u, 240u, 247u, 46u, 61u, 95u, 35u, 
	36u, 48u, 57u, 64u, 90u, 192u, 223u, 224u, 
	239u, 240u, 247u, 46u, 61u, 95u, 35u, 36u, 
	48u, 57u, 64u, 90u, 192u, 223u, 224u, 239u, 
	240u, 247u, 46u, 61u, 95u, 35u, 36u, 48u, 
	57u, 64u, 90u, 192u, 223u, 224u, 239u, 240u, 
	247u, 61u, 48u, 57u, 0u, 48u, 57u, 128u, 
	191u, 128u, 191u, 128u, 191u, 128u, 191u, 128u, 
	191u, 128u, 191u, 128u, 191u, 128u, 191u, 128u, 
	191u, 128u, 191u, 128u, 191u, 128u, 191u, 128u, 
	191u, 128u, 191u, 128u, 191u, 128u, 191u, 128u, 
	191u, 128u, 191u, 128u, 191u, 128u, 191u, 128u, 
	191u, 128u, 191u, 128u, 191u, 128u, 191u, 0u, 
	9u, 64u, 90u, 192u, 223u, 224u, 239u, 240u, 
	247u, 0
};

static const char _sav_very_long_string_parse_single_lengths[] = {
	0, 0, 3, 3, 3, 3, 3, 3, 
	3, 1, 0, 1, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 2, 0
};

static const char _sav_very_long_string_parse_range_lengths[] = {
	0, 4, 6, 6, 6, 6, 6, 6, 
	6, 0, 1, 1, 1, 1, 1, 1, 
	1, 1, 1, 1, 1, 1, 1, 1, 
	1, 1, 1, 1, 1, 1, 1, 1, 
	1, 1, 1, 1, 0, 4
};

static const unsigned char _sav_very_long_string_parse_index_offsets[] = {
	0, 0, 5, 15, 25, 35, 45, 55, 
	65, 75, 77, 79, 82, 84, 86, 88, 
	90, 92, 94, 96, 98, 100, 102, 104, 
	106, 108, 110, 112, 114, 116, 118, 120, 
	122, 124, 126, 128, 130, 133
};

static const char _sav_very_long_string_parse_indicies[] = {
	0, 2, 3, 4, 1, 5, 6, 5, 
	5, 5, 5, 7, 8, 9, 1, 10, 
	6, 10, 10, 10, 10, 11, 12, 13, 
	1, 14, 6, 14, 14, 14, 14, 15, 
	16, 17, 1, 18, 6, 18, 18, 18, 
	18, 19, 20, 21, 1, 22, 6, 22, 
	22, 22, 22, 23, 24, 25, 1, 26, 
	6, 26, 26, 26, 26, 27, 28, 29, 
	1, 30, 6, 30, 30, 30, 30, 31, 
	32, 33, 1, 6, 1, 34, 1, 35, 
	36, 1, 37, 1, 38, 1, 39, 1, 
	30, 1, 31, 1, 32, 1, 26, 1, 
	27, 1, 28, 1, 22, 1, 23, 1, 
	24, 1, 18, 1, 19, 1, 20, 1, 
	14, 1, 15, 1, 16, 1, 10, 1, 
	11, 1, 12, 1, 5, 1, 7, 1, 
	8, 1, 40, 41, 1, 0, 2, 3, 
	4, 1, 0
};

static const char _sav_very_long_string_parse_trans_targs[] = {
	2, 0, 12, 13, 14, 3, 10, 33, 
	34, 35, 4, 30, 31, 32, 5, 27, 
	28, 29, 6, 24, 25, 26, 7, 21, 
	22, 23, 8, 18, 19, 20, 9, 15, 
	16, 17, 11, 36, 11, 2, 12, 13, 
	36, 37
};

static const char _sav_very_long_string_parse_trans_actions[] = {
	5, 0, 5, 5, 5, 0, 7, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 10, 1, 3, 0, 0, 0, 
	0, 0
};

static const int sav_very_long_string_parse_start = 1;

static const int sav_very_long_string_parse_en_main = 1;


#line 169 "src/spss/readstat_sav_parse.rl"


readstat_error_t sav_parse_very_long_string_record(void *data, int count, sav_ctx_t *ctx) {
    unsigned char *c_data = (unsigned char *)data;
    int var_count = count_vars(ctx);
    readstat_error_t retval = READSTAT_OK;

    char temp_key[8*4+1];
    unsigned int temp_val = 0;
    unsigned char *str_start = NULL;
    size_t str_len = 0;

    size_t error_buf_len = 1024 + count;
    char *error_buf = NULL;
    unsigned char *p = NULL;
    unsigned char *pe = NULL;

    unsigned char *output_buffer = NULL;
    varlookup_t *table = NULL;
    int cs;

    if (ctx->converter) {
        size_t input_len = count;
        size_t output_len = input_len * 4;

        pe = p = output_buffer = readstat_malloc(output_len);

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

    error_buf = readstat_malloc(error_buf_len);
    table = build_lookup_table(var_count, ctx);
    
    
#line 1008 "src/spss/readstat_sav_parse.c"
	{
	cs = sav_very_long_string_parse_start;
	}

#line 1013 "src/spss/readstat_sav_parse.c"
	{
	int _klen;
	unsigned int _trans;
	const char *_acts;
	unsigned int _nacts;
	const unsigned char *_keys;

	if ( p == pe )
		goto _test_eof;
	if ( cs == 0 )
		goto _out;
_resume:
	_keys = _sav_very_long_string_parse_trans_keys + _sav_very_long_string_parse_key_offsets[cs];
	_trans = _sav_very_long_string_parse_index_offsets[cs];

	_klen = _sav_very_long_string_parse_single_lengths[cs];
	if ( _klen > 0 ) {
		const unsigned char *_lower = _keys;
		const unsigned char *_mid;
		const unsigned char *_upper = _keys + _klen - 1;
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

	_klen = _sav_very_long_string_parse_range_lengths[cs];
	if ( _klen > 0 ) {
		const unsigned char *_lower = _keys;
		const unsigned char *_mid;
		const unsigned char *_upper = _keys + (_klen<<1) - 2;
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
	_trans = _sav_very_long_string_parse_indicies[_trans];
	cs = _sav_very_long_string_parse_trans_targs[_trans];

	if ( _sav_very_long_string_parse_trans_actions[_trans] == 0 )
		goto _again;

	_acts = _sav_very_long_string_parse_actions + _sav_very_long_string_parse_trans_actions[_trans];
	_nacts = (unsigned int) *_acts++;
	while ( _nacts-- > 0 )
	{
		switch ( *_acts++ )
		{
	case 0:
#line 212 "src/spss/readstat_sav_parse.rl"
	{
            varlookup_t *found = bsearch(temp_key, table, var_count, sizeof(varlookup_t), &compare_key_varlookup);
            if (found) {
                ctx->varinfo[found->index]->string_length = temp_val;
            }
        }
	break;
	case 1:
#line 219 "src/spss/readstat_sav_parse.rl"
	{
            memcpy(temp_key, str_start, str_len);
            temp_key[str_len] = '\0';
        }
	break;
	case 2:
#line 224 "src/spss/readstat_sav_parse.rl"
	{
            if ((*p) != '\0') {
                unsigned char digit = (*p) - '0';
                if (temp_val <= (UINT_MAX - digit) / 10) {
                    temp_val = 10 * temp_val + digit;
                } else {
                    {p++; goto _out; }
                }
            }
        }
	break;
	case 3:
#line 241 "src/spss/readstat_sav_parse.rl"
	{ str_start = p; }
	break;
	case 4:
#line 241 "src/spss/readstat_sav_parse.rl"
	{ str_len = p - str_start; }
	break;
	case 5:
#line 243 "src/spss/readstat_sav_parse.rl"
	{ temp_val = 0; }
	break;
#line 1128 "src/spss/readstat_sav_parse.c"
		}
	}

_again:
	if ( cs == 0 )
		goto _out;
	if ( ++p != pe )
		goto _resume;
	_test_eof: {}
	_out: {}
	}

#line 251 "src/spss/readstat_sav_parse.rl"

    
    if (cs < 36 || p != pe) {
        if (ctx->handle.error) {
            snprintf(error_buf, error_buf_len, "Parsed %ld of %ld bytes. Remaining bytes: %.*s",
                    (long)(p - c_data), (long)(pe - c_data), (int)(pe - p), p);
            ctx->handle.error(error_buf, ctx->user_ctx);
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
