#include <stdlib.h>

#define MAX_MALLOC_SIZE (1<<20) /* One megabyte ought to be enough for anyone */

void *readstat_malloc(size_t len) {
    if (len > MAX_MALLOC_SIZE || len == 0) {
        return NULL;
    }
    return malloc(len);
}

void *readstat_calloc(size_t count, size_t size) {
    if (count > MAX_MALLOC_SIZE || size > MAX_MALLOC_SIZE || count * size > MAX_MALLOC_SIZE) {
        return NULL;
    }
    if (count == 0 || size == 0) {
        return NULL;
    }
    return calloc(count, size);
}

void *readstat_realloc(void *ptr, size_t len) {
    if (len > MAX_MALLOC_SIZE || len == 0) {
        if (ptr)
            free(ptr);
        return NULL;
    }
    return realloc(ptr, len);
}
