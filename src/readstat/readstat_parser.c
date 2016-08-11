
#include <stdlib.h>
#include "readstat.h"
#include "readstat_io_unistd.h"

readstat_parser_t *readstat_parser_init() {
    readstat_parser_t *parser = calloc(1, sizeof(readstat_parser_t));
    parser->io = calloc(1, sizeof(readstat_io_t));
    unistd_io_init(parser);
    parser->output_encoding = "UTF-8";
    return parser;
}

void readstat_parser_free(readstat_parser_t *parser) {
    if (parser) {
        if (parser->io)
            free(parser->io);
        free(parser);
    }
}

readstat_error_t readstat_set_info_handler(readstat_parser_t *parser, readstat_info_handler info_handler) {
    parser->info_handler = info_handler;
    return READSTAT_OK;
}

readstat_error_t readstat_set_metadata_handler(readstat_parser_t *parser, readstat_metadata_handler metadata_handler) {
    parser->metadata_handler = metadata_handler;
    return READSTAT_OK;
}

readstat_error_t readstat_set_note_handler(readstat_parser_t *parser, readstat_note_handler note_handler) {
    parser->note_handler = note_handler;
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

readstat_error_t readstat_set_fweight_handler(readstat_parser_t *parser, readstat_fweight_handler fweight_handler) {
    parser->fweight_handler = fweight_handler;
    return READSTAT_OK;
}

readstat_error_t readstat_set_open_handler(readstat_parser_t *parser, readstat_open_handler open_handler) {
    parser->io->open = open_handler;
    return READSTAT_OK;
}

readstat_error_t readstat_set_close_handler(readstat_parser_t *parser, readstat_close_handler close_handler) {
    parser->io->close = close_handler;
    return READSTAT_OK;
}

readstat_error_t readstat_set_seek_handler(readstat_parser_t *parser, readstat_seek_handler seek_handler) {
    parser->io->seek = seek_handler;
    return READSTAT_OK;
}

readstat_error_t readstat_set_read_handler(readstat_parser_t *parser, readstat_read_handler read_handler) {
    parser->io->read = read_handler;
    return READSTAT_OK;
}

readstat_error_t readstat_set_update_handler(readstat_parser_t *parser, readstat_update_handler update_handler) {
    parser->io->update = update_handler;
    return READSTAT_OK;
}

readstat_error_t readstat_set_io_ctx(readstat_parser_t *parser, void *io_ctx) {
    if (!parser->io->external_io)
        free(parser->io->io_ctx);

    parser->io->io_ctx = io_ctx;
    parser->io->external_io = 1;

    return READSTAT_OK;
}

readstat_error_t readstat_set_file_character_encoding(readstat_parser_t *parser, const char *encoding) {
    parser->input_encoding = encoding;
    return READSTAT_OK;
}

readstat_error_t readstat_set_handler_character_encoding(readstat_parser_t *parser, const char *encoding) {
    parser->output_encoding = encoding;
    return READSTAT_OK;
}

readstat_error_t readstat_set_row_limit(readstat_parser_t *parser, long row_limit) {
    parser->row_limit = row_limit;
    return READSTAT_OK;
}
