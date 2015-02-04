
#include "readstat_spss.h"
#include <stdlib.h>

const char *spss_format(int type) {
    const char *result = NULL;
    switch (type) {
        case SPSS_FORMAT_TYPE_DATE:
            result = "dd-Mon-yyyy";
            break;
        case SPSS_FORMAT_TYPE_ADATE:
            result = "NN/dd/yyyy";
            break;
        case SPSS_FORMAT_TYPE_EDATE:
            result = "dd.NN.yyyy";
            break;
        case SPSS_FORMAT_TYPE_JDATE:
            result = "yyyyddd";
            break;
        case SPSS_FORMAT_TYPE_SDATE:
            result = "yyyy/NN/dd";
            break;
        case SPSS_FORMAT_TYPE_QYR:
            result = "q Q yyyy";
            break;
        case SPSS_FORMAT_TYPE_MOYR:
            result = "Mon yyyy";
            break;
        case SPSS_FORMAT_TYPE_WKYR:
            result = "ww WK yyyy";
            break;
        case SPSS_FORMAT_TYPE_DATETIME:
            result = "dd-Mon-yyyy hh:mm";
            break;
        default:
            break;
    }
    return result;
}
