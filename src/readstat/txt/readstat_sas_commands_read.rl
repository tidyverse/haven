#include <stdlib.h>
#include <strings.h>

#include "../readstat.h"
#include "readstat_schema.h"

#include "readstat_copy.h"
#include "commands_util.h"

%%{
    machine sas_commands;
    write data noerror nofinal;
}%%

readstat_schema_t *readstat_parse_sas_commands(readstat_parser_t *parser,
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
    
    double double_value = NAN;
    uint64_t first_integer = 0;
    uint64_t integer = 0;
    int line_no = 0;
    unsigned char *line_start = p;

    char varname[32];
    char argname[32];
    char labelset[32];
    char string_value[32];
    char buf[1024];

    readstat_type_t var_type = READSTAT_TYPE_DOUBLE;
    label_type_t label_type = LABEL_TYPE_DOUBLE;
    int var_row = 0, var_col = 0;
    int var_len = 0;

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

        action incr_hex_integer {
            int value = 0;
            if (fc >= '0' && fc <= '9') {
                value = fc - '0';
            } else if (fc >= 'A' && fc <= 'F') {
                value = fc - 'A' + 10;
            } else if (fc >= 'a' && fc <= 'f') {
                value = fc - 'a' + 10;
            }
            integer = 16 * integer + value;
        }

        action copy_pos {
            var_col = integer - 1;
            var_len = 1;
        }

        action set_len {
            var_len = integer - var_col;
        }
                                       
        action set_str {
            var_type = READSTAT_TYPE_STRING;
        }
                                       
        action set_dbl {
            var_type = READSTAT_TYPE_DOUBLE;
        }

        action copy_buf {
            readstat_copy(buf, sizeof(buf), (char *)str_start, str_len);
        }

        action copy_labelset {
            readstat_copy(labelset, sizeof(labelset), (char *)str_start, str_len);
        }

        action copy_string {
            readstat_copy(string_value, sizeof(string_value), (char *)str_start, str_len);
        }

        action copy_argname {
            readstat_copy(argname, sizeof(argname), (char *)str_start, str_len);
        }

        action copy_varname {
            readstat_copy_lower(varname, sizeof(varname), (char *)str_start, str_len);
        }

        action handle_arg {
            if (strcasecmp(argname, "firstobs") == 0) {
                schema->first_line = integer;
            }
            if (strcasecmp(argname, "dlm") == 0) {
                schema->field_delimiter = integer ? integer : buf[0];
            }
        }

        action handle_var {
            readstat_schema_entry_t *entry = readstat_schema_find_or_create_entry(schema, varname);
            entry->variable.type = var_type;
            entry->row = var_row;
            entry->col = var_col;
            entry->len = var_len;
        }

        action handle_var_len {
            readstat_schema_entry_t *entry = readstat_schema_find_or_create_entry(schema, varname);
            entry->len = var_len;
        }

        action handle_var_label {
            readstat_schema_entry_t *entry = readstat_schema_find_or_create_entry(schema, varname);
            readstat_copy(entry->variable.label, sizeof(entry->variable.label), buf, sizeof(buf));
        }

        action handle_var_labelset {
            readstat_schema_entry_t *entry = readstat_schema_find_or_create_entry(schema, varname);
            readstat_copy(entry->labelset, sizeof(entry->labelset), labelset, sizeof(labelset));
        }

        action handle_value_label {
            error = submit_value_label(parser, labelset, label_type,
                first_integer, integer, double_value, string_value, buf, user_ctx); 
            if (error != READSTAT_OK)
                goto cleanup;
        }
        
        single_quoted_string = "'" ( [^']* ) >{ str_start = fpc; } %{ str_len = fpc - str_start; } "'";
        
        double_quoted_string = "\"" ( [^"]* ) >{ str_start = fpc; } %{ str_len = fpc - str_start; } "\"";

        unquoted_string = [A-Za-z] [_A-Za-z0-9\.]*;
        
        quoted_string = ( single_quoted_string | double_quoted_string ) %copy_buf;

        hex_string = "'" ( [0-9A-Fa-f]+ ) >start_integer $incr_hex_integer "'x";
        
        newline = ( "\n" | "\r\n" ) %{ line_no++; line_start = p; };
                                       
        missing_value = "." [A-Z]?;
        
        identifier = ( [$_A-Za-z] [_A-Za-z0-9]* ) >{ str_start = fpc; } %{ str_len = fpc - str_start; };

        identifier_eval = "&"? identifier "."?;
        
        integer = [0-9]+ >start_integer $incr_integer;
        
        true_whitespace = [ \t] | newline;
                                       
        multiline_comment = "/*" ( any* - ( any* "*/" any* ) ) "*/";

        comment = "*" ( any* - ( any* ";" true_whitespace* newline any* ) ) ";" true_whitespace* newline |
            multiline_comment;
                                       
        whitespace = true_whitespace | multiline_comment;
        
        var = identifier %copy_varname;

        labelset = identifier %copy_labelset;

        arg = identifier %copy_argname (whitespace* "=" whitespace* (identifier_eval | quoted_string | hex_string | integer) >start_integer %handle_arg)?;

        args = arg ( whitespace+ arg)*;

        options_cmd = "OPTIONS"i whitespace+ args
            whitespace* ";";

        let_macro = "%LET"i whitespace+ identifier whitespace* "=" whitespace* (unquoted_string | quoted_string) 
            whitespace* ";";

        libname_cmd = "LIBNAME"i whitespace+ identifier whitespace+ ( 
                quoted_string (whitespace+ args)? | 
                "CLEAR"i | "_ALL_"i whitespace* "CLEAR"i |
                "LIST"i | "_ALL_"i whitespace* "LIST"i
                ) whitespace* ";";
                                       
        footnote_cmd = "FOOTNOTE"i whitespace+ quoted_string whitespace* ";";

        empty_cmd = ";";

        value_label = ( "-" integer %{ label_type = LABEL_TYPE_DOUBLE; double_value = -integer; } |
                integer %{ label_type = LABEL_TYPE_DOUBLE; double_value = integer; } |
                integer whitespace+ "-" whitespace+ %{ first_integer = integer; } integer %{ label_type = LABEL_TYPE_RANGE; } |
                unquoted_string %{ label_type = LABEL_TYPE_STRING; } %copy_string |
                quoted_string %{ label_type = LABEL_TYPE_STRING; } %copy_string
                ) whitespace* "=" whitespace* quoted_string %handle_value_label;

        var_len = ("$" whitespace* integer %set_str | integer %set_dbl) %{ var_len = integer; };

        value_cmd = "VALUE"i whitespace+ labelset whitespace+ ("(" args ")" whitespace*)?
            value_label (whitespace+ value_label)* 
            whitespace* ";";
        
        proc_format_cmd = "PROC"i whitespace+ "FORMAT"i whitespace* ( args whitespace* )? ";"
            ( whitespace | empty_cmd | value_cmd )+;

        filename_cmd = "FILENAME"i (whitespace+ args)? whitespace+ quoted_string
            whitespace* ";";

        if_statement = "IF"i ( whitespace | identifier | "-"? integer | "(" | ")" | ".")+ ";";

        data_cmd = "DATA"i (whitespace+ identifier_eval | unquoted_string | quoted_string )+
            whitespace* ";";

        missing_cmd = "MISSING"i whitespace+ identifier 
            whitespace* ";";

        # lrecl_option = "LRECL"i whitespace* "=" whitespace* integer %handle_info;

        infile_cmd = "INFILE"i (whitespace+ quoted_string)? (whitespace* args)?
            whitespace* ";";

        length_spec = var whitespace+ var_len %handle_var_len;

        length_cmd = "LENGTH"i whitespace+ length_spec (whitespace+ length_spec)*
            whitespace* ";";

        label_spec = var whitespace* "=" whitespace* quoted_string %handle_var_label;

        label_cmd = "LABEL"i whitespace+ label_spec (whitespace+ label_spec)*
            whitespace* ";";
                                       
        date_separator = [SN];
                                       
        date_format = ( "MMDDYY" integer |
                       "DATE" |
                       "DATE9" |
                       "DATETIME" |
                       "DAY" |
                       "DDMMYY" date_separator? integer |
                       "DOWNAME" |
                       "JULDAY" |
                       "JULIAN" |
                       "MMDDYY" date_separator? integer |
                       "MMYY" date_separator? |
                       "MONNAME" |
                       "MONTH" |
                       "MONYY" |
                       "PDFJULG" |
                       "WEEKDATE" |
                       "WEEKDAY" |
                       "WORDDATE" |
                       "WORDDATX" |
                       "QTR" |
                       "QTRR" |
                       "TIME" |
                       "TIMEAMPM" |
                       "TOD" |
                       "YEAR" |
                       "YYMMDD" |
                       "YYMM" date_separator? |
                       "YYQ" date_separator? |
                       "YYQR" date_separator? );
                       
                                       
        format_lbl_spec = labelset "." %handle_var_labelset;
                                       
        format_dbl_spec = integer "." integer?;
                                       
        format_date_spec = date_format "." integer?;
                                       
        var_format_spec = var whitespace+ ( format_lbl_spec | format_dbl_spec | format_date_spec );

        format_cmd = "FORMAT"i whitespace+ var_format_spec (whitespace+ var_format_spec)*
            whitespace* ";";
                                       
        var_attribute = (
                        "LENGTH"i whitespace* "=" whitespace* var_len %handle_var_len |
                        "LABEL"i whitespace* "=" whitespace* quoted_string %handle_var_label |
                        "FORMAT"i whitespace* "=" whitespace* format_dbl_spec
                        );
       
        var_attributes = var_attribute (whitespace+ var_attribute)*;
       
        attrib_spec = var whitespace+ var_attributes %handle_var;
       
        attrib_cmd = "ATTRIB"i whitespace+ attrib_spec (whitespace+ attrib_spec)* whitespace* ";";
                                       
        input_format_spec = ("$CHAR" integer %set_str | identifier %set_dbl);

        input_int_spec = var whitespace+ integer %copy_pos "-" integer %set_len %set_dbl %handle_var;
                                       
        input_dbl_spec = "@" integer %copy_pos whitespace+ var whitespace+ (var_len | input_format_spec) "." %handle_var integer?;

        input_txt_spec = var whitespace+ "$" whitespace+ integer %copy_pos "-" integer %set_len %set_str %handle_var;

        row_spec = "#" integer %{ var_row = integer - 1; };
                                       
        input_spec = (input_int_spec | input_dbl_spec | input_txt_spec | row_spec | var);

        input_cmd = "INPUT"i whitespace+ %{ var_row = 0; } input_spec (whitespace+ input_spec)*
            whitespace* ";";
                                       
        invalue_missing_spec = single_quoted_string whitespace* "=" whitespace* missing_value;
        
        invalue_format_spec = format_dbl_spec | format_date_spec;
        
        invalue_other_spec = "OTHER" whitespace* "=" whitespace* "(|" invalue_format_spec "|)";
                                       
        invalue_spec = invalue_missing_spec | invalue_other_spec;
                                       
        invalue_cmd = "INVALUE"i whitespace+ identifier whitespace+ invalue_spec (whitespace+ invalue_spec)* whitespace* ";";

        proc_print_cmd = "PROC"i whitespace+ "PRINT"i (whitespace+ args) (whitespace+ "(" args ")")? 
            whitespace* ";";

        proc_contents_cmd = "PROC"i whitespace+ "CONTENTS"i (whitespace+ args) whitespace* ";";

        run_cmd = "RUN"i whitespace* ";";

        command = 
            options_cmd |
            let_macro |
            libname_cmd |
            footnote_cmd |
            value_cmd |
            proc_format_cmd |
            filename_cmd |
            attrib_cmd |
            data_cmd |
            if_statement |
            missing_cmd |
            infile_cmd |
            format_cmd |
            label_cmd |
            length_cmd |
            input_cmd |
            invalue_cmd |
            proc_print_cmd |
            proc_contents_cmd |
            run_cmd;
        
        main := ( true_whitespace | comment | command )*;

        write init;
        write exec;
    }%%
                                       
   /* suppress warnings */
   (void)sas_commands_en_main;

    if (cs < %%{ write first_final; }%%) {
        char error_buf[1024];
        if (p == pe) {
            snprintf(error_buf, sizeof(error_buf), "Error parsing SAS command file (end-of-file unexpectedly reached)");
        } else {
            snprintf(error_buf, sizeof(error_buf), "Error parsing SAS command file around line #%d, col #%ld (%c)",
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
