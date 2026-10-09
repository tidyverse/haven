//
//  readstat_por_parse.h
//

ssize_t readstat_por_parse_double(const char *data, size_t len, double *result,
        readstat_error_handler error_cb, void *user_ctx);

/* Correctly rounded conversion of a base-30 digit string (values 0-29, most
 * significant first) times 30^exponent to the nearest double. Defined in
 * readstat_por.c. */
double por_base30_to_double(const unsigned char *digits, size_t n_digits, long exponent);
