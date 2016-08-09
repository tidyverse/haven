
#include <time.h>
#include "../readstat.h"
#include "../readstat_iconv.h"

#include "readstat_dta.h"
#include "readstat_dta_parse_timestamp.h"

%%{
    machine dta_timestamp_parse;
    write data nofinal noerror;
}%%

readstat_error_t dta_parse_timestamp(const char *data, size_t len, struct tm *timestamp, dta_ctx_t *ctx) {
    readstat_error_t retval = READSTAT_OK;
    char error_buf[8192];
    const char *p = data;
    const char *pe = p + len;
    const char *eof = pe;
    int cs;
    int temp_val = 0;
    %%{
        action incr_val {
            temp_val = 10 * temp_val + (fc - '0');
        }

        integer = [0-9]+ >{ temp_val = 0; } $incr_val;

        day = integer %{ timestamp->tm_mday = temp_val; };

        month = 
            "Jan" %{ timestamp->tm_mon = 0; } |
            "Feb" %{ timestamp->tm_mon = 1; } |
            "Mar" %{ timestamp->tm_mon = 2; } |
            "Apr" %{ timestamp->tm_mon = 3; } |
            "May" %{ timestamp->tm_mon = 4; } |
            "Jun" %{ timestamp->tm_mon = 5; } |
            "Jul" %{ timestamp->tm_mon = 6; } |
            "Aug" %{ timestamp->tm_mon = 7; } |
            "Sep" %{ timestamp->tm_mon = 8; } |
            "Oct" %{ timestamp->tm_mon = 9; } |
            "Nov" %{ timestamp->tm_mon = 10; } |
            "Dec" %{ timestamp->tm_mon = 11; };

        year = integer %{ timestamp->tm_year = temp_val - 1900; };
        
        hour = integer %{ timestamp->tm_hour = temp_val; };

        minute = integer %{ timestamp->tm_min = temp_val; };

        main := " "? day " " month " " year " "+ hour ":" minute;

        write init;
        write exec;
    }%%

    if (cs < %%{ write first_final; }%%|| p != pe) {
        if (ctx->error_handler) {
            snprintf(error_buf, sizeof(error_buf), "Invalid timestamp string (length=%d): %*s", (int)len, (int)-len, data);
            ctx->error_handler(error_buf, ctx->user_ctx);
        }
        retval = READSTAT_ERROR_BAD_TIMESTAMP;
    }

    (void)dta_timestamp_parse_en_main;

    return retval;
}
