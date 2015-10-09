
int readstat_open(const char *filename);
int readstat_close(int fd);
#if defined _WIN32 || defined __CYGWIN__
_off64_t readstat_lseek(int fildes, _off64_t offset, int whence);
#elif defined _AIX
off64_t readstat_lseek(int fildes, off64_t offset, int whence);
#else
off_t readstat_lseek(int fildes, off_t offset, int whence);
#endif
readstat_error_t readstat_update_progress(int fd, size_t file_size, 
        readstat_progress_handler progress_handler, void *user_ctx);
