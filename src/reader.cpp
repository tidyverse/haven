#include <algorithm>
#include <memory>
#include <string>

#include "haven_types.h"
#include "reader.h"
#include "reader_input.h"
#include "readstat.h"

#include "cpp11/integers.hpp"
#include "cpp11/list.hpp"
#include "cpp11/sexp.hpp"

// Parser wrappers -------------------------------------------------------------

namespace {

int reader_open(const char* path, void* io_ctx) {
  return ((ReaderInput*)io_ctx)->open();
}
int reader_close(void* io_ctx) { return ((ReaderInput*)io_ctx)->close(); }
readstat_off_t
reader_seek(readstat_off_t offset, readstat_io_flags_t whence, void* io_ctx) {
  return ((ReaderInput*)io_ctx)->seek(offset, whence);
}
ssize_t reader_read(void* buf, size_t nbyte, void* io_ctx) {
  return ((ReaderInput*)io_ctx)->read(buf, nbyte);
}
readstat_error_t reader_update(
    long file_size,
    readstat_progress_handler progress_handler,
    void* user_ctx,
    void* io_ctx) {
  return READSTAT_OK;
}

int reader_metadata(readstat_metadata_t* metadata, void* ctx) {
  ((Reader*)ctx)
      ->setInfo(
          readstat_get_row_count(metadata), readstat_get_var_count(metadata));
  ((Reader*)ctx)->setMetadata(readstat_get_file_label(metadata));
  ((Reader*)ctx)
      ->setTimestamp(
          readstat_get_creation_time(metadata),
          readstat_get_modified_time(metadata));
  return 0;
}

int reader_note(int note_index, const char* note, void* ctx) {
  ((Reader*)ctx)->setNote(note_index, note);
  return 0;
}

int reader_variable(
    int index,
    readstat_variable_t* variable,
    const char* val_labels,
    void* ctx) {
  return ((Reader*)ctx)->createVariable(index, variable, val_labels);
}
int reader_value(
    int obs_index,
    readstat_variable_t* variable,
    readstat_value_t value,
    void* ctx) {
  // Check for user interrupts every 10,000 rows or cols
  if ((obs_index + 1) % 10000 == 0 || (variable->index + 1) % 10000 == 0)
    cpp11::check_user_interrupt();

  ((Reader*)ctx)->setValue(obs_index, variable, value);
  return 0;
}
int reader_value_label(
    const char* val_labels,
    readstat_value_t value,
    const char* label,
    void* ctx) {
  ((Reader*)ctx)->setValueLabels(val_labels, value, label);
  return 0;
}

void print_error(const char* error_message, void* ctx) {
  Rprintf("%s\n", error_message);
}

struct ParserDeleter {
  void operator()(readstat_parser_t* parser) const {
    readstat_parser_free(parser);
  }
};
typedef std::unique_ptr<readstat_parser_t, ParserDeleter> Parser;

Parser haven_init_parser(long n_max) {
  Parser parser(readstat_parser_init());
  readstat_set_metadata_handler(parser.get(), reader_metadata);
  readstat_set_note_handler(parser.get(), reader_note);
  readstat_set_variable_handler(parser.get(), reader_variable);
  // With n_max = 0 we only need the header. n_max = -1 means read all rows
  if (n_max != 0) {
    readstat_set_value_handler(parser.get(), reader_value);
  }
  if (n_max > 0) {
    readstat_set_row_limit(parser.get(), n_max);
  }
  readstat_set_value_label_handler(parser.get(), reader_value_label);
  readstat_set_error_handler(parser.get(), print_error);
  return parser;
}

void haven_init_io(readstat_parser_t* parser, ReaderInput& builder_input) {
  readstat_set_open_handler(parser, reader_open);
  readstat_set_close_handler(parser, reader_close);
  readstat_set_seek_handler(parser, reader_seek);
  readstat_set_read_handler(parser, reader_read);
  readstat_set_update_handler(parser, reader_update);
  readstat_set_io_ctx(parser, (void*)&builder_input);
  // Always set the encoding (NULL clears it) since the parser is reused
  // across the catalog and data file parses.
  readstat_set_file_character_encoding(
      parser,
      builder_input.encoding == "" ? NULL : builder_input.encoding.c_str());
}

void haven_parse(
    FileExt ext,
    readstat_parser_t* parser,
    ReaderInput& builder_input,
    Reader* builder) {
  haven_init_io(parser, builder_input);

  readstat_error_t result;
  switch (ext) {
  // the path for readstat_parse_* is baked into the io context (ReaderInput)
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
bool sas_encoding_unspecified(ReaderInput& input) {
  const int SAS_ENCODING_OFFSET = 70;
  unsigned char code = 1;

  if (input.open() != 0)
    return false;
  bool ok = input.seek(SAS_ENCODING_OFFSET, READSTAT_SEEK_SET) ==
                SAS_ENCODING_OFFSET &&
            input.read(&code, 1) == 1;
  input.seek(0, READSTAT_SEEK_SET);
  input.close();

  return ok && code == 0;
}

int sas_encoding_metadata(readstat_metadata_t* metadata, void* ctx) {
  const char* encoding = readstat_get_file_encoding(metadata);
  if (encoding != NULL) {
    *((std::string*)ctx) = encoding;
  }
  return READSTAT_HANDLER_ABORT;
}

std::string sas_file_encoding(ReaderInput& input) {
  std::string encoding;

  Parser parser(readstat_parser_init());
  readstat_set_metadata_handler(parser.get(), sas_encoding_metadata);
  haven_init_io(parser.get(), input);
  readstat_parse_sas7bdat(parser.get(), "", &encoding);

  return encoding;
}

std::unique_ptr<ReaderInput>
make_input(SEXP spec, const std::string& encoding) {
  switch (TYPEOF(spec)) {
  case STRSXP:
    return std::unique_ptr<ReaderInput>(new ReaderInputFile(spec, encoding));
  case RAWSXP:
    return std::unique_ptr<ReaderInput>(new ReaderInputRaw(spec, encoding));
  default:
    cpp11::stop("This kind of input is not handled.");
  }
}

// # nocov start

struct ReaderOptions {
  std::string vendor;
  std::string encoding = "";
  std::string catalog_encoding = "";
  SEXP catalog_spec = R_NilValue;
  bool user_na = false;
  cpp11::integers cols_skip = cpp11::integers();
  long n_max = 0;
  long rows_skip = 0;

  ReaderOptions(cpp11::list opts) {
    vendor = cpp11::as_cpp<std::string>(opts["vendor"]);

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

FileExt parseExt(const std::string& vendor) {
  if (vendor == "sas7bdat")
    return HAVEN_SAS7BDAT;
  if (vendor == "xpt")
    return HAVEN_XPT;
  if (vendor == "dta")
    return HAVEN_DTA;
  if (vendor == "sav")
    return HAVEN_SAV;
  if (vendor == "por")
    return HAVEN_POR;
  cpp11::stop("Unknown vendor: %s", vendor.c_str());
}

// # nocov end

cpp11::list df_parse(FileExt ext, SEXP spec, const ReaderOptions& opts) {
  // Not every option applies to every format
  bool user_na = opts.user_na && extVendor(ext) == HAVEN_SPSS;
  std::string encoding = ext == HAVEN_XPT ? "" : opts.encoding;
  bool has_catalog = ext == HAVEN_SAS7BDAT && opts.catalog_spec != R_NilValue;

  Reader builder(ext, user_na);
  builder.skipCols(opts.cols_skip);
  builder.setQuiet(opts.n_max == 0);
  builder.setMaxRows(opts.n_max);

  Parser parser = haven_init_parser(opts.n_max);
  readstat_set_row_offset(parser.get(), opts.rows_skip);

  std::unique_ptr<ReaderInput> input = make_input(spec, encoding);

  if (has_catalog) {
    std::unique_ptr<ReaderInput> cat_input =
        make_input(opts.catalog_spec, opts.catalog_encoding);
    if (opts.catalog_encoding == "" && sas_encoding_unspecified(*cat_input)) {
      cat_input->encoding = sas_file_encoding(*input);
    }
    haven_parse(HAVEN_SAS7BCAT, parser.get(), *cat_input, &builder);
  }

  haven_parse(ext, parser.get(), *input, &builder);

  return builder.output();
}

} // namespace

[[cpp11::register]]
cpp11::list read(SEXP spec, cpp11::list options) {
  ReaderOptions opts(options);
  return df_parse(parseExt(opts.vendor), spec, opts);
}
