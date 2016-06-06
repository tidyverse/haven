
#include <errno.h>
#include "readstat.h"
#include "readstat_iconv.h"
#include "readstat_convert.h"

static void unpad(char *string, size_t len) {
    string[len] = '\0';
    /* remove space padding */
    ssize_t i;
    for (i=len-1; i>=0; i--) {
        if (string[i] == ' ') {
            string[i] = '\0';
        } else {
            break;
        }
    }
}

readstat_error_t readstat_convert(char *dst, size_t dst_len, const char *src, size_t src_len, iconv_t converter) {
    if (converter) {
        size_t dst_left = dst_len;
        char *dst_end = dst;
        size_t status = iconv(converter, (readstat_iconv_inbuf_t)&src, &src_len, &dst_end, &dst_left);
        if (status == (size_t)-1) {
            if (errno == E2BIG) {
                return READSTAT_ERROR_CONVERT_LONG_STRING;
            } else if (errno == EILSEQ) {
                return READSTAT_ERROR_CONVERT_BAD_STRING;
            } else if (errno == EINVAL) {
                return READSTAT_ERROR_CONVERT_SHORT_STRING;
            }
            return READSTAT_ERROR_CONVERT;
        }
        unpad(dst, dst_len - dst_left);
    } else {
        memcpy(dst, src, src_len);
        unpad(dst, src_len);
    }
    return READSTAT_OK;
}
