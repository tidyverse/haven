#include <sys/types.h>

#include "../readstat.h"
#include "readstat_por_parse.h"

%%{
    machine por_field_parse;
    write data nofinal noerror;
}%%

ssize_t readstat_por_parse_double(const char *data, size_t len, double *result, 
        readstat_error_handler error_cb, void *user_ctx) {
    ssize_t retval = 0;
    double val = 0.0;
    double denom = 30.0;
    double temp_frac = 0.0;
    double num = 0.0;
    double exp = 0.0;
    
    double temp_val = 0.0;
    
    const unsigned char *p = (const unsigned char *)data;
    const unsigned char *pe = p + len;
    
    int cs;
    int is_negative = 0, exp_is_negative = 0;
    int success = 0;
    
    %%{
        action incr_val {
            if (fc >= '0' && fc <= '9') {
                temp_val = 30 * temp_val + (fc - '0');
            } else if (fc >= 'A' && fc <= 'T') {
                temp_val = 30 * temp_val + (10 + fc - 'A');
            }
        }

        action incr_frac {
            if (fc >= '0' && fc <= '9') {
                temp_frac += (fc - '0') / denom;
            } else if (fc >= 'A' && fc <= 'T') {
                temp_frac += (10 + fc - 'A') / denom;
            }
            denom *= 30.0;
        }
        
        value = [0-9A-T]+ >{ temp_val = 0; } $incr_val;

        frac_value = [0-9A-T]+ >{ temp_frac = 0.0; } $incr_frac;

        fraction = "." frac_value;
        
        nonmissing_value = (("-" %{ is_negative = 1; })? value %{ num = temp_val; } fraction?
                            ( ("+" | "-" %{ exp_is_negative = 1; }) value %{ exp = temp_val; })?) "/";

        nonmissing_fraction = ("-" %{ is_negative = 1; })? fraction "/";
        
        missing_value = "*." >{ val = NAN; };
        
        main := " "* (missing_value | nonmissing_value | nonmissing_fraction ) @{ success = 1; fbreak; };
        
        write init;
        write exec;
    }%%

    if (!isnan(val)) {
        val = 1.0 * num + temp_frac;
        if (exp_is_negative)
            exp *= -1;
        if (exp) {
            val *= pow(30.0, exp);
        }
        if (is_negative)
            val *= -1;
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
