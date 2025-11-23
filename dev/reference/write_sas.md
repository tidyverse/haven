# Write SAS files

**\[deprecated\]**

`write_sas()` creates sas7bdat files. Unfortunately the SAS file format
is complex and undocumented, so `write_sas()` is unreliable and in most
cases SAS will not read files that it produces.

[`write_xpt()`](https://haven.tidyverse.org/dev/reference/read_xpt.md)
writes files in the open SAS transport format, which has limitations but
will be reliably read by SAS.

## Usage

``` r
write_sas(data, path)
```

## Arguments

- data:

  Data frame to write.

- path:

  Path to file where the data will be written.
