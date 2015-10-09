
#define SPSS_FORMAT_TYPE_A        1
#define SPSS_FORMAT_TYPE_AHEX     2
#define SPSS_FORMAT_TYPE_COMMA    3
#define SPSS_FORMAT_TYPE_DOLLAR   4
#define SPSS_FORMAT_TYPE_F        5
#define SPSS_FORMAT_TYPE_IB       6
#define SPSS_FORMAT_TYPE_PIBHEX   7
#define SPSS_FORMAT_TYPE_P        8
#define SPSS_FORMAT_TYPE_PIB      9
#define SPSS_FORMAT_TYPE_PK       10
#define SPSS_FORMAT_TYPE_RB       11
#define SPSS_FORMAT_TYPE_RBHEX    12
#define SPSS_FORMAT_TYPE_Z        15
#define SPSS_FORMAT_TYPE_N        16
#define SPSS_FORMAT_TYPE_E        17
#define SPSS_FORMAT_TYPE_DATE     20
#define SPSS_FORMAT_TYPE_TIME     21
#define SPSS_FORMAT_TYPE_DATETIME 22
#define SPSS_FORMAT_TYPE_ADATE    23
#define SPSS_FORMAT_TYPE_JDATE    24
#define SPSS_FORMAT_TYPE_DTIME    25
#define SPSS_FORMAT_TYPE_WKDAY    26
#define SPSS_FORMAT_TYPE_MONTH    27
#define SPSS_FORMAT_TYPE_MOYR     28
#define SPSS_FORMAT_TYPE_QYR      29
#define SPSS_FORMAT_TYPE_WKYR     30
#define SPSS_FORMAT_TYPE_PCT      31
#define SPSS_FORMAT_TYPE_DOT      32
#define SPSS_FORMAT_TYPE_CCA      33
#define SPSS_FORMAT_TYPE_CCB      34
#define SPSS_FORMAT_TYPE_CCC      35
#define SPSS_FORMAT_TYPE_CCD      36
#define SPSS_FORMAT_TYPE_CCE      37
#define SPSS_FORMAT_TYPE_EDATE    38
#define SPSS_FORMAT_TYPE_SDATE    39

#define spss_format_is_date(type) \
    (type == SPSS_FORMAT_TYPE_DATE || \
     type == SPSS_FORMAT_TYPE_DATETIME || \
     type == SPSS_FORMAT_TYPE_ADATE || \
     type == SPSS_FORMAT_TYPE_JDATE || \
     type == SPSS_FORMAT_TYPE_SDATE || \
     type == SPSS_FORMAT_TYPE_EDATE || \
     type == SPSS_FORMAT_TYPE_QYR || \
     type == SPSS_FORMAT_TYPE_MOYR || \
     type == SPSS_FORMAT_TYPE_WKYR)

#define SAV_HIGHEST_DOUBLE   0x7FEFFFFFFFFFFFFFUL
#define SAV_MISSING_DOUBLE   0xFFEFFFFFFFFFFFFFUL
#define SAV_LOWEST_DOUBLE    0xFFEFFFFFFFFFFFFEUL

typedef struct spss_format_s {
    int          type;
    int          width;
    int          decimal_places;
} spss_format_t;

typedef struct spss_varinfo_s {
    readstat_types_t type;
    int              labels_index;
    int              index;
    int              offset;
    int              width;
    int              string_length;
    spss_format_t    print_format;
    spss_format_t    write_format;
    int              n_segments;
    int              n_missing_values;
    int              missing_range;
    double           missing_values[3];
    char             name[8*4+1];
    char             longname[64*4+1];
    char            *label;
    readstat_missingness_t missingness;
} spss_varinfo_t;

int spss_format(char *buffer, size_t len, spss_format_t *format);
void spss_tag_missing_double(readstat_value_t *value, readstat_missingness_t *missingness);
int spss_varinfo_compare(const void *elem1, const void *elem2);

readstat_missingness_t spss_missingness_for_info(spss_varinfo_t *info);
readstat_variable_t *spss_init_variable_for_info(spss_varinfo_t *info);
void spss_free_variable(readstat_variable_t *);

uint64_t spss_64bit_value(readstat_value_t value);
