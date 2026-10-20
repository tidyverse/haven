#ifndef HAVEN_READER_INPUT_H
#define HAVEN_READER_INPUT_H

#include <fstream>
#include <sstream>
#include <string>

#include "readstat.h"

#include "cpp11/raws.hpp"
#include "cpp11/strings.hpp"

// IO handling -----------------------------------------------------------

class ReaderInput {
public:
  virtual ~ReaderInput() {};

  virtual int open() = 0;
  virtual int close() = 0;
  virtual readstat_off_t
  seek(readstat_off_t offset, readstat_io_flags_t whence) = 0;
  virtual ssize_t read(void* buf, size_t nbyte) = 0;
  virtual readstat_off_t tell() = 0;
  virtual std::string
  source() const = 0; // human readable description of input source
  std::string encoding;
};

template <typename Stream> class ReaderInputStream : public ReaderInput {
protected:
  Stream file_;
  // Track the position ourselves because tellg() is too slow to call per row
  readstat_off_t pos_ = 0;

public:
  readstat_off_t seek(readstat_off_t offset, readstat_io_flags_t whence) {
    std::ios_base::seekdir dir;
    switch (whence) {
    case READSTAT_SEEK_SET:
      dir = file_.beg;
      break;
    case READSTAT_SEEK_CUR:
      dir = file_.cur;
      break;
    case READSTAT_SEEK_END:
    default:
      dir = file_.end;
      break;
    }

    file_.seekg(offset, dir);
    pos_ = file_.tellg(); // returns -1 if failed
    return pos_;
  }

  ssize_t read(void* buf, size_t nbyte) {
    file_.read((char*)buf, nbyte);
    if (!file_.good() && !file_.eof()) {
      return -1;
    }
    if (pos_ >= 0) {
      pos_ += file_.gcount();
    }
    return file_.gcount();
  }

  readstat_off_t tell() { return pos_; }
};

class ReaderInputFile : public ReaderInputStream<std::ifstream> {
  std::string filename_;

public:
  ReaderInputFile(cpp11::strings path, std::string encoding = "") {
    filename_ = std::string(Rf_translateChar(path[0]));
    this->encoding = encoding;
  }

  std::string source() const { return filename_; }

  int open() {
    file_.open(filename_.c_str(), std::ifstream::binary);
    return file_.is_open() ? 0 : -1;
  }

  int close() {
    file_.close();
    return file_.is_open() ? -1 : 0;
  }
};

class ReaderInputRaw : public ReaderInputStream<std::istringstream> {
public:
  ReaderInputRaw(cpp11::raws raw_data, std::string encoding = "") {
    std::string string_data((char*)RAW(raw_data), Rf_length(raw_data));
    file_.str(string_data);
    this->encoding = encoding;
  }

  std::string source() const { return "file"; }

  int open() { return 0; }

  int close() { return 0; }
};

#endif
