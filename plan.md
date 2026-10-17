# Plan: S7 metadata in a single `haven_meta` attribute

Tackles #710, #712, #718, #719, and #799 by replacing haven's scattered metadata attributes (`format.sas`, `format.spss`, `format.stata`, `display_width`, `width`, `notes`, `creation_timestamp`, `modified_timestamp`) with a single `haven_meta` attribute that holds an S7 object.

## Design

### Classes

Two abstract parents, no common root. A root would have no properties, and the only thing it would buy us (checking "is this haven metadata?") is better handled by the setter, which has to distinguish column from data frame metadata anyway.

```
meta_var (abstract): format
├─ meta_var_sas:   informat, width
├─ meta_var_spss:  display_width, width, measure, alignment
└─ meta_var_stata: type

meta_df (abstract): created, modified
├─ meta_df_sas:   table_name, xpt_version
├─ meta_df_spss:  notes, mr_sets
└─ meta_df_stata: notes, version
```

* Every property is `NULL | <type>` with default `NULL`, so a field is only "present" when the file provided it.
* Validators enforce: `measure` in nominal/ordinal/scale; `alignment` in left/right/center; Stata `type` in byte/int/long/float/double/strL; `width`/`display_width` positive integers; `xpt_version` in 5/8; Stata `version` in the range `write_dta()` supports.
* `label`, `labels`, `na_values`, and `na_range` stay as plain attributes (shared with the labelled ecosystem).

Classes are per vendor, not per file type. Within a vendor the file types differ in which fields they can carry, not in what the fields mean, and converting between them is the main use case (e.g. `read_sas()` on a sas7bdat then `write_xpt()`). Per-file-type classes would force a conversion step for that and duplicate every shared field. Per-format support is documented in a table instead (see below). The one field whose meaning differs by file type is the SAS version, so it's named `xpt_version` and only populated by `read_xpt()`.

### Accessors

* `meta(x, vendor = NULL)`: returns the `haven_meta` attribute, or one synthesised from legacy attributes, or `NULL`. If `vendor` is supplied, returns an empty object of the right class when absent and errors if the metadata belongs to another vendor. Column vs data frame class chosen via `is.data.frame(x)`.
* `` `meta<-`(x, vendor = NULL, value) ``: `value` must be `NULL` or a `meta_df` (for data frames) / `meta_var` (otherwise); if `vendor` supplied, must match.
* Enables `meta(x)@format`, `meta(df)@created`, `meta(x, "sas")@informat <- "DATE9."`.
* `zap_meta(x)`: removes `haven_meta` and all legacy attributes from a vector or every column of a data frame (and the data frame itself).

### Readers

C++ (`DfReader.cpp`) stops setting legacy attributes and instead returns plain lists of fields (column- and file-level); R converts them to S7 objects. Benchmark a 10,000-column file; if construction is too slow, add an internal fast path that skips validation.

### Writers

R wrappers flatten each column's metadata into a simple per-column list passed to C++, using only fields from the matching vendor's class (via `S7_inherits()`), plus shared `meta_df` fields (`created`) for any vendor. Every field that a reader populates and ReadStat can write must be honoured, so `read_*() |> write_*()` round-trips. New defaults: `write_xpt(name, version)` and `write_dta(version)` come from `meta_df_*` when present. `write_sav()` gains a `label` argument.

### Legacy compatibility

`meta()` falls back to building an object from legacy attributes (`format.*`, `display_width`, `width`, `notes`, `*_timestamp`), so old RDS files and packages like xportr/metatools that set `format.sas`/`width` keep working with no writer-side special-casing. `haven_meta` wins if both are present. Plan to keep the fallback indefinitely (document it).

### Deprecations

`zap_formats()` and `zap_widths()` are deprecated (2.6.0) in favour of `zap_meta()`. Their behaviour is unchanged: they only remove the legacy attributes and don't touch `haven_meta`.

### Documentation

* `?meta`: accessor, setter, the `meta_var`/`meta_df` parents, legacy fallback, and `zap_meta()` cross-link. Links to each vendor's class topic (`?meta_sas`, `?meta_spss`, `?meta_stata`).
* One topic per vendor documenting the column and data frame classes together: `?meta_sas` (`meta_var_sas`, `meta_df_sas`), `?meta_spss`, `?meta_stata`. Each has a table of fields × which reader populates it and which writer honours it (e.g. sas7bdat vs xpt; sav vs por), since support differs by file type.
* `read_*()`/`write_*()` docs get a short paragraph linking to the relevant `?meta_*` topic instead of describing attributes inline.
* Reorganise the pkgdown reference index, dropping the "Remove attributes" section so each zapper lives next to the thing it removes:
  * New "Metadata" section: `meta`, `meta_sas`, `meta_spss`, `meta_stata`, `zap_meta`.
  * "Labelled vectors" gains `zap_label` and `zap_labels`.
  * "Tagged missing values" is renamed to "Missing values" and gains `zap_missing` and `zap_empty`.
  * New "Deprecated" section at the end: `zap_formats`, `zap_widths`, `write_sas`.

### Out of scope

SAS sort information (#719) and Stata characteristics other than notes (ReadStat doesn't parse them). Header-only reading (#710) is a separate function, done later.

## Todo

### Setup

- [x] Add S7 to Imports.
- [x] Add `^plan\.md$` to `.Rbuildignore`.

### Classes and accessors (`R/meta.R`)

- [x] Define `meta_var`, `meta_df` (abstract) and the six vendor classes with validators.
- [x] Implement `meta()` and `meta<-`, including the `vendor` argument.
- [x] Implement `legacy_meta()` fallback.
- [x] Register S7 methods in `.onLoad()` (`S7::methods_register()`).
- [x] Tests in `tests/testthat/test-meta.R`: construction, validation errors (snapshots), accessor/setter with and without `vendor`, vendor mismatch error, legacy fallback, precedence.

### Zapping

- [x] Add `zap_meta()` with tests.
- [x] Deprecate `zap_formats()` and `zap_widths()` (badge, `deprecate_warn("2.6.0", ...)`, Old/New examples, silence existing tests with `rlang::local_options(lifecycle_verbosity = "quiet")`, snapshot deprecation tests).

### Readers

- [x] `DfReader.cpp`: replace legacy attribute setting with plain field lists; capture new fields (SAS informat/width, SPSS width/measure/alignment, Stata storage type, SAS table name, xpt version, Stata version, SPSS mr_sets).
- [x] Build S7 objects in C++ by filling in prototypes from `meta_proto()` (replaces the planned R post-processing, which was too slow).
- [x] Benchmark wide-file reading; add fast path if needed.
- [x] Update existing tests that check legacy attributes.
- [x] Remove `zap_file_metadata()`; roundtrip helpers use `zap_meta()`.

### Writers

- [x] R-side flattening of `haven_meta` (with legacy fallback) into per-column lists; vendor filtering.
- [x] `DfWriter.cpp`: consume the flattened lists instead of reading legacy attributes directly.
- [x] Honour new fields: SAS informat/width, SPSS measure/alignment/width, Stata storage type, notes (sav and dta, #712), `created`.
- [x] `write_xpt(name, version)` and `write_dta(version)` defaults from metadata.
- [x] `write_sav(label)` argument.
- [x] Round-trip tests per format using `helper-roundtrip.R`; sas7bdat → xpt test; cross-vendor test (SAS metadata ignored by `write_dta()`, `created` kept).

### vctrs

- [ ] Carry `haven_meta` through `vec_ptype2()`/`vec_cast()` for `haven_labelled` and `haven_labelled_spss` (first input wins), with tests.

### Documentation and release notes

- [ ] Roxygen for `?meta` (linking to each class topic), `?meta_sas`, `?meta_spss`, `?meta_stata` (support tables), `?zap_meta`.
- [ ] Update `read_*()`/`write_*()` docs to link to metadata topics.
- [x] Reorganise `_pkgdown.yml` (Metadata, Labelled vectors, Missing values, Deprecated sections as above); run `devtools::document()` and `pkgdown::check_pkgdown()`. (Done with Zapping; Metadata section currently only has `zap_meta`.)
- [ ] Add `meta`, `meta_sas`, `meta_spss`, `meta_stata` to the Metadata section of `_pkgdown.yml`.
- [ ] NEWS bullets: new metadata system, new fields, legacy attributes no longer set by readers, `write_sav(label)`, `zap_meta()`, deprecations.
- [ ] Run `air format .` and `devtools::check()`.
- [ ] Coordinate with xportr/metatools maintainers (revdep check).

## Open questions

- Should readers have an option to skip metadata entirely for speed?
