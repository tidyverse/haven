#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "../readstat.h"
#include "../readstat_malloc.h"

%%{
    machine mr_extractor;

    action extract_mr_name {
        mr_name = (char *)readstat_malloc(p - start + 1);
        if (mr_name == NULL) {
            retval = READSTAT_ERROR_MALLOC;
            goto cleanup;
        }
        memcpy(mr_name, start, p - start);
        mr_name[p - start] = '\0';
    }

    action extract_mr_type {
        mr_type = *p;
        start = p + 1;
    }

    action extract_counted_value {
        int n_cv_digs = p - start;
        char *n_dig_str = (char *)readstat_malloc(n_cv_digs + 1);
        if (n_dig_str == NULL) {
            retval = READSTAT_ERROR_MALLOC;
            goto cleanup;
        }
        memcpy(n_dig_str, start, n_cv_digs);
        n_dig_str[n_cv_digs] = '\0';
        int n_digs = strtol(n_dig_str, NULL, 10);
        free(n_dig_str);
        if (n_digs != 0) {
            char *cv = (char *)readstat_malloc(n_digs + 1);
            if (cv == NULL) {
                retval = READSTAT_ERROR_MALLOC;
                goto cleanup;
            }
            memcpy(cv, p + 1, n_digs);
            cv[n_digs] = '\0';
            mr_counted_value = strtol(cv, NULL, 10);
            free(cv);
            p = p + 1 + n_digs;
            start = p + 1;
        }
        else {
            mr_counted_value = -1;
        }
    }

    action extract_label {
        char *lbl_len_str = (char *)readstat_malloc(p - start + 1);
        if (lbl_len_str == NULL) {
            retval = READSTAT_ERROR_MALLOC;
            goto cleanup;
        }
        memcpy(lbl_len_str, start, p - start);
        lbl_len_str[p - start] = '\0';
        int len = strtol(lbl_len_str, NULL, 10);
        free(lbl_len_str);
        mr_label = (char *)readstat_malloc(len + 1);
        if (mr_label == NULL) {
            retval = READSTAT_ERROR_MALLOC;
            goto cleanup;
        }
        memcpy(mr_label, p + 1, len);
        mr_label[len] = '\0';
        p = p + 1 + len;
        start = p + 1;
    }

    action extract_subvar {
        int len = p - start;
        char *subvar = (char *)readstat_malloc(len + 1);
        if (subvar == NULL) {
            retval = READSTAT_ERROR_MALLOC;
            goto cleanup;
        }
        memcpy(subvar, start, len);
        subvar[len] = '\0';
        start = p + 1;    
        char **new_subvariables = readstat_realloc(mr_subvariables, sizeof(char *) * (mr_subvar_count + 1));
        if (new_subvariables == NULL) {
            free(subvar);
            retval = READSTAT_ERROR_MALLOC;
            goto cleanup;
        }
        mr_subvariables = new_subvariables;
        mr_subvariables[mr_subvar_count++] = subvar;
    }

    nc = (alnum | '_' | '.' ); # name character (including dots)
    name = nc+ '=' > extract_mr_name;
    type = ('C' | 'D'){1} > extract_mr_type;
    counted_value = digit* ' ' > extract_counted_value;
    label = digit+ ' '+ > extract_label;

    end = (space | '\0'); # subvar token terminator
    subvariable = (nc+ end >extract_subvar);

    main := name type counted_value label subvariable+;

    write data nofinal noerror;
}%%

readstat_error_t extract_mr_data(const char *line, mr_set_t *result) {
    readstat_error_t retval = READSTAT_OK;

    // Variables needed for Ragel operation
    int cs = 0;
    char *p = (char *)line;
    char *start = p;
    char *pe = p + strlen(p) + 1;

    // Variables needed for passing Ragel intermediate results
    char mr_type = '\0';
    int mr_counted_value = -1;
    int mr_subvar_count = 0;
    char **mr_subvariables = NULL;
    char *mr_name = NULL;
    char *mr_label = NULL;

    // Execute Ragel finite state machine (FSM)
    %% write init;
    %% write exec;

    // Check if FSM finished successfully
    if (cs < %%{ write first_final; }%% || p != pe) {
        retval = READSTAT_ERROR_BAD_MR_STRING;
        goto cleanup;
    }

    (void)mr_extractor_en_main;

    // Assign parsed values to output parameter
    result->name = mr_name;
    result->label = mr_label;
    result->type = mr_type;
    result->counted_value = mr_counted_value;
    result->subvariables = mr_subvariables;
    result->num_subvars = mr_subvar_count;
    if (result->type == 'D') {
        result->is_dichotomy = 1;
    }

cleanup:
    if (retval != READSTAT_OK) {
        if (mr_subvariables != NULL) {
            for (int i = 0; i < mr_subvar_count; i++) {
                if (mr_subvariables[i] != NULL) free(mr_subvariables[i]);
            }
            free(mr_subvariables);
        }
        if (mr_name != NULL) free(mr_name);
        if (mr_label != NULL) free(mr_label);
    }
    return retval;
}


readstat_error_t parse_mr_line(const char *line, mr_set_t *result) {
    *result = (mr_set_t){0};
    return extract_mr_data(line, result);
}

%%{
    machine mr_parser;

    action mr_line {
        char *mln = (char *)readstat_malloc(p - start);
        if (mln == NULL) {
            retval = READSTAT_ERROR_MALLOC;
            goto cleanup;
        }
        memcpy(mln, start + 1, p - start);
        mln[p - start - 1] = '\0';
        mr_set_t *new_mr_sets = readstat_realloc(*mr_sets, ((*n_mr_lines) + 1) * sizeof(mr_set_t));
        if (new_mr_sets == NULL) {
            free(mln);
            retval = READSTAT_ERROR_MALLOC;
            goto cleanup;
        }
        *mr_sets = new_mr_sets;
        retval = parse_mr_line(mln, &(*mr_sets)[*n_mr_lines]);
        free(mln);
        if (retval != READSTAT_OK) {
            goto cleanup;
        }
        (*n_mr_lines)++;
        start = p + 1;
    }
    line_start = '$';
    line_end = '\n';
    line_char = any - (line_end + line_start);
    mr_line = line_start line_char* line_end > mr_line;
    main := mr_line+ '\0';

    write data nofinal noerror;
}%%

readstat_error_t parse_mr_string(const char *line, mr_set_t **mr_sets, size_t *n_mr_lines) {
    readstat_error_t retval = READSTAT_OK;
    int cs = 0;
    char *p = (char *)line;
    char *start = p;
    char *pe = p + strlen(p) + 1;
    *mr_sets = NULL;
    *n_mr_lines = 0;

    %% write init;
    %% write exec;

    if (cs < %%{ write first_final; }%% || p != pe) {
        retval = READSTAT_ERROR_BAD_MR_STRING;
        goto cleanup;
    }

    (void)mr_parser_en_main;

cleanup:
    return retval;
}
