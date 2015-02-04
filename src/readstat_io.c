
#include <fcntl.h>
#include <unistd.h>

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
