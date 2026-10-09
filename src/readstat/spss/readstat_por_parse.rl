#include <sys/types.h>

#include "../readstat.h"
#include "readstat_por_parse.h"

#define POR_PARSE_MAX_DIGITS 128

%%{
    machine por_field_parse;
    write data nofinal noerror;
}%%

static int por_base30_digit_value(unsigned char c) {
    if (c >= '0' && c <= '9')
        return c - '0';
    return 10 + c - 'A';
}

ssize_t readstat_por_parse_double(const char *data, size_t len, double *result,
        readstat_error_handler error_cb, void *user_ctx) {
    ssize_t retval = 0;
    double val = 0.0;

    /* Significant digits of the mantissa, most significant first; leading
     * zeros are dropped and digits beyond POR_PARSE_MAX_DIGITS are folded
     * into the scale. */
    unsigned char digits[POR_PARSE_MAX_DIGITS] = { 0 };
    size_t n_digits = 0;
    long scale = 0;
    long exp = 0;

    const unsigned char *p = (const unsigned char *)data;
    const unsigned char *pe = p + len;

    int cs;
    int is_negative = 0, exp_is_negative = 0;
    int got_dot = 0;
    int is_missing = 0;
    int success = 0;

    %%{
        action mantissa_digit {
            int digit = por_base30_digit_value(fc);
            if (n_digits == 0 && digit == 0) {
                if (got_dot)
                    scale--;
            } else if (n_digits < POR_PARSE_MAX_DIGITS) {
                digits[n_digits++] = digit;
                if (got_dot)
                    scale--;
            } else if (!got_dot) {
                scale++;
            }
        }

        action exponent_digit {
            if (exp < 100000)
                exp = 30 * exp + por_base30_digit_value(fc);
        }

        trig = [0-9A-T];

        mantissa = (trig+ $mantissa_digit ("." @{ got_dot = 1; } (trig+ $mantissa_digit)?)?)
                 | ("." @{ got_dot = 1; } trig+ $mantissa_digit);

        exponent = ("+" | "-" @{ exp_is_negative = 1; }) trig+ $exponent_digit;

        nonmissing_value = ("-" @{ is_negative = 1; })? mantissa exponent? "/";

        missing_value = "*." @{ is_missing = 1; };

        main := " "* (missing_value | nonmissing_value) @{ success = 1; fbreak; };

        write init;
        write exec;
    }%%

    if (is_missing) {
        val = NAN;
    } else {
        if (exp_is_negative)
            exp = -exp;
        val = por_base30_to_double(digits, n_digits, scale + exp);
        if (is_negative)
            val = -val;
    }

    if (!success) {
        retval = -1;
        if (error_cb) {
            char error_buf[1024];
            snprintf(error_buf, sizeof(error_buf), "Read bytes: %ld   String: %.*s  Ending state: %d",
                    (long)(p - (const unsigned char *)data), (int)len, data, cs);
            error_cb(error_buf, user_ctx);
        }
    }

    if (retval == 0) {
        if (result)
            *result = val;

        retval = (p - (const unsigned char *)data);
    }

    /* suppress warning */
    (void)por_field_parse_en_main;

    return retval;
}
