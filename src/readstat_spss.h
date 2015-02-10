
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

typedef struct spss_format_s {
    int          type;
    int          width;
    int          decimal_places;
} spss_format_t;

int spss_format(char *buffer, size_t len, spss_format_t *format);
