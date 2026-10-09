
readstat_error_t readstat_convert(char *dst, size_t dst_len, const char *src, size_t src_len, iconv_t converter);
/* Same, but keeps trailing spaces (Stata treats them as significant) */
readstat_error_t readstat_convert_notrim(char *dst, size_t dst_len, const char *src, size_t src_len, iconv_t converter);
