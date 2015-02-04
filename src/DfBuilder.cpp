#include <Rcpp.h>
using namespace Rcpp;

extern "C" {
  #include "readstat.h"
}

class DfBuilder {
  int nrows_, ncols_;
  List output_;

public:
  DfBuilder(): nrows_(0), ncols_(0) {
    output_ = List::create();
  }

  int info(int obs_count, int var_count) {
    nrows_ = obs_count;
    ncols_ = var_count;

    Rcout << nrows_ << "/" << ncols_ << "\n";

    output_ = List(ncols_);
    return 0;
  }

  List output() {
    return output_;
  }

};

int dfbuilder_info(int obs_count, int var_count, void *ctx) {
  return ((DfBuilder*) ctx)->info(obs_count, var_count);
}


// [[Rcpp::export]]
List sas7bdat_df(std::string filename) {
  DfBuilder* builder = new DfBuilder();

  int result = parse_sas7bdat(
    filename.c_str(),
    builder,
    dfbuilder_info,
    NULL,
    NULL
  );

  List output = builder->output();
  delete builder;

  if (result != 0) {
    stop("Failed to parse %s (%i)", filename, result);
  }

  return output;
}
