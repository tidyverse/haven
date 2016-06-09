#define R_NO_REMAP
#include <R.h>
#include <Rinternals.h>
#include <stdbool.h>

// Scalar operators -------------------------------------------------------

// IEEE 754 defines binary64 as
// * 1  bit : sign
// * 11 bits: exponent
// * 52 bits: significand
//
// R stores the value "1954" in the last 32 bits: this payload marks
// the value as a NA, not a regular NaN.
//
// (Note that this discussion like most discussion of FP on the web, assumes
// a big-endian architecture - in little endian the sign bit is the last
// bit)

typedef union {
  double value;           // 8 bytes
  char byte[8];           // 8 * 1 bytes
} ieee_double;


#ifdef WORDS_BIGENDIAN
// First two bytes are sign & expoonent
// Last four bytes are 1954
const int TAG_BYTE = 3;
#else
const int TAG_BYTE = 4;
#endif

double make_tagged_na(char x) {
  ieee_double y;

  y.value = NA_REAL;
  y.byte[TAG_BYTE] = x;

  return y.value;
}

char tagged_na_value(double x) {
  ieee_double y;
  y.value = x;

  return y.byte[TAG_BYTE];
}

char first_char(SEXP x) {
  if (TYPEOF(x) != CHARSXP)
    return '\0';

  if (x == NA_STRING)
    return '\0';

  return CHAR(x)[0];
}

// Vectorised wrappers -----------------------------------------------------

SEXP tagged_na_(SEXP x) {
  if (TYPEOF(x) != STRSXP)
    Rf_errorcall(R_NilValue, "`x` must be a character vector");

  int n = Rf_length(x);
  SEXP out = PROTECT(Rf_allocVector(REALSXP, n));

  for (int i = 0; i < n; ++i) {
    char xi = first_char(STRING_ELT(x, i));
    REAL(out)[i] = make_tagged_na(xi);
  }

  UNPROTECT(1);
  return out;
}

SEXP na_tag_(SEXP x) {
  if (TYPEOF(x) != REALSXP)
    Rf_errorcall(R_NilValue, "`x` must be a double vector");

  int n = Rf_length(x);
  SEXP out = PROTECT(Rf_allocVector(STRSXP, n));

  for (int i = 0; i < n; ++i) {
    double xi = REAL(x)[i];

    if (!isnan(xi)) {
      SET_STRING_ELT(out, i, NA_STRING);
    } else {
      char tag = tagged_na_value(xi);
      if (tag == '\0') {
        SET_STRING_ELT(out, i, NA_STRING);
      } else {
        SET_STRING_ELT(out, i, Rf_mkCharLenCE(&tag, 1, CE_UTF8));
      }
    }
  }

  UNPROTECT(1);
  return out;
}

SEXP falses(int n) {
  SEXP out = PROTECT(Rf_allocVector(LGLSXP, n));
  for (int i = 0; i < n; ++i)
    LOGICAL(out)[i] = 0;

  UNPROTECT(1);
  return out;
}

SEXP is_tagged_na_(SEXP x, SEXP tag_) {
  if (TYPEOF(x) != REALSXP) {
    return falses(Rf_length(x));
  }

  bool has_tag;
  char check_tag;
  if (TYPEOF(tag_) == NILSXP) {
    has_tag = false;
    check_tag = '\0';
  } else if (TYPEOF(tag_) == STRSXP) {
    if (Rf_length(tag_) != 1)
      Rf_errorcall(R_NilValue, "`tag` must be a character vector of length 1");
    has_tag = true;
    check_tag = first_char(STRING_ELT(tag_, 0));
  } else {
    Rf_errorcall(R_NilValue, "`tag` must be NULL or a character vector");
  }

  int n = Rf_length(x);
  SEXP out = PROTECT(Rf_allocVector(LGLSXP, n));

  for (int i = 0; i < n; ++i) {
    double xi = REAL(x)[i];

    if (!isnan(xi)) {
      LOGICAL(out)[i] = false;
    } else {
      char tag = tagged_na_value(xi);
      if (tag == '\0') {
        LOGICAL(out)[i] = false;
      } else {
        if (has_tag) {
          LOGICAL(out)[i] = tag == check_tag;
        } else {
          LOGICAL(out)[i] = true;
        }
      }
    }
  }

  UNPROTECT(1);
  return out;
}
