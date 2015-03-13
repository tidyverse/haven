
#include <fcntl.h>
#include <unistd.h>

#include "readstat.h"

int readstat_open(const char *filename) {
    return open(filename, O_RDONLY
#if defined _WIN32 || defined __CYGWIN__
            | O_BINARY
#endif
            );
}

int readstat_close(int fd) {
    return close(fd);
}

readstat_error_t readstat_update_progress(int fd, size_t file_size, 
        readstat_progress_handler progress_handler, void *user_ctx) {
    off_t current_offset = lseek(fd, 0, SEEK_CUR);
    if (current_offset == -1)
        return READSTAT_ERROR_READ;

    if (progress_handler(1.0 * current_offset / file_size, user_ctx))
        return READSTAT_ERROR_USER_ABORT;

    return READSTAT_OK;
}
