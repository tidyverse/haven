#include <stdlib.h>
#include <strings.h>
#include <inttypes.h>

#include "../readstat.h"
#include "readstat_schema.h"

#include "readstat_copy.h"
#include "commands_util.h"

%%{
    machine spss_commands;
    write data noerror nofinal;
}%%

readstat_schema_t *readstat_parse_spss_commands(readstat_parser_t *parser,
    const char *filepath, void *user_ctx, readstat_error_t *outError) {
    readstat_schema_t *schema = NULL;
    unsigned char *bytes = NULL;
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
    
    unsigned char *eof = pe;
    
    unsigned char *str_start = NULL;

    size_t str_len = 0;
    
    int cs;
    
    int i;
    int line_no = 0;
    uint64_t first_integer = 0, integer = 0;
    double double_value = NAN;
    unsigned char *line_start = p;

    char varname[32];
    char argname[32];
    char string_value[32];
    char buf[1024];

    char var_list[1024][32];

    long var_col = 0;
    long var_row = 0;
    long var_len = 0;
    long var_count = 0;
    readstat_type_t var_type = READSTAT_TYPE_DOUBLE;
    label_type_t label_type = LABEL_TYPE_DOUBLE;

    int labelset_count = 0;

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

        action copy_pos {
            var_col = integer - 1;
            var_len = 1;
        }

        action set_len {
            var_len = integer - var_col;
        }

        action copy_quoted_buf {
            readstat_copy_quoted(buf, sizeof(buf), (char *)str_start, str_len);
        }
        
        action copy_quoted_string {
            readstat_copy_quoted(string_value, sizeof(string_value), (char *)str_start, str_len);
        }

        action copy_string {
            readstat_copy(string_value, sizeof(string_value), (char *)str_start, str_len);
        }

        action copy_varname {
            readstat_copy(varname, sizeof(varname), (char *)str_start, str_len);
        }
        
        action copy_argname {
            readstat_copy(argname, sizeof(argname), (char *)str_start, str_len);
        }

        action handle_var {
            readstat_schema_entry_t *entry = readstat_schema_find_or_create_entry(schema, varname);
            entry->variable.type = var_type;
            entry->row = var_row;
            entry->col = var_col;
            entry->len = var_len;
        }

        action handle_var_label {
            readstat_schema_entry_t *entry = readstat_schema_find_or_create_entry(schema, varname);
            readstat_copy(entry->variable.label, sizeof(entry->variable.label), buf, sizeof(buf));
        }

        action reset_variable_list {
            var_count = 0;
        }

        action add_variable_to_list {
            if (var_count < sizeof(var_list)/sizeof(var_list[0])) {
                memcpy(var_list[var_count++], varname, sizeof(varname));
            }
        }
        
        action handle_get_data_arg {
            if (strcasecmp(argname, "FIRSTCASE") == 0) {
                schema->first_line = integer;
            }
            if (strcasecmp(argname, "DELIMITERS") == 0) {
                schema->field_delimiter = buf[0];
            }
        }

        action handle_labelset {
            char labelset_name[256];
            snprintf(labelset_name, sizeof(labelset_name), "labels%d", labelset_count++);
            for (i=0; i<var_count; i++) {
                readstat_schema_entry_t *entry = readstat_schema_find_or_create_entry(schema, varname);
                readstat_copy(entry->labelset, sizeof(entry->labelset), labelset_name, sizeof(labelset_name));
            }
        }
        
        action handle_value_label {
            char labelset_name[256];
            snprintf(labelset_name, sizeof(labelset_name), "labels%d", labelset_count);
            error = submit_value_label(parser, labelset_name, label_type,
                first_integer, integer, double_value, string_value, buf, user_ctx);
            if (error != READSTAT_OK)
                goto cleanup;
        }
        
        single_quoted_string = "'" ( [^']* ) >{ str_start = fpc; } %{ str_len = fpc - str_start; } "'";
        
        double_quoted_string = "\"" ( [^"]* ) >{ str_start = fpc; } %{ str_len = fpc - str_start; } "\"";
        
        quoted_string = ( single_quoted_string | double_quoted_string ) %copy_quoted_buf;
        
        newline = ( "\n" | "\r\n" ) %{ line_no++; line_start = p; };
        
        identifier = ( [A-Za-z] [_A-Za-z0-9]* ) >{ str_start = fpc; } %{ str_len = fpc - str_start; };
                                       
        integer = [0-9]+ >start_integer $incr_integer;
                                       
        double_value = "-"? integer ("." integer)?;
                                       
        whitespace = [ \t] | newline;
        
        pos = ( integer %copy_pos ("-" whitespace* integer %set_len)? );
                                       
        multiline_comment = "/*" ( any* - ( any* "*/" any* ) ) "*/";
                                       
        comment = "*" ( any* - ( any* "." whitespace* newline any* ) ) "." whitespace* newline | 
            multiline_comment |
            "COMMENT " [^\.]* ".";
                                       
        var = identifier %copy_varname;

        width = (whitespace+ "(A"i integer? ")" %{ var_type = READSTAT_TYPE_STRING; } |
                whitespace+ "(" integer ")" )?;
                                       
       type = ( "A"i integer %{ var_type = READSTAT_TYPE_STRING; }
               | "F"i integer %{ var_type = READSTAT_TYPE_DOUBLE; } ("." integer)?
               | "DATE"i integer %{ var_type = READSTAT_TYPE_DOUBLE; }
               | "ADATE"i integer %{ var_type = READSTAT_TYPE_STRING; } );
        
        slash_arg = "/" identifier %copy_argname whitespace* "=" whitespace* (identifier | quoted_string | integer);

        slash_args = slash_arg ( whitespace* slash_arg)*;
                                       
        select_cmd = "SELECT"i whitespace+ "IF"i whitespace+
                                       (whitespace | identifier | "-"? integer
                                        | "(" | ")" | quoted_string
                                        )+ ".";

        file_handle_cmd = "FILE"i whitespace+ "HANDLE"i whitespace+ identifier whitespace+ slash_args whitespace* ".";

        save_cmd = "SAVE"i whitespace+ ( "OUTFILE"i | "DICTIONARY"i ) whitespace* "="? whitespace* quoted_string whitespace* ("/" whitespace* "COMPRESSED" whitespace*)? ".";
        
        data_list_arg = ( "RECORD"i ("S"i)? whitespace* "=" whitespace* integer |
                         "FILE"i whitespace* "=" whitespace* (quoted_string | identifier) |
                         "TABLE"i | "FIXED"i );
        data_list_args = data_list_arg (whitespace+ data_list_arg)*;
        
        data_list_cmd = "DATA"i whitespace+ "LIST"i whitespace+ data_list_args whitespace*
            ( "/" ( integer %{ var_row = integer - 1; } )?
            whitespace+ var whitespace+ pos %{ var_type = READSTAT_TYPE_DOUBLE; } width
            (whitespace+ var >handle_var whitespace+ pos %{ var_type = READSTAT_TYPE_DOUBLE; } width )+ whitespace* )+
            "." %handle_var;
                                       
        get_data_variable = var whitespace+ type %handle_var;
                                       
        get_data_variable_list = get_data_variable ( whitespace+ get_data_variable )*;
                                       
        get_data_arg = ( slash_arg %handle_get_data_arg | "/VARIABLES"i whitespace* "=" whitespace* get_data_variable_list );
                                       
        get_data_args = get_data_arg (whitespace* get_data_arg)*;
                                       
        get_data_cmd = "GET"i whitespace+ "DATA"i whitespace+ get_data_args whitespace* ".";
                                       
        get_file_cmd = "GET"i whitespace+ "FILE"i whitespace* "=" whitespace* quoted_string whitespace* ".";
        
        dataset_cmd_arg = "WINDOW"i whitespace* "=" whitespace* identifier;
                                       
        dataset_cmd_args = dataset_cmd_arg (whitespace+ dataset_cmd_arg)*;
                                       
        dataset_cmd = "DATASET"i whitespace+ "NAME"i whitespace+ (identifier | quoted_string) (whitespace+ dataset_cmd_args)? ".";
                                       
        format_string = "F" integer "." integer;
                                       
        format_spec = identifier whitespace+ "(" format_string ")";
                                       
        formats_cmd = "FORMATS"i whitespace+ format_spec (whitespace+ "/" whitespace+ format_spec)* whitespace* ".";

        variable_labels_cmd = "VARIABLE"i whitespace+ "LABEL"i ("S"i)?
            (whitespace+ var whitespace+ quoted_string %handle_var_label (whitespace* "/")? )+
            whitespace* ".";

        variable_list = var %reset_variable_list %add_variable_to_list (whitespace+ var %add_variable_to_list)*;

        missing_value_label = "." %{ label_type = -1; } whitespace+ quoted_string %handle_value_label;
        
        value_label = ( "-" integer %{ label_type = LABEL_TYPE_DOUBLE; double_value = -integer; } |
                       integer %{ label_type = LABEL_TYPE_DOUBLE; double_value = integer; } |
                       integer whitespace+ "-" whitespace+ %{ first_integer = integer; } integer %{ label_type = LABEL_TYPE_RANGE; } |
                       quoted_string %{ label_type = LABEL_TYPE_STRING; } %copy_quoted_string )
            whitespace+ quoted_string %handle_value_label;

        variable_value_labels = variable_list whitespace+ ( value_label | missing_value_label )
            (whitespace+ value_label)* %handle_labelset;
                                       
        variable_level = variable_list whitespace+ ( "(SCALE)"i | "(NOMINAL)"i | "(ORDINAL)"i );
                                       
        variable_level_subcmd = "VARIABLE"i whitespace+ "LEVEL"i whitespace+ variable_level
            ( whitespace* "/" whitespace* variable_level )*;

        value_labels_cmd = "VALUE"i whitespace+ "LABELS"i whitespace+ ("/" whitespace*)? variable_value_labels
            ( whitespace+ "/" whitespace* variable_value_labels )*
            whitespace*  ("/" whitespace* ( variable_level_subcmd whitespace* )? )? ".";
                                       
        recode_cmd = "RECODE"i whitespace+ identifier whitespace+ "(" double_value (whitespace+ double_value)* "=" whitespace* "SYSMIS" whitespace* ")" whitespace* ".";
                                       
        execute_cmd = "EXECUTE"i whitespace* ".";
        list_cmd = "LIST"i whitespace* ".";
        display_cmd = "DISPLAY"i (whitespace+ identifier)* whitespace* ".";
        input_program_cmd = "INPUT"i whitespace+ "PROGRAM"i whitespace* ".";
        end_input_program_cmd = "END"i whitespace+ input_program_cmd;
        set_cmd = "SET"i whitespace+ identifier whitespace* "=" whitespace* (identifier | integer | quoted_string) whitespace* ".";

        command = 
            file_handle_cmd | 
            data_list_cmd |
            get_data_cmd |
            get_file_cmd |
            dataset_cmd |
            display_cmd |
            formats_cmd |
            variable_labels_cmd |
            value_labels_cmd |
            recode_cmd |
            select_cmd |
            save_cmd | 
            list_cmd |
            input_program_cmd |
            end_input_program_cmd |
            set_cmd |
            execute_cmd;
        
        main := ( whitespace | comment | command )*;

        write init;
        write exec;
    }%%
                                       
   /* suppress warnings */
   (void)spss_commands_en_main;
                                       
    if (cs < %%{ write first_final; }%%) {
        char error_buf[1024];
        if (p == pe) {
            snprintf(error_buf, sizeof(error_buf), "Error parsing SPSS command file (end-of-file unexpectedly reached)");
        } else {
            snprintf(error_buf, sizeof(error_buf), "Error parsing SPSS command file around line #%d, col #%ld (%c)",
                    line_no + 1, (long)(p - line_start + 1), *p);
        }
        if (parser->handlers.error) {
            parser->handlers.error(error_buf, user_ctx);
        }
        error = READSTAT_ERROR_PARSE;
        goto cleanup;
    }

    error = submit_columns(parser, schema, user_ctx);

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
