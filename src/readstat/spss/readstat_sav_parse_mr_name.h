#ifndef READSTAT_PARSE_MR_NAME_H
#define READSTAT_PARSE_MR_NAME_H

#include "../readstat.h"
#include "../readstat_malloc.h"

readstat_error_t parse_mr_string(const char *line, mr_set_t **mr_sets, size_t *n_mr_lines);

#endif // READSTAT_PARSE_MR_NAME_H
