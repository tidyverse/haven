
#include <fcntl.h>
#include <unistd.h>

#include "readstat.h"

int readstat_open(const char *filename) {
    return open(filename, O_RDONLY
#if defined _WIN32 || defined __CYGWIN__
            | O_BINARY
#elif defined _AIX
            | O_LARGEFILE
#endif
            );
}

int readstat_close(int fd) {
    return close(fd);
}

#if defined _WIN32 || defined __CYGWIN__
_off64_t readstat_lseek(int fildes, _off64_t offset, int whence) {
    return lseek64(fildes, offset, whence);
}
#elif defined _AIX
off64_t readstat_lseek(int fildes, off64_t offset, int whence) {
    return lseek64(fildes, offset, whence);
}
#else
off_t readstat_lseek(int fildes, off_t offset, int whence) {
    return lseek(fildes, offset, whence);
}
#endif

readstat_error_t readstat_update_progress(int fd, long file_size, 
        readstat_progress_handler progress_handler, void *user_ctx) {
    long current_offset = readstat_lseek(fd, 0, SEEK_CUR);
    if (current_offset == -1)
        return READSTAT_ERROR_READ;

    if (progress_handler(1.0 * current_offset / file_size, user_ctx))
        return READSTAT_ERROR_USER_ABORT;

    return READSTAT_OK;
}
