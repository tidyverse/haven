
int readstat_open(const char *filename);
int readstat_close(int fd);
readstat_error_t readstat_update_progress(int fd, size_t file_size, 
        readstat_progress_handler progress_handler, void *user_ctx);
