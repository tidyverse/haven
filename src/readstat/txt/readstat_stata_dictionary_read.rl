#include <stdlib.h>

#include "../readstat.h"
#include "readstat_schema.h"

#include "readstat_copy.h"

%%{
    machine stata_dictionary;
    write data noerror nofinal;
}%%

readstat_schema_t *readstat_parse_stata_dictionary(readstat_parser_t *parser,
    const char *filepath, void *user_ctx, readstat_error_t *outError) {
    if (parser->io->open(filepath, parser->io->io_ctx) == -1) {
        if (outError)
            *outError = READSTAT_ERROR_OPEN;
        return NULL;
    }
    readstat_schema_t *schema = NULL;
    unsigned char *bytes = NULL;
    int cb_return_value = READSTAT_HANDLER_OK;
    int total_entry_count = 0;
    int partial_entry_count = 0;
    readstat_error_t error = READSTAT_OK;
    ssize_t len = parser->io->seek(0, READSTAT_SEEK_END, parser->io->io_ctx);
    if (len == -1) {
        error = READSTAT_ERROR_SEEK;
        goto cleanup;
    }
    parser->io->seek(0, READSTAT_SEEK_SET, parser->io->io_ctx);

    bytes = malloc(len);

    parser->io->read(bytes, len, parser->io->io_ctx);

    unsigned char *p = bytes;
    unsigned char *pe = bytes + len;

    unsigned char *str_start = NULL;

    size_t str_len = 0;

    int cs;
//    u_char *eof = pe;

    int integer = 0;
    int current_row = 0;
    int current_col = 0;
    int line_no = 0;
    unsigned char *line_start = p;

    readstat_schema_entry_t current_entry;
    
    if ((schema = calloc(1, sizeof(readstat_schema_t))) == NULL) {
        error = READSTAT_ERROR_MALLOC;
        goto cleanup;
    }

    schema->rows_per_observation = 1;
    
    %%{
        action start_integer {
            integer = 0;
        }

        action incr_integer {
            integer = 10 * integer + (fc - '0');
        }

        action start_entry {
            memset(&current_entry, 0, sizeof(readstat_schema_entry_t));
            current_entry.decimal_separator = '.';
            current_entry.variable.type = READSTAT_TYPE_DOUBLE;
            current_entry.variable.index = total_entry_count;
        }

        action end_entry {
            current_entry.row = current_row;
            current_entry.col = current_col;
            current_col += current_entry.len;
            cb_return_value = READSTAT_HANDLER_OK;
            if (parser->handlers.variable) {
                current_entry.variable.index_after_skipping = partial_entry_count;
                cb_return_value = parser->handlers.variable(total_entry_count, &current_entry.variable, NULL, user_ctx);
                if (cb_return_value == READSTAT_HANDLER_ABORT) {
                    error = READSTAT_ERROR_USER_ABORT;
                    goto cleanup;
                }
            }
            if (cb_return_value == READSTAT_HANDLER_SKIP_VARIABLE) {
                current_entry.skip = 1;
            } else {
                partial_entry_count++;
            } 
            schema->entries = realloc(schema->entries, sizeof(readstat_schema_entry_t) * (schema->entry_count+1));
            memcpy(&schema->entries[schema->entry_count++], &current_entry, sizeof(readstat_schema_entry_t));
            total_entry_count++;
        }

        action copy_filename {
            readstat_copy(schema->filename, sizeof(schema->filename), (char *)str_start, str_len);
        }

        action copy_varname {
            readstat_copy(current_entry.variable.name, sizeof(current_entry.variable.name),
                 (char *)str_start, str_len);
        }

        action copy_varlabel {
            readstat_copy(current_entry.variable.label, sizeof(current_entry.variable.label),
                 (char *)str_start, str_len);
        }

        quoted_string = "\"" ( [^"]* ) >{ str_start = fpc; } %{ str_len = fpc - str_start; } "\"";

        unquoted_string = [A-Za-z0-9_/\\\.\-]+ >{ str_start = fpc; } %{ str_len = fpc - str_start; };

        identifier = ( [A-Za-z] [_\.A-Za-z0-9]* ) >{ str_start = fpc; } %{ str_len = fpc - str_start; };

        newline = ( "\n" | "\r\n" ) %{ line_no++; line_start = p; };
        
        spacetab = [ \t];
        
        whitespace = spacetab | newline;

        filename = ( quoted_string | unquoted_string ) %copy_filename;

        integer = [0-9]+ >start_integer $incr_integer;

        lines_marker = "_lines(" spacetab* integer spacetab* ")" %{ schema->rows_per_observation = integer; };

        line_marker = "_line(" spacetab* integer spacetab* ")" %{ current_row = integer - 1; };

        column_marker = "_column(" spacetab* integer spacetab* ")" %{ current_col = integer - 1; };

        newline_marker = "_newline" %{ current_row++; } ( "(" spacetab* integer spacetab* ")" %{ current_row += (integer - 1); } )?;

        skip_marker = "_skip(" spacetab* integer spacetab* ")" %{ current_col += (integer - 1) };

        lrecl_marker = "_lrecl(" spacetab* integer spacetab* ")" %{ schema->cols_per_observation = integer; };

        firstlineoffile_marker = "_firstlineoffile(" spacetab* integer spacetab* ")" %{ schema->first_line = integer - 1; };

        marker = lrecl_marker | firstlineoffile_marker | lines_marker | line_marker | column_marker | newline_marker;

        type = "byte" %{ current_entry.variable.type = READSTAT_TYPE_INT8; }
            | "int" %{ current_entry.variable.type = READSTAT_TYPE_INT16; }
            | "long" %{ current_entry.variable.type = READSTAT_TYPE_INT32; }
            | "float" %{ current_entry.variable.type = READSTAT_TYPE_FLOAT; }
            | "double" %{ current_entry.variable.type = READSTAT_TYPE_DOUBLE; }
            | "str" integer %{ current_entry.variable.type = READSTAT_TYPE_STRING;
                               current_entry.variable.storage_width = integer; };

        varname = identifier %copy_varname;

        varlabel = quoted_string %copy_varlabel;

        format = "%" integer %{ current_entry.len = integer; } 
        ( "s" | "S" | ( ( ( "." | "," %{ current_entry.decimal_separator = ','; } ) integer )? ( "f" | "g" | "e" ) ) );

        entry = ( ( type spacetab+ )? varname ( spacetab+ format )? ( spacetab+ varlabel )? spacetab* newline ) >start_entry %end_entry;

        comment = "*" [^\r\n]* newline | "/*" ( any* - ( any* "*/" any* ) ) "*/";

        contents = ( whitespace* ( marker | entry | comment ) )* whitespace*;

        main := comment* ("infile" whitespace+)? "dictionary" whitespace+ 
                ( "using" whitespace+ filename whitespace+ )?
                                "{" contents "}" any*;

        write init;
        write exec;
    }%%

    /* suppress warnings */
    (void)stata_dictionary_en_main;

    if (cs < %%{ write first_final; }%%) {
        char error_buf[1024];
        if (p == pe) {
            snprintf(error_buf, sizeof(error_buf), "Error parsing .dct file (end-of-file unexpectedly reached)");
        } else {
            snprintf(error_buf, sizeof(error_buf), "Error parsing .dct file around line #%d, col #%ld (%c)",
                    line_no + 1, (long)(p - line_start + 1), *p);
        }
        if (parser->handlers.error) {
            parser->handlers.error(error_buf, user_ctx);
        }
        error = READSTAT_ERROR_PARSE;
        goto cleanup;
    }

cleanup:
    parser->io->close(parser->io->io_ctx);
    free(bytes);
    if (error != READSTAT_OK) {
        if (outError)
            *outError = error;
        readstat_schema_free(schema);
        schema = NULL;
    }

    return schema;
}
