#include <iconv.h>

/* ICONV_CONST defined by autotools; so we hack this in manually */
#if defined(_WIN32) || defined(__sun)
  #define ICONV_CONST const
#else
  #define ICONV_CONST
#endif

typedef ICONV_CONST char ** readstat_iconv_inbuf_t;

typedef struct readstat_charset_entry_s {
    int     code;
    char    name[32];
} readstat_charset_entry_t;
