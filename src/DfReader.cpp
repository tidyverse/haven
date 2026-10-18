#include <memory>
#include <string>

#include "DfReader.h"
#include "DfReaderInput.h"
#include "haven_types.h"
#include "readstat.h"

#include "cpp11/integers.hpp"
#include "cpp11/list.hpp"
#include "cpp11/sexp.hpp"

// Parser wrappers -------------------------------------------------------------

namespace {

int dfreader_open(const char* path, void* io_ctx) {
  return ((DfReaderInput*)io_ctx)->open(io_ctx);
}
int dfreader_close(void* io_ctx) {
  return ((DfReaderInput*)io_ctx)->close(io_ctx);
}
readstat_off_t
dfreader_seek(readstat_off_t offset, readstat_io_flags_t whence, void* io_ctx) {
  return ((DfReaderInput*)io_ctx)->seek(offset, whence, io_ctx);
}
ssize_t dfreader_read(void* buf, size_t nbyte, void* io_ctx) {
  return ((DfReaderInput*)io_ctx)->read(buf, nbyte, io_ctx);
}
readstat_error_t dfreader_update(
    long file_size,
    readstat_progress_handler progress_handler,
    void* user_ctx,
    void* io_ctx) {
  return READSTAT_OK;
}

int dfreader_metadata(readstat_metadata_t* metadata, void* ctx) {
  ((DfReader*)ctx)
      ->setInfo(
          readstat_get_row_count(metadata), readstat_get_var_count(metadata));
  ((DfReader*)ctx)->setMetadata(readstat_get_file_label(metadata));
  ((DfReader*)ctx)
      ->setTimestamp(
          readstat_get_creation_time(metadata),
          readstat_get_modified_time(metadata));
  return 0;
}

int dfreader_note(int note_index, const char* note, void* ctx) {
  ((DfReader*)ctx)->setNote(note_index, note);
  return 0;
}

int dfreader_variable(
    int index,
    readstat_variable_t* variable,
    const char* val_labels,
    void* ctx) {
  return ((DfReader*)ctx)->createVariable(index, variable, val_labels);
}
int dfreader_value(
    int obs_index,
    readstat_variable_t* variable,
    readstat_value_t value,
    void* ctx) {
  // Check for user interrupts every 10,000 rows or cols
  if ((obs_index + 1) % 10000 == 0 || (variable->index + 1) % 10000 == 0)
    cpp11::check_user_interrupt();

  ((DfReader*)ctx)->setValue(obs_index, variable, value);
  return 0;
}
int dfreader_value_label(
    const char* val_labels,
    readstat_value_t value,
    const char* label,
    void* ctx) {
  ((DfReader*)ctx)->setValueLabels(val_labels, value, label);
  return 0;
}

void print_error(const char* error_message, void* ctx) {
  Rprintf("%s\n", error_message);
}

} // namespace

readstat_parser_t* haven_init_parser() {
  readstat_parser_t* parser = readstat_parser_init();
  readstat_set_metadata_handler(parser, dfreader_metadata);
  readstat_set_note_handler(parser, dfreader_note);
  readstat_set_variable_handler(parser, dfreader_variable);
  readstat_set_value_handler(parser, dfreader_value);
  readstat_set_value_label_handler(parser, dfreader_value_label);
  readstat_set_error_handler(parser, print_error);
  return parser;
}

void haven_init_io(readstat_parser_t* parser, DfReaderInput& builder_input) {
  readstat_set_open_handler(parser, dfreader_open);
  readstat_set_close_handler(parser, dfreader_close);
  readstat_set_seek_handler(parser, dfreader_seek);
  readstat_set_read_handler(parser, dfreader_read);
  readstat_set_update_handler(parser, dfreader_update);
  readstat_set_io_ctx(parser, (void*)&builder_input);
  // Always set the encoding (NULL clears it) since the parser is reused
  // across the catalog and data file parses.
  readstat_set_file_character_encoding(
      parser,
      builder_input.encoding == "" ? NULL : builder_input.encoding.c_str());
}

void haven_set_row_limit(readstat_parser_t* parser, long n) {
  // readstat uses 0 to specify "all rows" but what we want is "minimal rows"
  readstat_set_row_limit(parser, n == 0 ? 1 : n);
}

template <FileExt ext>
void haven_parse(
    readstat_parser_t* parser,
    DfReaderInput& builder_input,
    DfReader* builder) {
  haven_init_io(parser, builder_input);

  readstat_error_t result;
  switch (ext) {
  // the path for readstat_parse_* is baked into the io context (DfReaderInput)
  case HAVEN_SAS7BDAT:
    result = readstat_parse_sas7bdat(parser, "", builder);
    break;
  case HAVEN_SAS7BCAT:
    result = readstat_parse_sas7bcat(parser, "", builder);
    break;
  case HAVEN_XPT:
    result = readstat_parse_xport(parser, "", builder);
    break;
  case HAVEN_DTA:
    result = readstat_parse_dta(parser, "", builder);
    break;
  case HAVEN_SAV:
    result = readstat_parse_sav(parser, "", builder);
    break;
  case HAVEN_POR:
    result = readstat_parse_por(parser, "", builder);
    break;
  default:
    result = READSTAT_ERROR_PARSE;
    break;
  }

  if (result != READSTAT_OK) {
    std::string source = builder_input.source();
    readstat_parser_free(parser);
    std::string msg(readstat_error_message(result));
    cpp11::stop("Failed to parse %s: %s.", source.c_str(), msg.c_str());
  }
}

// SAS catalogs typically don't record their encoding (header byte is 0), in
// which case ReadStat falls back to WINDOWS-1252. Since catalogs are usually
// written alongside their data file, the data file's encoding is a better
// guess.
// TODO: once https://github.com/WizardMac/ReadStat/pull/404 is merged and
// vendored here, detect this via readstat_get_file_encoding() instead.
bool sas_encoding_unspecified(DfReaderInput& input) {
  const int SAS_ENCODING_OFFSET = 70;
  unsigned char code = 1;

  if (input.open(&input) != 0)
    return false;
  bool ok = input.seek(SAS_ENCODING_OFFSET, READSTAT_SEEK_SET, &input) ==
                SAS_ENCODING_OFFSET &&
            input.read(&code, 1, &input) == 1;
  input.seek(0, READSTAT_SEEK_SET, &input);
  input.close(&input);

  return ok && code == 0;
}

int sas_encoding_metadata(readstat_metadata_t* metadata, void* ctx) {
  const char* encoding = readstat_get_file_encoding(metadata);
  if (encoding != NULL) {
    *((std::string*)ctx) = encoding;
  }
  return READSTAT_HANDLER_ABORT;
}

std::string sas_file_encoding(DfReaderInput& input) {
  std::string encoding;

  readstat_parser_t* parser = readstat_parser_init();
  readstat_set_metadata_handler(parser, sas_encoding_metadata);
  haven_init_io(parser, input);
  readstat_parse_sas7bdat(parser, "", &encoding);
  readstat_parser_free(parser);

  return encoding;
}

std::unique_ptr<DfReaderInput>
make_input(SEXP spec, const std::string& encoding) {
  switch (TYPEOF(spec)) {
  case STRSXP:
    return std::unique_ptr<DfReaderInput>(
        new DfReaderInputFile(spec, encoding));
  case RAWSXP:
    return std::unique_ptr<DfReaderInput>(new DfReaderInputRaw(spec, encoding));
  default:
    cpp11::stop("This kind of input is not handled.");
  }
}

template <FileExt ext>
cpp11::list df_parse(
    SEXP spec,
    cpp11::integers cols_skip,
    const long& n_max = -1,
    const long& rows_skip = 0,
    const std::string& encoding = "",
    const bool& user_na = false,
    SEXP catalog_spec = R_NilValue,
    const std::string& catalog_encoding = "") {
  DfReader builder(ext, user_na);
  builder.skipCols(cols_skip);
  builder.setQuiet(n_max == 0);

  readstat_parser_t* parser = haven_init_parser();
  haven_set_row_limit(parser, n_max);
  readstat_set_row_offset(parser, rows_skip);

  std::unique_ptr<DfReaderInput> input = make_input(spec, encoding);

  if (ext == HAVEN_SAS7BDAT && catalog_spec != R_NilValue) {
    std::unique_ptr<DfReaderInput> cat_input =
        make_input(catalog_spec, catalog_encoding);
    if (catalog_encoding == "" && sas_encoding_unspecified(*cat_input)) {
      cat_input->encoding = sas_file_encoding(*input);
    }
    haven_parse<HAVEN_SAS7BCAT>(parser, *cat_input, &builder);
  }

  haven_parse<ext>(parser, *input, &builder);
  readstat_parser_free(parser);

  if (n_max >= 0) {
    builder.limitRows(n_max); // must enforce n_max = 0
  }

  return builder.output();
}

// # nocov start

struct ReaderOptions {
  std::string encoding = "";
  std::string catalog_encoding = "";
  SEXP catalog_spec = R_NilValue;
  bool user_na = false;
  cpp11::integers cols_skip = cpp11::integers();
  long n_max = 0;
  long rows_skip = 0;

  ReaderOptions(cpp11::list opts) {
    SEXP x;
    if ((x = opts["encoding"]) != R_NilValue)
      encoding = cpp11::as_cpp<std::string>(x);
    if ((x = opts["catalog_encoding"]) != R_NilValue)
      catalog_encoding = cpp11::as_cpp<std::string>(x);
    if ((x = opts["catalog_file"]) != R_NilValue)
      catalog_spec = x;
    if ((x = opts["user_na"]) != R_NilValue)
      user_na = cpp11::as_cpp<bool>(x);
    if ((x = opts["cols_skip"]) != R_NilValue)
      cols_skip = cpp11::as_cpp<cpp11::integers>(x);
    if ((x = opts["n_max"]) != R_NilValue)
      n_max = cpp11::as_cpp<long>(x);
    if ((x = opts["rows_skip"]) != R_NilValue)
      rows_skip = cpp11::as_cpp<long>(x);
  }
};

[[cpp11::register]]
cpp11::list read(SEXP spec, cpp11::list options) {
  ReaderOptions opts(options);
  std::string vendor = cpp11::as_cpp<std::string>(options["vendor"]);

  if (vendor == "sas7bdat") {
    return df_parse<HAVEN_SAS7BDAT>(
        spec,
        opts.cols_skip,
        opts.n_max,
        opts.rows_skip,
        opts.encoding,
        false,
        opts.catalog_spec,
        opts.catalog_encoding);
  } else if (vendor == "xpt") {
    return df_parse<HAVEN_XPT>(
        spec, opts.cols_skip, opts.n_max, opts.rows_skip);
  } else if (vendor == "dta") {
    return df_parse<HAVEN_DTA>(
        spec, opts.cols_skip, opts.n_max, opts.rows_skip, opts.encoding);
  } else if (vendor == "sav") {
    return df_parse<HAVEN_SAV>(
        spec,
        opts.cols_skip,
        opts.n_max,
        opts.rows_skip,
        opts.encoding,
        opts.user_na);
  } else if (vendor == "por") {
    return df_parse<HAVEN_POR>(
        spec,
        opts.cols_skip,
        opts.n_max,
        opts.rows_skip,
        opts.encoding,
        opts.user_na);
  } else {
    cpp11::stop("Unknown vendor: %s", vendor.c_str());
  }
}

// # nocov end
