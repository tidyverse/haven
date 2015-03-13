
#include <stdlib.h>
#include "readstat.h"

readstat_parser_t *readstat_parser_init() {
    readstat_parser_t *parser = calloc(1, sizeof(readstat_parser_t));
    return parser;
}

void readstat_parser_free(readstat_parser_t *parser) {
    if (parser)
        free(parser);
}

readstat_error_t readstat_set_info_handler(readstat_parser_t *parser, readstat_info_handler info_handler) {
    parser->info_handler = info_handler;
    return READSTAT_OK;
}

readstat_error_t readstat_set_variable_handler(readstat_parser_t *parser, readstat_variable_handler variable_handler) {
    parser->variable_handler = variable_handler;
    return READSTAT_OK;
}

readstat_error_t readstat_set_value_handler(readstat_parser_t *parser, readstat_value_handler value_handler) {
    parser->value_handler = value_handler;
    return READSTAT_OK;
}

readstat_error_t readstat_set_value_label_handler(readstat_parser_t *parser, readstat_value_label_handler label_handler) {
    parser->value_label_handler = label_handler;
    return READSTAT_OK;
}

readstat_error_t readstat_set_error_handler(readstat_parser_t *parser, readstat_error_handler error_handler) {
    parser->error_handler = error_handler;
    return READSTAT_OK;
}

readstat_error_t readstat_set_progress_handler(readstat_parser_t *parser, readstat_progress_handler progress_handler) {
    parser->progress_handler = progress_handler;
    return READSTAT_OK;
}

rdata_parser_t *rdata_parser_init() {
    rdata_parser_t *parser = calloc(1, sizeof(rdata_parser_t));
    return parser;
}

void rdata_parser_free(rdata_parser_t *parser) {
    if (parser)
        free(parser);
}

readstat_error_t rdata_set_table_handler(rdata_parser_t *parser, rdata_table_handler table_handler) {
    parser->table_handler = table_handler;
    return READSTAT_OK;
}

readstat_error_t rdata_set_column_handler(rdata_parser_t *parser, rdata_column_handler column_handler) {
    parser->column_handler = column_handler;
    return READSTAT_OK;
}

readstat_error_t rdata_set_column_name_handler(rdata_parser_t *parser, rdata_column_name_handler column_name_handler) {
    parser->column_name_handler = column_name_handler;
    return READSTAT_OK;
}

readstat_error_t rdata_set_text_value_handler(rdata_parser_t *parser, rdata_text_value_handler text_value_handler) {
    parser->text_value_handler = text_value_handler;
    return READSTAT_OK;
}

readstat_error_t rdata_set_value_label_handler(rdata_parser_t *parser, rdata_text_value_handler value_label_handler) {
    parser->value_label_handler = value_label_handler;
    return READSTAT_OK;
}

readstat_error_t rdata_set_error_handler(rdata_parser_t *parser, readstat_error_handler error_handler) {
    parser->error_handler = error_handler;
    return READSTAT_OK;
}
