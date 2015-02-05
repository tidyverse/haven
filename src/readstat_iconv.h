#include <iconv.h>

#ifdef WIN32
typedef const char ** readstat_iconv_inbuf_t;
#else
typedef char ** readstat_iconv_inbuf_t;
#endif
