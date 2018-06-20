
#include <stdlib.h>
#include "../readstat.h"

#include "readstat_spss.h"
#include "readstat_spss_parse.h"

%%{
    machine spss_format_parser;
    write data nofinal noerror;
}%%

readstat_error_t spss_parse_format(const char *data, int count, spss_format_t *fmt) {
    unsigned char *p = (unsigned char *)data;
    unsigned char *pe = (unsigned char *)data + count;
    unsigned char *eof = pe;

    int cs;
    unsigned int integer = 0;

    %%{
        action start_integer {
            integer = 0;
        }

        action incr_integer {
            integer = 10 * integer + (fc - '0');
        }

        action save_width {
            fmt->width = integer;
        }

        action save_precision {
            fmt->decimal_places = integer;
        }

        type = ("A"i %{ fmt->type = SPSS_FORMAT_TYPE_A; } |
                "AHEX"i %{ fmt->type = SPSS_FORMAT_TYPE_AHEX; } |
                "COMMA"i %{ fmt->type = SPSS_FORMAT_TYPE_COMMA; } |
                "DOLLAR"i %{ fmt->type = SPSS_FORMAT_TYPE_DOLLAR; } |
                "F"i %{ fmt->type = SPSS_FORMAT_TYPE_F; } |
                "IB"i %{ fmt->type = SPSS_FORMAT_TYPE_IB; } |
                "PIBHEX"i %{ fmt->type = SPSS_FORMAT_TYPE_PIBHEX; } |
                "P"i %{ fmt->type = SPSS_FORMAT_TYPE_P; } |
                "PIB"i %{ fmt->type = SPSS_FORMAT_TYPE_PIB; } |
                "PK"i %{ fmt->type = SPSS_FORMAT_TYPE_PK; } |
                "RB"i %{ fmt->type = SPSS_FORMAT_TYPE_RB; } |
                "RBHEX"i %{ fmt->type = SPSS_FORMAT_TYPE_RBHEX; } |
                "Z"i %{ fmt->type = SPSS_FORMAT_TYPE_Z; } |
                "N"i %{ fmt->type = SPSS_FORMAT_TYPE_N; } |
                "E"i %{ fmt->type = SPSS_FORMAT_TYPE_E; } |
                "DATE"i %{ fmt->type = SPSS_FORMAT_TYPE_DATE; } |
                "TIME"i %{ fmt->type = SPSS_FORMAT_TYPE_TIME; } |
                "DATETIME"i %{ fmt->type = SPSS_FORMAT_TYPE_DATETIME; } |
                "YMDHMS"i %{ fmt->type = SPSS_FORMAT_TYPE_YMDHMS; } |
                "ADATE"i %{ fmt->type = SPSS_FORMAT_TYPE_ADATE; } |
                "JDATE"i %{ fmt->type = SPSS_FORMAT_TYPE_JDATE; } |
                "DTIME"i %{ fmt->type = SPSS_FORMAT_TYPE_DTIME; } |
                "MTIME"i %{ fmt->type = SPSS_FORMAT_TYPE_MTIME; } |
                "WKDAY"i %{ fmt->type = SPSS_FORMAT_TYPE_WKDAY; } |
                "MONTH"i %{ fmt->type = SPSS_FORMAT_TYPE_MONTH; } |
                "MOYR"i %{ fmt->type = SPSS_FORMAT_TYPE_MOYR; } |
                "QYR"i %{ fmt->type = SPSS_FORMAT_TYPE_QYR; } |
                "WKYR"i %{ fmt->type = SPSS_FORMAT_TYPE_WKYR; } |
                "PCT"i %{ fmt->type = SPSS_FORMAT_TYPE_PCT; } |
                "DOT"i %{ fmt->type = SPSS_FORMAT_TYPE_DOT; } |
                "CCA"i %{ fmt->type = SPSS_FORMAT_TYPE_CCA; } |
                "CCB"i %{ fmt->type = SPSS_FORMAT_TYPE_CCB; } |
                "CCC"i %{ fmt->type = SPSS_FORMAT_TYPE_CCC; } |
                "CCD"i %{ fmt->type = SPSS_FORMAT_TYPE_CCD; } |
                "CCE"i %{ fmt->type = SPSS_FORMAT_TYPE_CCE; } |
                "EDATE"i %{ fmt->type = SPSS_FORMAT_TYPE_EDATE; } |
                "SDATE"i %{ fmt->type = SPSS_FORMAT_TYPE_SDATE; }
                );

        integer = [0-9]+ >start_integer $incr_integer;

        width = integer %save_width;

        precision = integer %save_precision;

        main := type (width ("." precision)? )?;

        write init;
        write exec;
    }%%

    /* suppress warning */
    (void)spss_format_parser_en_main;

    if (cs < %%{ write first_final; }%% || p != eof) {
        return READSTAT_ERROR_PARSE;
    }

    return READSTAT_OK;
}
