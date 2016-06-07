#include <iconv.h>

#ifdef WINICONV_CONST
typedef const char ** readstat_iconv_inbuf_t;
#else
typedef char ** readstat_iconv_inbuf_t;
#endif

typedef struct readstat_charset_entry_s {
    int     code;
    char    name[32];
} readstat_charset_entry_t;
