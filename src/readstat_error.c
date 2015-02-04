
#include "readstat.h"

const char *readstat_error_message(readstat_error_t error_code) {
    if (error_code == READSTAT_OK)
        return NULL;

    if (error_code == READSTAT_ERROR_OPEN)
        return "Unable to open file";

    if (error_code == READSTAT_ERROR_READ)
        return "Unable to read file";

    if (error_code == READSTAT_ERROR_MALLOC)
        return "Unable to allocate memory";

    if (error_code == READSTAT_ERROR_USER_ABORT)
        return "The parsing was aborted (callback returned non-zero value)";

    if (error_code == READSTAT_ERROR_PARSE)
        return "Invalid file, or file has unsupported features";

    if (error_code == READSTAT_ERROR_UNSUPPORTED_COMPRESSION)
        return "File has unsupported compression scheme";

    if (error_code == READSTAT_ERROR_UNSUPPORTED_CHARSET)
        return "File has an unsupported character set";

    if (error_code == READSTAT_ERROR_ROW_COUNT_MISMATCH)
        return "File did not contain the expected number of rows";

    if (error_code == READSTAT_ERROR_ROW_WIDTH_MISMATCH)
        return "A row in the file was not the expected length";

    return "Unknown error";
}
