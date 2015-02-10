
#include <stdlib.h>
#include <stdio.h>

#include "readstat_spss.h"

typedef struct spss_type_s {
    int     type;
    char    string[100];
} spss_type_t;

static spss_type_t spss_type_strings[] = {
    { .type = SPSS_FORMAT_TYPE_A,      .string = "A" },
    { .type = SPSS_FORMAT_TYPE_AHEX,   .string = "AHEX" },
    { .type = SPSS_FORMAT_TYPE_COMMA,  .string = "COMMA" },
    { .type = SPSS_FORMAT_TYPE_DOLLAR, .string = "DOLLAR" },
    { .type = SPSS_FORMAT_TYPE_F,      .string = "F" },
    { .type = SPSS_FORMAT_TYPE_IB,     .string = "IB" },
    { .type = SPSS_FORMAT_TYPE_PIBHEX,  .string = "PIBHEX" },
    { .type = SPSS_FORMAT_TYPE_P,  .string = "P" },
    { .type = SPSS_FORMAT_TYPE_PIB,  .string = "PIB" },
    { .type = SPSS_FORMAT_TYPE_PK,  .string = "PK" },
    { .type = SPSS_FORMAT_TYPE_RB,  .string = "RB" },
    { .type = SPSS_FORMAT_TYPE_RBHEX,  .string = "RBHEX" },
    { .type = SPSS_FORMAT_TYPE_Z,  .string = "Z" },
    { .type = SPSS_FORMAT_TYPE_N,  .string = "N" },
    { .type = SPSS_FORMAT_TYPE_E,  .string = "E" },
    { .type = SPSS_FORMAT_TYPE_DATE,  .string = "DATE" },
    { .type = SPSS_FORMAT_TYPE_TIME,  .string = "TIME" },
    { .type = SPSS_FORMAT_TYPE_DATETIME,  .string = "DATETIME" },
    { .type = SPSS_FORMAT_TYPE_ADATE,  .string = "ADATE" },
    { .type = SPSS_FORMAT_TYPE_JDATE,  .string = "JDATE" },
    { .type = SPSS_FORMAT_TYPE_DTIME,  .string = "DTIME" },
    { .type = SPSS_FORMAT_TYPE_WKDAY,  .string = "WKDAY" },
    { .type = SPSS_FORMAT_TYPE_MONTH,  .string = "MONTH" },
    { .type = SPSS_FORMAT_TYPE_MOYR,  .string = "MOYR" },
    { .type = SPSS_FORMAT_TYPE_QYR,  .string = "QYR" },
    { .type = SPSS_FORMAT_TYPE_WKYR,  .string = "WKYR" },
    { .type = SPSS_FORMAT_TYPE_PCT,  .string = "PCT" },
    { .type = SPSS_FORMAT_TYPE_DOT,  .string = "DOT" },
    { .type = SPSS_FORMAT_TYPE_CCA,  .string = "CCA" },
    { .type = SPSS_FORMAT_TYPE_CCB,  .string = "CCB" },
    { .type = SPSS_FORMAT_TYPE_CCC,  .string = "CCC" },
    { .type = SPSS_FORMAT_TYPE_CCD,  .string = "CCD" },
    { .type = SPSS_FORMAT_TYPE_CCE,  .string = "CCE" },
    { .type = SPSS_FORMAT_TYPE_EDATE,  .string = "EDATE" },
    { .type = SPSS_FORMAT_TYPE_SDATE,  .string = "SDATE" }
};

int spss_format(char *buffer, size_t len, spss_format_t *format) {
    int i;
    for (i=0; i < sizeof(spss_type_strings)/sizeof(spss_type_strings[0]); i++) {
        if (format->type == spss_type_strings[i].type) {
            if (format->decimal_places || format->type == SPSS_FORMAT_TYPE_F) {
                snprintf(buffer, len, "%s%d.%d", spss_type_strings[i].string, format->width, format->decimal_places);
            } else if (format->width) {
                snprintf(buffer, len, "%s%d", spss_type_strings[i].string, format->width);
            } else {
                snprintf(buffer, len, "%s", spss_type_strings[i].string);
            }
            return 1;
        }
    }
    return 0;
}
