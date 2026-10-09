
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <iconv.h>

#include "../readstat.h"
#include "../readstat_writer.h"
#include "readstat_sas.h"
#include "readstat_sas_rle.h"
#include "readstat_xport.h"
#include "readstat_xport_parse_format.h"

/* This writer produces files laid out the way SAS 9.4 lays them out, following
 * the sas7bdat specification (Shotwell & Yu, as extended by
 * FredHutch/sas7bdat-specification) and the FredHutch/sas7bdat writer, whose
 * output is verified against SAS. Fields whose purpose is unknown are set to
 * the constant values that SAS writes.
 *
 * File layout (uncompressed):
 *
 *   header | META page ... | MIX page (last metadata + first rows) | DATA page ...
 *
 * File layout (RLE compressed):
 *
 *   header | META page ... (metadata subheaders, then row subheaders) | COMP page | META page ...
 */

#define SAS7BDAT_PAGE_NUMBER_MASK        0xFEDCBA98u
#define SAS7BDAT_MAX_SUBHEADER_SIZE      32740
#define SAS7BDAT_MAX_ATTRS_SUBHEADER_SIZE 24588
#define SAS7BDAT_MAX_NAMES_PER_SUBHEADER 4089
#define SAS7BDAT_COUNTS_VECTOR_COUNT     12

#define SAS7BDAT_ROW_SIZE_UNCOMPRESSED_FLAGS   0x00223011
#define SAS7BDAT_ROW_SIZE_COMPRESSED_FLAGS     0x0022703A

typedef struct sas7bdat_subheader_s {
    uint32_t        signature;
    char           *data;
    size_t          len;
    size_t          capacity;
    unsigned char   comp;
    unsigned char   type;
    int             is_row_data;
    int             text_index;
    int             page;       /* 1-based layout page */
    int             position;   /* 1-based position within the page */
} sas7bdat_subheader_t;

typedef struct sas7bdat_page_s {
    int64_t         first;      /* index of the first subheader on the page */
    int             count;      /* subheaders on the page, excluding the terminator */
    size_t          data_bytes; /* total length of subheader data on the page */
} sas7bdat_page_t;

typedef struct sas7bdat_write_ctx_s {
    sas_header_info_t      *hinfo;
    int                     u64;
    int                     compressed;
    size_t                  sig_len;
    size_t                  footer_len;
    size_t                  int_len;
    size_t                  page_size;
    size_t                  phs;    /* page header size */
    size_t                  sps;    /* subheader pointer size */

    /* Physical row layout: numeric columns first, then character columns */
    size_t                 *phys_offsets;
    size_t                  row_length;
    char                   *phys_row;

    /* Subheaders, in page order */
    sas7bdat_subheader_t  **subheaders;
    int64_t                 subheader_count;
    int64_t                 subheader_capacity;
    int64_t                 meta_subheader_count;

    /* Page layout */
    sas7bdat_page_t        *pages;
    int                     page_count;
    int                     page_capacity;
    int                     last_meta_page;     /* 1-based */

    /* Column text */
    int                     text_subheader_count;
    sas7bdat_subheader_t   *current_text;
    sas_text_ref_t          label_ref;
    sas_text_ref_t          dstype_ref;
    sas_text_ref_t          compression_ref;
    sas_text_ref_t          blank_ref;
    sas_text_ref_t          creator_ref;
    sas_text_ref_t         *name_refs;
    sas_text_ref_t         *label_refs;
    sas_text_ref_t         *format_refs;
    sas_text_ref_t         *informat_refs;
    uint16_t               *format_widths;
    uint16_t               *format_digits;
    uint16_t               *informat_widths;
    uint16_t               *informat_digits;
    size_t                  max_name_len;
    size_t                  max_label_len;
    size_t                  name_len_sum;
    size_t                  hash_payload_size;

    sas7bdat_subheader_t   *row_size_subheader;
    sas7bdat_subheader_t   *counts_subheader;

    /* Row placement (uncompressed) */
    int64_t                 max_rows_on_mix_page;
    int64_t                 rows_per_data_page;
    int64_t                 total_pages;

    /* Streaming state (uncompressed) */
    char                   *page;
    int64_t                 physical_page;
    int                     page_is_mix;
    int                     page_layout_index;
    size_t                  page_row_offset;
    int64_t                 page_rows;
    int64_t                 page_max_rows;
} sas7bdat_write_ctx_t;

/* Little helpers ------------------------------------------------------- */

#define OFF(ctx, off32, off64) ((ctx)->u64 ? (off64) : (off32))

static void w2(char *p, uint16_t v) { memcpy(p, &v, sizeof(v)); }
static void w4(char *p, uint32_t v) { memcpy(p, &v, sizeof(v)); }
static void w8(char *p, uint64_t v) { memcpy(p, &v, sizeof(v)); }

static void wint(sas7bdat_write_ctx_t *ctx, char *p, uint64_t v) {
    if (ctx->u64) {
        w8(p, v);
    } else {
        w4(p, (uint32_t)v);
    }
}

static void wsig(sas7bdat_write_ctx_t *ctx, char *p, uint32_t signature) {
    if (ctx->u64) {
        if (signature >= 0xFF000000) {
            w8(p, (uint64_t)(int64_t)(int32_t)signature);
        } else {
            w8(p, signature);
        }
    } else {
        w4(p, signature);
    }
}

static void wref(char *p, sas_text_ref_t ref) {
    w2(p, ref.index);
    w2(p+2, ref.offset);
    w2(p+4, ref.length);
}

static void wloc(sas7bdat_write_ctx_t *ctx, char *p, uint64_t page, uint64_t position) {
    wint(ctx, p, page);
    wint(ctx, p + ctx->int_len, position);
}

static size_t align8(size_t v) {
    return (v + 7) / 8 * 8;
}

static size_t sas7bdat_variable_width(readstat_type_t type, size_t user_width) {
    if (type == READSTAT_TYPE_STRING) {
        return user_width;
    }
    return 8;
}

static size_t sas7bdat_variable_size_overhead(sas7bdat_write_ctx_t *ctx) {
    /* signature + 8-byte size field + footer */
    return ctx->sig_len + 8 + ctx->footer_len;
}

/* Subheaders ----------------------------------------------------------- */

static sas7bdat_subheader_t *sas7bdat_subheader_init(uint32_t signature, size_t len,
        size_t capacity, unsigned char type) {
    sas7bdat_subheader_t *subheader = calloc(1, sizeof(sas7bdat_subheader_t));
    if (subheader == NULL)
        return NULL;
    if (capacity < len)
        capacity = len;
    subheader->signature = signature;
    subheader->len = len;
    subheader->capacity = capacity;
    subheader->type = type;
    subheader->data = calloc(1, capacity ? capacity : 1);
    if (subheader->data == NULL) {
        free(subheader);
        return NULL;
    }
    return subheader;
}

static void sas7bdat_subheader_free(sas7bdat_subheader_t *subheader) {
    if (!subheader)
        return;
    free(subheader->data);
    free(subheader);
}

/* Sets the 2-byte "size of data" field that variable-length subheaders carry
 * right after their signature. */
static void sas7bdat_subheader_set_size_field(sas7bdat_write_ctx_t *ctx, sas7bdat_subheader_t *subheader) {
    w2(&subheader->data[ctx->sig_len], subheader->len - ctx->sig_len - ctx->footer_len);
}

static uint16_t sas7bdat_subheader_size_field(sas7bdat_write_ctx_t *ctx, sas7bdat_subheader_t *subheader) {
    return subheader->len - ctx->sig_len - ctx->footer_len;
}

/* Page layout ---------------------------------------------------------- */

static sas7bdat_page_t *sas7bdat_current_page(sas7bdat_write_ctx_t *ctx) {
    if (ctx->page_count == 0)
        return NULL;
    return &ctx->pages[ctx->page_count-1];
}

/* Bytes on a fresh page available to the data of a single subheader
 * (after the page header, its pointer, and the terminating pointer). */
static size_t sas7bdat_new_page_capacity(sas7bdat_write_ctx_t *ctx) {
    return ctx->page_size - ctx->phs - 2 * ctx->sps;
}

/* Bytes still available on the current page for the data of one more
 * subheader, keeping room for its pointer and the terminating pointer. */
static size_t sas7bdat_free_for_new_subheader(sas7bdat_write_ctx_t *ctx) {
    sas7bdat_page_t *page = sas7bdat_current_page(ctx);
    if (page == NULL)
        return sas7bdat_new_page_capacity(ctx);
    size_t used = ctx->phs + (page->count + 2) * ctx->sps + page->data_bytes;
    if (used >= ctx->page_size)
        return 0;
    return ctx->page_size - used;
}

static readstat_error_t sas7bdat_layout_new_page(sas7bdat_write_ctx_t *ctx) {
    if (ctx->page_count == ctx->page_capacity) {
        int new_capacity = ctx->page_capacity ? 2 * ctx->page_capacity : 8;
        sas7bdat_page_t *pages = realloc(ctx->pages, new_capacity * sizeof(sas7bdat_page_t));
        if (pages == NULL)
            return READSTAT_ERROR_MALLOC;
        ctx->pages = pages;
        ctx->page_capacity = new_capacity;
    }
    sas7bdat_page_t *page = &ctx->pages[ctx->page_count++];
    page->first = ctx->subheader_count;
    page->count = 0;
    page->data_bytes = 0;
    return READSTAT_OK;
}

static readstat_error_t sas7bdat_layout_add(sas7bdat_write_ctx_t *ctx, sas7bdat_subheader_t *subheader) {
    readstat_error_t retval = READSTAT_OK;

    if (sas7bdat_current_page(ctx) == NULL || sas7bdat_free_for_new_subheader(ctx) < subheader->len) {
        if ((retval = sas7bdat_layout_new_page(ctx)) != READSTAT_OK)
            return retval;
    }
    if (sas7bdat_free_for_new_subheader(ctx) < subheader->len)
        return READSTAT_ERROR_ROW_IS_TOO_WIDE_FOR_PAGE;

    if (ctx->subheader_count == ctx->subheader_capacity) {
        int64_t new_capacity = ctx->subheader_capacity ? 2 * ctx->subheader_capacity : 32;
        sas7bdat_subheader_t **subheaders = realloc(ctx->subheaders,
                new_capacity * sizeof(sas7bdat_subheader_t *));
        if (subheaders == NULL)
            return READSTAT_ERROR_MALLOC;
        ctx->subheaders = subheaders;
        ctx->subheader_capacity = new_capacity;
    }

    sas7bdat_page_t *page = sas7bdat_current_page(ctx);
    ctx->subheaders[ctx->subheader_count++] = subheader;
    page->count++;
    page->data_bytes += subheader->len;
    subheader->page = ctx->page_count;
    subheader->position = page->count;

    if (!subheader->is_row_data)
        ctx->meta_subheader_count = ctx->subheader_count;

    return READSTAT_OK;
}

/* Column text ---------------------------------------------------------- */

static readstat_error_t sas7bdat_text_close(sas7bdat_write_ctx_t *ctx) {
    readstat_error_t retval = READSTAT_OK;
    if (ctx->current_text) {
        sas7bdat_subheader_set_size_field(ctx, ctx->current_text);
        retval = sas7bdat_layout_add(ctx, ctx->current_text);
        if (retval == READSTAT_OK)
            ctx->current_text = NULL;
    }
    return retval;
}

static readstat_error_t sas7bdat_text_add_bytes(sas7bdat_write_ctx_t *ctx,
        const char *bytes, size_t len, sas_text_ref_t *out_ref) {
    readstat_error_t retval = READSTAT_OK;
    sas_text_ref_t ref = { 0 };
    size_t padded_len = (len + 3) / 4 * 4;
    size_t overhead = sas7bdat_variable_size_overhead(ctx);

    if (len == 0 || len > UINT16_MAX) {
        if (len > UINT16_MAX)
            retval = READSTAT_ERROR_STRING_VALUE_IS_TOO_LONG;
        goto cleanup;
    }

    if (ctx->current_text && ctx->current_text->len + padded_len > ctx->current_text->capacity) {
        if ((retval = sas7bdat_text_close(ctx)) != READSTAT_OK)
            goto cleanup;
    }

    if (ctx->current_text == NULL) {
        size_t needed = overhead + padded_len;
        size_t available = sas7bdat_free_for_new_subheader(ctx);
        if (available < needed)
            available = sas7bdat_new_page_capacity(ctx);
        if (available > SAS7BDAT_MAX_SUBHEADER_SIZE)
            available = SAS7BDAT_MAX_SUBHEADER_SIZE;
        if (available < needed) {
            retval = READSTAT_ERROR_STRING_VALUE_IS_TOO_LONG;
            goto cleanup;
        }
        available = available / 4 * 4;

        sas7bdat_subheader_t *subheader = sas7bdat_subheader_init(SAS_SUBHEADER_SIGNATURE_COLUMN_TEXT,
                overhead, available, 1);
        if (subheader == NULL) {
            retval = READSTAT_ERROR_MALLOC;
            goto cleanup;
        }
        subheader->text_index = ctx->text_subheader_count++;
        ctx->current_text = subheader;
    }

    sas7bdat_subheader_t *text = ctx->current_text;
    size_t data_offset = text->len - ctx->footer_len;
    ref.index = text->text_index;
    ref.offset = data_offset - ctx->sig_len;
    ref.length = len;
    memcpy(&text->data[data_offset], bytes, len);
    text->len += padded_len;

cleanup:
    if (out_ref)
        *out_ref = ref;
    return retval;
}

static readstat_error_t sas7bdat_text_add(sas7bdat_write_ctx_t *ctx, const char *string, sas_text_ref_t *out_ref) {
    return sas7bdat_text_add_bytes(ctx, string, string ? strlen(string) : 0, out_ref);
}

/* Formats -------------------------------------------------------------- */

/* Splits a format such as "DATETIME19.2" into name "DATETIME", width 19 and
 * 2 digits, the way SAS stores it. If the string can't be split without
 * losing information, it is stored verbatim as the format name. */
static void sas7bdat_split_format(const char *format, char *name, size_t name_len,
        uint16_t *width, uint16_t *digits) {
    name[0] = '\0';
    *width = 0;
    *digits = 0;
    if (format == NULL || format[0] == '\0')
        return;

    xport_format_t parsed;
    if (xport_parse_format(format, strlen(format), &parsed, NULL, NULL) == READSTAT_OK &&
            parsed.width >= 0 && parsed.width <= UINT16_MAX &&
            parsed.decimals >= 0 && parsed.decimals <= UINT16_MAX) {
        char rebuilt[64];
        size_t len = snprintf(rebuilt, sizeof(rebuilt), "%s", parsed.name);
        if (parsed.width && len < sizeof(rebuilt))
            len += snprintf(rebuilt + len, sizeof(rebuilt) - len, "%d", parsed.width);
        if (parsed.decimals && len < sizeof(rebuilt))
            len += snprintf(rebuilt + len, sizeof(rebuilt) - len, ".%d", parsed.decimals);
        if (len < sizeof(rebuilt) && strcmp(rebuilt, format) == 0) {
            snprintf(name, name_len, "%s", parsed.name);
            *width = parsed.width;
            *digits = parsed.decimals;
            return;
        }
    }
    snprintf(name, name_len, "%s", format);
}

/* Column name hash table ----------------------------------------------- */

static int sas7bdat_is_prime(int n) {
    int i;
    if (n < 2)
        return 0;
    for (i=2; i*i<=n; i++) {
        if (n % i == 0)
            return 0;
    }
    return 1;
}

static int sas7bdat_smallest_prime_not_less_than(int n) {
    if (n <= 2)
        return 2;
    if (n % 2 == 0)
        n++;
    while (!sas7bdat_is_prime(n))
        n += 2;
    return n;
}

static uint32_t sas7bdat_hash_name(const char *name) {
    size_t len = strlen(name);
    size_t i = 0;
    uint32_t hash = 0;
    while (i < len) {
        uint32_t word = 0;
        int j;
        for (j=0; j<4 && i+j<len; j++) {
            unsigned char c = (unsigned char)name[i+j];
            if (c < 0x80)
                c = toupper(c);
            word |= ((uint32_t)c) << (8*j);
        }
        hash ^= word;
        i += 4;
    }
    return hash;
}

static int16_t *sas7bdat_build_hash_table(readstat_writer_t *writer, int *out_bucket_count) {
    int ncols = writer->variables_count;
    int bucket_count = sas7bdat_smallest_prime_not_less_than((int)(ncols * 1.3));
    int16_t *table = calloc(bucket_count, sizeof(int16_t));
    int i, j;
    if (table == NULL)
        return NULL;

    for (i=0; i<ncols; i++) {
        readstat_variable_t *variable = readstat_get_variable(writer, i);
        uint32_t hash = sas7bdat_hash_name(variable->name);
        uint32_t probe = (hash / bucket_count) % bucket_count;
        uint32_t bucket = hash % bucket_count;
        if (probe == 0)
            probe = 1;
        for (j=0; j<bucket_count; j++) {
            int16_t current = table[bucket];
            if (current == 0) {
                table[bucket] = i + 1;
                break;
            }
            if (current > 0) {
                table[bucket] = -current;
            }
            bucket = (bucket + probe) % bucket_count;
        }
    }
    *out_bucket_count = bucket_count;
    return table;
}

/* Metadata subheaders -------------------------------------------------- */

static readstat_error_t sas7bdat_add_fixed_subheader(sas7bdat_write_ctx_t *ctx, uint32_t signature,
        size_t len, sas7bdat_subheader_t **out_subheader) {
    readstat_error_t retval = READSTAT_OK;
    sas7bdat_subheader_t *subheader = sas7bdat_subheader_init(signature, len, len, 0);
    if (subheader == NULL)
        return READSTAT_ERROR_MALLOC;
    if ((retval = sas7bdat_layout_add(ctx, subheader)) != READSTAT_OK) {
        sas7bdat_subheader_free(subheader);
        return retval;
    }
    if (out_subheader)
        *out_subheader = subheader;
    return retval;
}

static readstat_error_t sas7bdat_add_column_text(readstat_writer_t *writer, sas7bdat_write_ctx_t *ctx) {
    readstat_error_t retval = READSTAT_OK;
    int i;
    char first_entry[4] = { 0, 0, 0, 20 }; /* the last byte is the encoding (UTF-8) */
    const char blank[9] = "        ";

    ctx->name_refs = calloc(writer->variables_count, sizeof(sas_text_ref_t));
    ctx->label_refs = calloc(writer->variables_count, sizeof(sas_text_ref_t));
    ctx->format_refs = calloc(writer->variables_count, sizeof(sas_text_ref_t));
    ctx->informat_refs = calloc(writer->variables_count, sizeof(sas_text_ref_t));
    ctx->format_widths = calloc(writer->variables_count, sizeof(uint16_t));
    ctx->format_digits = calloc(writer->variables_count, sizeof(uint16_t));
    ctx->informat_widths = calloc(writer->variables_count, sizeof(uint16_t));
    ctx->informat_digits = calloc(writer->variables_count, sizeof(uint16_t));
    if (!ctx->name_refs || !ctx->label_refs || !ctx->format_refs || !ctx->informat_refs ||
            !ctx->format_widths || !ctx->format_digits || !ctx->informat_widths || !ctx->informat_digits) {
        retval = READSTAT_ERROR_MALLOC;
        goto cleanup;
    }

    if ((retval = sas7bdat_text_add_bytes(ctx, first_entry, sizeof(first_entry), NULL)) != READSTAT_OK)
        goto cleanup;
    if (ctx->compressed) {
        if ((retval = sas7bdat_text_add(ctx, SAS_COMPRESSION_SIGNATURE_RLE, &ctx->compression_ref)) != READSTAT_OK)
            goto cleanup;
    }
    if ((retval = sas7bdat_text_add(ctx, blank, &ctx->blank_ref)) != READSTAT_OK)
        goto cleanup;
    if ((retval = sas7bdat_text_add(ctx, blank, &ctx->dstype_ref)) != READSTAT_OK)
        goto cleanup;
    if ((retval = sas7bdat_text_add(ctx, "DATASTEP", &ctx->creator_ref)) != READSTAT_OK)
        goto cleanup;
    if ((retval = sas7bdat_text_add(ctx, writer->file_label, &ctx->label_ref)) != READSTAT_OK)
        goto cleanup;

    for (i=0; i<writer->variables_count; i++) {
        readstat_variable_t *variable = readstat_get_variable(writer, i);
        char format_name[64];
        char informat_name[64];
        size_t name_len = strlen(variable->name);
        size_t label_len = strlen(variable->label);

        sas7bdat_split_format(variable->format, format_name, sizeof(format_name),
                &ctx->format_widths[i], &ctx->format_digits[i]);
        sas7bdat_split_format(variable->informat, informat_name, sizeof(informat_name),
                &ctx->informat_widths[i], &ctx->informat_digits[i]);

        if ((retval = sas7bdat_text_add(ctx, variable->name, &ctx->name_refs[i])) != READSTAT_OK)
            goto cleanup;
        if ((retval = sas7bdat_text_add(ctx, variable->label, &ctx->label_refs[i])) != READSTAT_OK)
            goto cleanup;
        if ((retval = sas7bdat_text_add(ctx, informat_name, &ctx->informat_refs[i])) != READSTAT_OK)
            goto cleanup;
        if ((retval = sas7bdat_text_add(ctx, format_name, &ctx->format_refs[i])) != READSTAT_OK)
            goto cleanup;

        ctx->name_len_sum += name_len;
        if (name_len > ctx->max_name_len)
            ctx->max_name_len = name_len;
        if (label_len > ctx->max_label_len)
            ctx->max_label_len = label_len;
    }

    retval = sas7bdat_text_close(ctx);

cleanup:
    return retval;
}

/* Chooses the size of the next variable-length subheader: fill what is left
 * on the current page if that is enough for `min_size`, otherwise use a
 * full page. */
static size_t sas7bdat_next_subheader_capacity(sas7bdat_write_ctx_t *ctx, size_t min_size, size_t max_size) {
    size_t available = sas7bdat_free_for_new_subheader(ctx);
    if (available < min_size)
        available = sas7bdat_new_page_capacity(ctx);
    if (available > max_size)
        available = max_size;
    return available;
}

static readstat_error_t sas7bdat_add_column_names(readstat_writer_t *writer, sas7bdat_write_ctx_t *ctx) {
    readstat_error_t retval = READSTAT_OK;
    size_t overhead = sas7bdat_variable_size_overhead(ctx);
    int offset = 0;

    while (offset < writer->variables_count) {
        size_t capacity = sas7bdat_next_subheader_capacity(ctx, overhead + 8, SAS7BDAT_MAX_SUBHEADER_SIZE);
        int count = (capacity - overhead) / 8;
        int i;
        if (count > SAS7BDAT_MAX_NAMES_PER_SUBHEADER)
            count = SAS7BDAT_MAX_NAMES_PER_SUBHEADER;
        if (count > writer->variables_count - offset)
            count = writer->variables_count - offset;
        if (count == 0)
            return READSTAT_ERROR_ROW_IS_TOO_WIDE_FOR_PAGE;

        sas7bdat_subheader_t *subheader = sas7bdat_subheader_init(SAS_SUBHEADER_SIGNATURE_COLUMN_NAME,
                overhead + 8 * count, 0, 1);
        if (subheader == NULL)
            return READSTAT_ERROR_MALLOC;
        sas7bdat_subheader_set_size_field(ctx, subheader);
        for (i=0; i<count; i++) {
            wref(&subheader->data[ctx->sig_len + 8 + 8 * i], ctx->name_refs[offset + i]);
        }
        if ((retval = sas7bdat_layout_add(ctx, subheader)) != READSTAT_OK) {
            sas7bdat_subheader_free(subheader);
            return retval;
        }
        offset += count;
    }
    return retval;
}

static int sas7bdat_name_needs_quoting(const char *name) {
    size_t i;
    if (!isalpha((unsigned char)name[0]))
        return 1;
    for (i=1; name[i]; i++) {
        if (!isalnum((unsigned char)name[i]) && name[i] != '_')
            return 1;
    }
    return 0;
}

static readstat_error_t sas7bdat_add_column_attrs(readstat_writer_t *writer, sas7bdat_write_ctx_t *ctx) {
    readstat_error_t retval = READSTAT_OK;
    size_t overhead = sas7bdat_variable_size_overhead(ctx);
    size_t entry_size = ctx->int_len + 8;
    int offset = 0;

    while (offset < writer->variables_count) {
        size_t capacity = sas7bdat_next_subheader_capacity(ctx, overhead + entry_size,
                SAS7BDAT_MAX_ATTRS_SUBHEADER_SIZE);
        int count = (capacity - overhead) / entry_size;
        int i;
        if (count > writer->variables_count - offset)
            count = writer->variables_count - offset;
        if (count == 0)
            return READSTAT_ERROR_ROW_IS_TOO_WIDE_FOR_PAGE;

        sas7bdat_subheader_t *subheader = sas7bdat_subheader_init(SAS_SUBHEADER_SIGNATURE_COLUMN_ATTRS,
                overhead + entry_size * count, 0, 1);
        if (subheader == NULL)
            return READSTAT_ERROR_MALLOC;
        sas7bdat_subheader_set_size_field(ctx, subheader);
        for (i=0; i<count; i++) {
            readstat_variable_t *variable = readstat_get_variable(writer, offset + i);
            char *entry = &subheader->data[ctx->sig_len + 8 + entry_size * i];
            uint32_t width = sas7bdat_variable_width(variable->type, variable->storage_width);
            uint16_t name_flag = 0;
            if (sas7bdat_name_needs_quoting(variable->name)) {
                name_flag = 0x0C00;
            } else if (strlen(variable->name) <= 8) {
                name_flag = 0x0400;
            } else {
                name_flag = 0x0800;
            }
            wint(ctx, entry, ctx->phys_offsets[offset + i]);
            w4(entry + ctx->int_len, width);
            w2(entry + ctx->int_len + 4, name_flag);
            entry[ctx->int_len + 6] = (variable->type == READSTAT_TYPE_STRING ?
                    SAS_COLUMN_TYPE_CHR : SAS_COLUMN_TYPE_NUM);
        }
        if ((retval = sas7bdat_layout_add(ctx, subheader)) != READSTAT_OK) {
            sas7bdat_subheader_free(subheader);
            return retval;
        }
        offset += count;
    }
    return retval;
}

static readstat_error_t sas7bdat_add_column_hash_table(readstat_writer_t *writer, sas7bdat_write_ctx_t *ctx) {
    readstat_error_t retval = READSTAT_OK;
    int bucket_count = 0;
    int first_bucket = 0;
    int16_t *table = NULL;

    /* SAS only writes the hash table when there is more than one column */
    if (writer->variables_count < 2)
        return READSTAT_OK;

    table = sas7bdat_build_hash_table(writer, &bucket_count);
    if (table == NULL)
        return READSTAT_ERROR_MALLOC;

    while (first_bucket < bucket_count) {
        /* The first subheader carries the table header; later ones only buckets */
        size_t header_len = first_bucket == 0 ? OFF(ctx, 26, 30) : 8;
        size_t overhead = ctx->sig_len + header_len + ctx->footer_len;
        size_t capacity = sas7bdat_next_subheader_capacity(ctx, overhead + 2, SAS7BDAT_MAX_SUBHEADER_SIZE);
        int count = (capacity - overhead) / 2;
        int i;
        if (count > bucket_count - first_bucket)
            count = bucket_count - first_bucket;
        if (count == 0) {
            retval = READSTAT_ERROR_ROW_IS_TOO_WIDE_FOR_PAGE;
            goto cleanup;
        }

        sas7bdat_subheader_t *subheader = sas7bdat_subheader_init(SAS_SUBHEADER_SIGNATURE_COLUMN_LIST,
                overhead + 2 * count, 0, 1);
        if (subheader == NULL) {
            retval = READSTAT_ERROR_MALLOC;
            goto cleanup;
        }
        sas7bdat_subheader_set_size_field(ctx, subheader);
        uint16_t size_field = sas7bdat_subheader_size_field(ctx, subheader);
        if (first_bucket == 0) {
            char *d = subheader->data;
            wint(ctx, &d[OFF(ctx, 12, 16)], size_field - 8);
            w2(&d[OFF(ctx, 16, 24)], writer->variables_count);
            w2(&d[OFF(ctx, 18, 26)], bucket_count);
            w2(&d[OFF(ctx, 20, 28)], 1);
            w2(&d[OFF(ctx, 22, 30)], writer->variables_count);
        }
        for (i=0; i<count; i++) {
            w2(&subheader->data[ctx->sig_len + header_len + 2 * i], (uint16_t)table[first_bucket + i]);
        }
        ctx->hash_payload_size += size_field - 8;

        if ((retval = sas7bdat_layout_add(ctx, subheader)) != READSTAT_OK) {
            sas7bdat_subheader_free(subheader);
            goto cleanup;
        }
        first_bucket += count;
    }

cleanup:
    free(table);
    return retval;
}

static readstat_error_t sas7bdat_add_column_formats(readstat_writer_t *writer, sas7bdat_write_ctx_t *ctx) {
    readstat_error_t retval = READSTAT_OK;
    int i;
    for (i=0; i<writer->variables_count; i++) {
        sas7bdat_subheader_t *subheader = NULL;
        if ((retval = sas7bdat_add_fixed_subheader(ctx, SAS_SUBHEADER_SIGNATURE_COLUMN_FORMAT,
                        OFF(ctx, 52, 64), &subheader)) != READSTAT_OK)
            return retval;
        char *d = subheader->data;
        w2(&d[OFF(ctx, 12, 24)], ctx->format_widths[i]);
        w2(&d[OFF(ctx, 14, 26)], ctx->format_digits[i]);
        w2(&d[OFF(ctx, 16, 28)], ctx->informat_widths[i]);
        w2(&d[OFF(ctx, 18, 30)], ctx->informat_digits[i]);
        wref(&d[OFF(ctx, 28, 40)], ctx->informat_refs[i]);
        wref(&d[OFF(ctx, 34, 46)], ctx->format_refs[i]);
        wref(&d[OFF(ctx, 40, 52)], ctx->label_refs[i]);
    }
    return retval;
}

static readstat_error_t sas7bdat_build_metadata(readstat_writer_t *writer, sas7bdat_write_ctx_t *ctx) {
    readstat_error_t retval = READSTAT_OK;
    sas7bdat_subheader_t *column_size = NULL;

    if ((retval = sas7bdat_add_fixed_subheader(ctx, SAS_SUBHEADER_SIGNATURE_ROW_SIZE,
                    OFF(ctx, 480, 808), &ctx->row_size_subheader)) != READSTAT_OK)
        goto cleanup;
    if ((retval = sas7bdat_add_fixed_subheader(ctx, SAS_SUBHEADER_SIGNATURE_COLUMN_SIZE,
                    OFF(ctx, 12, 24), &column_size)) != READSTAT_OK)
        goto cleanup;
    wint(ctx, &column_size->data[ctx->int_len], writer->variables_count);
    if ((retval = sas7bdat_add_fixed_subheader(ctx, SAS_SUBHEADER_SIGNATURE_COUNTS,
                    OFF(ctx, 304, 600), &ctx->counts_subheader)) != READSTAT_OK)
        goto cleanup;
    if ((retval = sas7bdat_add_column_text(writer, ctx)) != READSTAT_OK)
        goto cleanup;
    if ((retval = sas7bdat_add_column_names(writer, ctx)) != READSTAT_OK)
        goto cleanup;
    if ((retval = sas7bdat_add_column_attrs(writer, ctx)) != READSTAT_OK)
        goto cleanup;
    if ((retval = sas7bdat_add_column_hash_table(writer, ctx)) != READSTAT_OK)
        goto cleanup;
    if ((retval = sas7bdat_add_column_formats(writer, ctx)) != READSTAT_OK)
        goto cleanup;

    ctx->last_meta_page = ctx->page_count;

cleanup:
    return retval;
}

/* Row Size and Subheader Counts contents (need the finished layout) ---- */

static int64_t sas7bdat_rows_on_mix_page(readstat_writer_t *writer, sas7bdat_write_ctx_t *ctx) {
    if (writer->row_count < ctx->max_rows_on_mix_page)
        return writer->row_count;
    return ctx->max_rows_on_mix_page;
}

static int64_t sas7bdat_physical_page(sas7bdat_write_ctx_t *ctx, int layout_page) {
    /* Compressed files carry an extra page after the last metadata page */
    if (ctx->compressed && layout_page > ctx->last_meta_page)
        return layout_page + 1;
    return layout_page;
}

static void sas7bdat_fill_row_size_subheader(readstat_writer_t *writer, sas7bdat_write_ctx_t *ctx) {
    sas7bdat_subheader_t *subheader = ctx->row_size_subheader;
    char *d = subheader->data;
    size_t il = ctx->int_len;
    size_t len = subheader->len;
    int64_t i;
    int p;

    /* Count subheaders and locate the Column Format subheaders */
    int64_t total_subheaders = 0;
    int first_format_page = 0, second_format_page = 0;
    int64_t formats_on_first_page = 0, formats_on_second_page = 0;
    int first_format_position = 0;
    for (p=0; p<ctx->last_meta_page; p++) {
        sas7bdat_page_t *page = &ctx->pages[p];
        for (i=page->first; i<page->first+page->count; i++) {
            if (!ctx->subheaders[i]->is_row_data)
                total_subheaders++;
        }
        total_subheaders++; /* the terminating pointer */
    }
    for (i=0; i<ctx->meta_subheader_count; i++) {
        sas7bdat_subheader_t *sh = ctx->subheaders[i];
        if (sh->signature != SAS_SUBHEADER_SIGNATURE_COLUMN_FORMAT)
            continue;
        if (first_format_page == 0) {
            first_format_page = sh->page;
            first_format_position = sh->position;
        }
        if (sh->page == first_format_page) {
            formats_on_first_page++;
        } else if (second_format_page == 0 || sh->page == second_format_page) {
            second_format_page = sh->page;
            formats_on_second_page++;
        }
    }

    sas7bdat_subheader_t *last_meta = ctx->subheaders[ctx->meta_subheader_count-1];
    sas7bdat_page_t *last_meta_page = &ctx->pages[ctx->last_meta_page-1];

    wint(ctx, &d[1*il], ctx->compressed ? 0x131 : 0xF0);
    wint(ctx, &d[2*il], total_subheaders + 2);
    wint(ctx, &d[4*il], ctx->compressed ? SAS7BDAT_ROW_SIZE_COMPRESSED_FLAGS : SAS7BDAT_ROW_SIZE_UNCOMPRESSED_FLAGS);
    wint(ctx, &d[5*il], ctx->row_length);
    wint(ctx, &d[6*il], writer->row_count);
    wint(ctx, &d[7*il], 0); /* deleted rows */
    wint(ctx, &d[8*il], ctx->compressed ? 2 : 0);
    wint(ctx, &d[9*il], formats_on_first_page);
    wint(ctx, &d[10*il], formats_on_second_page);
    wint(ctx, &d[11*il], ctx->hash_payload_size);
    wint(ctx, &d[12*il], ctx->name_len_sum);
    wint(ctx, &d[13*il], ctx->page_size);
    wint(ctx, &d[15*il], ctx->compressed ? 1 : ctx->max_rows_on_mix_page);
    memset(&d[16*il], 0xFF, 2*il);

    /* The first page number */
    w4(&d[OFF(ctx, 220, 440)], ctx->hinfo->page_number_mask ^ 1);

    /* Subheader and row locations, as (page, block) pairs */
    char *loc = &d[OFF(ctx, 264, 512)];
    wloc(ctx, loc, 1, 2); /* Column Size subheader */
    loc += 2*il;
    wloc(ctx, loc, sas7bdat_physical_page(ctx, last_meta->page), last_meta->position);
    loc += 2*il;
    if (writer->row_count == 0) {
        wloc(ctx, loc, 0, 3);
        loc += 2*il;
        wloc(ctx, loc, 0, 3);
        loc += 2*il;
    } else if (ctx->compressed) {
        sas7bdat_subheader_t *first_row = ctx->subheaders[ctx->meta_subheader_count];
        sas7bdat_subheader_t *last_row = ctx->subheaders[ctx->subheader_count-1];
        wloc(ctx, loc, sas7bdat_physical_page(ctx, first_row->page), first_row->position);
        loc += 2*il;
        wloc(ctx, loc, sas7bdat_physical_page(ctx, last_row->page), last_row->position);
        loc += 2*il;
    } else {
        int64_t rows_on_mix_page = sas7bdat_rows_on_mix_page(writer, ctx);
        if (rows_on_mix_page == 0) {
            wloc(ctx, loc, ctx->last_meta_page + 1, 1);
        } else {
            wloc(ctx, loc, ctx->last_meta_page, last_meta_page->count + 2);
        }
        loc += 2*il;
        if (writer->row_count == rows_on_mix_page) {
            wloc(ctx, loc, ctx->last_meta_page, last_meta_page->count + 1 + rows_on_mix_page);
        } else {
            int64_t rows_on_data_pages = writer->row_count - rows_on_mix_page;
            int64_t last_index = rows_on_data_pages % ctx->rows_per_data_page;
            if (last_index == 0)
                last_index = ctx->rows_per_data_page;
            wloc(ctx, loc, ctx->total_pages, last_index);
        }
        loc += 2*il;
    }
    wloc(ctx, loc, first_format_page, first_format_position);

    /* Text references */
    sas_text_ref_t first_entry_ref = { .index = 0, .offset = 8, .length = 4 };
    wref(&d[len-136], first_entry_ref);
    wref(&d[len-130], ctx->label_ref);
    wref(&d[len-124], ctx->dstype_ref);
    wref(&d[len-118], ctx->compression_ref);
    wref(&d[len-112], ctx->blank_ref);
    wref(&d[len-106], ctx->creator_ref);

    w2(&d[len-64], 4);
    w2(&d[len-62], 1);
    w2(&d[len-60], ctx->text_subheader_count);
    w2(&d[len-58], ctx->max_name_len);
    w2(&d[len-56], ctx->max_label_len);
    w2(&d[len-42], ctx->compressed ? 0 : ctx->rows_per_data_page);
    wint(ctx, &d[OFF(ctx, 444, 776)], writer->row_count);
    d[len-13] = 1;
}

static void sas7bdat_fill_counts_subheader(sas7bdat_write_ctx_t *ctx) {
    sas7bdat_subheader_t *subheader = ctx->counts_subheader;
    char *d = subheader->data;
    size_t il = ctx->int_len;
    const uint32_t signatures[] = {
        SAS_SUBHEADER_SIGNATURE_COLUMN_ATTRS,
        SAS_SUBHEADER_SIGNATURE_COLUMN_TEXT,
        SAS_SUBHEADER_SIGNATURE_COLUMN_NAME,
        SAS_SUBHEADER_SIGNATURE_COLUMN_LIST,
        0xFFFFFFFB,
        0xFFFFFFFA,
        0xFFFFFFF9
    };
    const int tracked = sizeof(signatures)/sizeof(signatures[0]);
    struct { int first_page, first_position, last_page, last_position; } locations[7] = { { 0 } };
    uint16_t max_payload = 0;
    int types_present = 0;
    int64_t i;
    int k;

    for (i=0; i<ctx->meta_subheader_count; i++) {
        sas7bdat_subheader_t *sh = ctx->subheaders[i];
        if (sh->type == 1) {
            uint16_t payload = sas7bdat_subheader_size_field(ctx, sh);
            if (payload > max_payload)
                max_payload = payload;
        }
        for (k=0; k<tracked; k++) {
            if (sh->signature != signatures[k])
                continue;
            if (locations[k].first_page == 0) {
                locations[k].first_page = sh->page;
                locations[k].first_position = sh->position;
                types_present++;
            }
            locations[k].last_page = sh->page;
            locations[k].last_position = sh->position;
        }
    }

    wint(ctx, &d[1*il], max_payload);
    wint(ctx, &d[2*il], types_present);
    w2(&d[3*il], tracked);
    wint(ctx, &d[OFF(ctx, 56, 112)], 1804);

    char *vector = &d[OFF(ctx, 64, 120)];
    for (k=0; k<tracked; k++) {
        wsig(ctx, vector, signatures[k]);
        wint(ctx, vector + 1*il, locations[k].first_page);
        wint(ctx, vector + 2*il, locations[k].first_position);
        wint(ctx, vector + 3*il, locations[k].last_page);
        wint(ctx, vector + 4*il, locations[k].last_position);
        vector += 5*il;
    }
    /* The remaining vectors stay zero */
}

/* Page rendering ------------------------------------------------------- */

static void sas7bdat_write_pointer(sas7bdat_write_ctx_t *ctx, char *ptr, size_t offset, size_t len,
        unsigned char comp, unsigned char type) {
    wint(ctx, ptr, offset);
    wint(ctx, ptr + ctx->int_len, len);
    ptr[2*ctx->int_len] = comp;
    ptr[2*ctx->int_len+1] = type;
}

static void sas7bdat_set_page_number(sas7bdat_write_ctx_t *ctx, char *page, int64_t physical_page) {
    w4(page, ctx->hinfo->page_number_mask ^ (uint32_t)(physical_page + 1));
}

/* Writes the page header fields that depend on the number of rows */
static void sas7bdat_finish_page_header(sas7bdat_write_ctx_t *ctx, char *page, int16_t page_type,
        int subheader_count, size_t data_bytes, int64_t rows) {
    size_t used = ctx->phs + subheader_count * ctx->sps + data_bytes + rows * ctx->row_length + (rows + 7) / 8;
    uint64_t unused = used < ctx->page_size ? ctx->page_size - used : 0;
    wint(ctx, &page[OFF(ctx, 12, 24)], unused);
    w2(&page[ctx->phs-8], page_type);
    w2(&page[ctx->phs-6], subheader_count + rows);
    w2(&page[ctx->phs-4], subheader_count);
}

/* Renders the subheaders of a layout page into the buffer. Returns the
 * number of subheader pointers written (including the terminator). */
static int sas7bdat_render_layout_page(sas7bdat_write_ctx_t *ctx, int layout_page, int64_t physical_page,
        char *buffer) {
    sas7bdat_page_t *page = &ctx->pages[layout_page];
    size_t ptr_offset = ctx->phs;
    size_t data_offset = ctx->page_size;
    int64_t i;

    memset(buffer, 0, ctx->page_size);
    sas7bdat_set_page_number(ctx, buffer, physical_page);

    for (i=page->first; i<page->first+page->count; i++) {
        sas7bdat_subheader_t *sh = ctx->subheaders[i];
        data_offset -= sh->len;
        if (!sh->is_row_data)
            wsig(ctx, sh->data, sh->signature);
        sas7bdat_write_pointer(ctx, &buffer[ptr_offset], data_offset, sh->len, sh->comp, sh->type);
        memcpy(&buffer[data_offset], sh->data, sh->len);
        ptr_offset += ctx->sps;
    }
    /* The terminating pointer */
    sas7bdat_write_pointer(ctx, &buffer[ptr_offset], data_offset, 0, SAS_COMPRESSION_TRUNC,
            ctx->compressed ? 1 : 0);

    return page->count + 1;
}

static readstat_error_t sas7bdat_flush_page(readstat_writer_t *writer, sas7bdat_write_ctx_t *ctx) {
    int16_t page_type = SAS_PAGE_TYPE_DATA;
    int subheader_count = 0;
    size_t data_bytes = 0;
    if (ctx->page_is_mix) {
        sas7bdat_page_t *page = &ctx->pages[ctx->page_layout_index];
        page_type = SAS_PAGE_TYPE_MIX;
        subheader_count = page->count + 1;
        data_bytes = page->data_bytes;
    }
    sas7bdat_finish_page_header(ctx, ctx->page, page_type, subheader_count, data_bytes, ctx->page_rows);
    return readstat_write_bytes(writer, ctx->page, ctx->page_size);
}

/* The mostly empty page that SAS writes after the metadata of compressed files */
static readstat_error_t sas7bdat_write_comp_page(readstat_writer_t *writer, sas7bdat_write_ctx_t *ctx,
        int64_t physical_page) {
    char *buffer = ctx->page;
    sas7bdat_page_t *first_page = &ctx->pages[0];
    int64_t rows_on_first_page = 0;
    int64_t i;

    for (i=first_page->first; i<first_page->first+first_page->count; i++) {
        if (ctx->subheaders[i]->is_row_data)
            rows_on_first_page++;
    }

    memset(buffer, 0, ctx->page_size);
    sas7bdat_set_page_number(ctx, buffer, physical_page);
    wint(ctx, &buffer[OFF(ctx, 12, 24)], 1);
    w2(&buffer[ctx->phs-8], SAS_PAGE_TYPE_COMP);
    w2(&buffer[ctx->phs-6], ctx->total_pages);
    w2(&buffer[ctx->phs-4], 1);
    w2(&buffer[ctx->phs-2], ctx->page_size - ctx->phs - ctx->sps - 2);
    buffer[ctx->page_size-7] = rows_on_first_page & 0xFF;
    buffer[ctx->page_size-6] = 1;
    w2(&buffer[ctx->page_size-2], first_page->count);

    return readstat_write_bytes(writer, buffer, ctx->page_size);
}

/* Page size selection -------------------------------------------------- */

static size_t sas7bdat_longest_string(readstat_writer_t *writer) {
    size_t longest = strlen(writer->file_label);
    int i;
    for (i=0; i<writer->variables_count; i++) {
        readstat_variable_t *variable = readstat_get_variable(writer, i);
        size_t len;
        if ((len = strlen(variable->name)) > longest) longest = len;
        if ((len = strlen(variable->label)) > longest) longest = len;
        if ((len = strlen(variable->format)) > longest) longest = len;
        if ((len = strlen(variable->informat)) > longest) longest = len;
    }
    return longest;
}

static int sas7bdat_page_is_too_small(readstat_writer_t *writer, sas7bdat_write_ctx_t *ctx) {
    size_t capacity = sas7bdat_new_page_capacity(ctx);
    size_t overhead = sas7bdat_variable_size_overhead(ctx);

    if (capacity < OFF(ctx, 480, 808) || capacity < OFF(ctx, 304, 600))
        return 1;
    if (capacity < overhead + (sas7bdat_longest_string(writer) + 3) / 4 * 4)
        return 1;
    if (capacity < overhead + ctx->int_len + 8)
        return 1;
    if (ctx->compressed) {
        if (capacity < ctx->row_length)
            return 1;
    } else {
        if (ctx->page_size - ctx->phs < ctx->row_length + 1)
            return 1;
    }
    return 0;
}

/* Context -------------------------------------------------------------- */

static void sas7bdat_write_ctx_free(sas7bdat_write_ctx_t *ctx) {
    int64_t i;
    if (ctx == NULL)
        return;
    for (i=0; i<ctx->subheader_count; i++) {
        sas7bdat_subheader_free(ctx->subheaders[i]);
    }
    sas7bdat_subheader_free(ctx->current_text);
    free(ctx->subheaders);
    free(ctx->pages);
    free(ctx->phys_offsets);
    free(ctx->phys_row);
    free(ctx->name_refs);
    free(ctx->label_refs);
    free(ctx->format_refs);
    free(ctx->informat_refs);
    free(ctx->format_widths);
    free(ctx->format_digits);
    free(ctx->informat_widths);
    free(ctx->informat_digits);
    free(ctx->page);
    free(ctx->hinfo);
    free(ctx);
}

static readstat_error_t sas7bdat_write_ctx_init(readstat_writer_t *writer, sas7bdat_write_ctx_t **out_ctx) {
    readstat_error_t retval = READSTAT_OK;
    sas7bdat_write_ctx_t *ctx = calloc(1, sizeof(sas7bdat_write_ctx_t));
    int i;
    int has_numeric = 0;
    size_t offset = 0;

    if (ctx == NULL)
        return READSTAT_ERROR_MALLOC;

    ctx->hinfo = sas_header_info_init(writer, writer->is_64bit);
    if (ctx->hinfo == NULL) {
        retval = READSTAT_ERROR_MALLOC;
        goto cleanup;
    }
    ctx->hinfo->page_number_mask = SAS7BDAT_PAGE_NUMBER_MASK;
    ctx->u64 = ctx->hinfo->u64;
    ctx->compressed = (writer->compression == READSTAT_COMPRESS_ROWS);
    ctx->sig_len = ctx->u64 ? 8 : 4;
    ctx->int_len = ctx->u64 ? 8 : 4;
    ctx->footer_len = ctx->u64 ? 12 : 8;
    ctx->phs = ctx->hinfo->page_header_size;
    ctx->sps = ctx->hinfo->subheader_pointer_size;

    /* Physical row layout: SAS puts the numeric columns first so that they
     * are naturally aligned, then the character columns. */
    ctx->phys_offsets = calloc(writer->variables_count ? writer->variables_count : 1, sizeof(size_t));
    if (ctx->phys_offsets == NULL) {
        retval = READSTAT_ERROR_MALLOC;
        goto cleanup;
    }
    for (i=0; i<writer->variables_count; i++) {
        readstat_variable_t *variable = readstat_get_variable(writer, i);
        if (variable->type != READSTAT_TYPE_STRING) {
            has_numeric = 1;
            ctx->phys_offsets[i] = offset;
            offset += sas7bdat_variable_width(variable->type, variable->storage_width);
        }
    }
    for (i=0; i<writer->variables_count; i++) {
        readstat_variable_t *variable = readstat_get_variable(writer, i);
        if (variable->type == READSTAT_TYPE_STRING) {
            ctx->phys_offsets[i] = offset;
            offset += sas7bdat_variable_width(variable->type, variable->storage_width);
        }
    }
    if (has_numeric)
        offset = align8(offset);
    ctx->row_length = offset;

    if (ctx->row_length == 0) {
        retval = READSTAT_ERROR_TOO_FEW_COLUMNS;
        goto cleanup;
    }

    ctx->page_size = ctx->hinfo->page_size;
    while (sas7bdat_page_is_too_small(writer, ctx)) {
        if (ctx->page_size >= 0x40000000) {
            retval = READSTAT_ERROR_ROW_IS_TOO_WIDE_FOR_PAGE;
            goto cleanup;
        }
        ctx->page_size <<= 1;
    }
    ctx->hinfo->page_size = ctx->page_size;
    if (ctx->u64) {
        /* SAS uses the same size for the header and the pages in 64-bit files */
        ctx->hinfo->header_size = ctx->page_size;
    }

    ctx->phys_row = malloc(ctx->row_length);
    ctx->page = malloc(ctx->page_size);
    if (ctx->phys_row == NULL || ctx->page == NULL) {
        retval = READSTAT_ERROR_MALLOC;
        goto cleanup;
    }

    if ((retval = sas7bdat_build_metadata(writer, ctx)) != READSTAT_OK)
        goto cleanup;

    /* Row capacity of the mix page and of the data pages */
    sas7bdat_page_t *last_meta_page = &ctx->pages[ctx->last_meta_page-1];
    size_t row_start = align8(ctx->phs + (last_meta_page->count + 1) * ctx->sps);
    size_t bytes_for_rows = 0;
    if (row_start + last_meta_page->data_bytes < ctx->page_size)
        bytes_for_rows = ctx->page_size - last_meta_page->data_bytes - row_start;
    /* Each row also needs one bit in the deleted-row bitmap */
    ctx->max_rows_on_mix_page = (8 * bytes_for_rows) / (8 * ctx->row_length + 1);
    ctx->rows_per_data_page = (8 * (ctx->page_size - ctx->phs)) / (8 * ctx->row_length + 1);

cleanup:
    if (retval != READSTAT_OK) {
        sas7bdat_write_ctx_free(ctx);
        ctx = NULL;
    }
    *out_ctx = ctx;
    return retval;
}

/* Emission ------------------------------------------------------------- */

static readstat_error_t sas7bdat_emit_header(readstat_writer_t *writer, sas7bdat_write_ctx_t *ctx) {
    sas_header_start_t header_start = {
        .a2 = ctx->u64 ? SAS_ALIGNMENT_OFFSET_4 : SAS_ALIGNMENT_OFFSET_0,
        /* SAS pads the timestamps in 64-bit files; 32-bit files carry '2' here */
        .a1 = ctx->u64 ? SAS_ALIGNMENT_OFFSET_4 : 0x32,
        .endian = machine_is_little_endian() ? SAS_ENDIAN_LITTLE : SAS_ENDIAN_BIG,
        .file_format = SAS_FILE_FORMAT_UNIX,
        .encoding = 20, /* UTF-8 */
        .file_type = "SAS FILE",
        .file_info = "DATA    "
    };

    memcpy(&header_start.magic, sas7bdat_magic_number, sizeof(header_start.magic));

    ctx->hinfo->page_count = ctx->total_pages;

    return sas_write_header(writer, ctx->hinfo, header_start);
}

static readstat_error_t sas7bdat_begin_uncompressed(readstat_writer_t *writer, sas7bdat_write_ctx_t *ctx) {
    readstat_error_t retval = READSTAT_OK;
    int p;

    int64_t rows_on_mix_page = sas7bdat_rows_on_mix_page(writer, ctx);
    int64_t data_pages = 0;
    if (writer->row_count > rows_on_mix_page) {
        data_pages = (writer->row_count - rows_on_mix_page + ctx->rows_per_data_page - 1) / ctx->rows_per_data_page;
    }
    ctx->total_pages = ctx->page_count + data_pages;

    sas7bdat_fill_row_size_subheader(writer, ctx);
    sas7bdat_fill_counts_subheader(ctx);

    if ((retval = sas7bdat_emit_header(writer, ctx)) != READSTAT_OK)
        goto cleanup;

    /* Metadata pages, except the last one, which also holds rows */
    for (p=0; p<ctx->page_count-1; p++) {
        int subheader_count = sas7bdat_render_layout_page(ctx, p, p, ctx->page);
        sas7bdat_finish_page_header(ctx, ctx->page, SAS_PAGE_TYPE_META, subheader_count,
                ctx->pages[p].data_bytes, 0);
        if ((retval = readstat_write_bytes(writer, ctx->page, ctx->page_size)) != READSTAT_OK)
            goto cleanup;
    }

    /* The mix page stays in memory until its rows arrive */
    p = ctx->page_count - 1;
    int subheader_count = sas7bdat_render_layout_page(ctx, p, p, ctx->page);
    ctx->physical_page = p;
    ctx->page_is_mix = 1;
    ctx->page_layout_index = p;
    ctx->page_row_offset = align8(ctx->phs + subheader_count * ctx->sps);
    ctx->page_rows = 0;
    ctx->page_max_rows = ctx->max_rows_on_mix_page;

cleanup:
    return retval;
}

static readstat_error_t sas7bdat_end_compressed(readstat_writer_t *writer, sas7bdat_write_ctx_t *ctx) {
    readstat_error_t retval = READSTAT_OK;
    int p;

    ctx->total_pages = ctx->page_count + 1;

    sas7bdat_fill_row_size_subheader(writer, ctx);
    sas7bdat_fill_counts_subheader(ctx);

    if ((retval = sas7bdat_emit_header(writer, ctx)) != READSTAT_OK)
        goto cleanup;

    for (p=0; p<ctx->page_count; p++) {
        int64_t physical_page = sas7bdat_physical_page(ctx, p + 1) - 1;
        int subheader_count = sas7bdat_render_layout_page(ctx, p, physical_page, ctx->page);
        sas7bdat_finish_page_header(ctx, ctx->page, SAS_PAGE_TYPE_META, subheader_count,
                ctx->pages[p].data_bytes, 0);
        if ((retval = readstat_write_bytes(writer, ctx->page, ctx->page_size)) != READSTAT_OK)
            goto cleanup;
        if (p + 1 == ctx->last_meta_page) {
            if ((retval = sas7bdat_write_comp_page(writer, ctx, physical_page + 1)) != READSTAT_OK)
                goto cleanup;
        }
    }

cleanup:
    return retval;
}

/* Writer callbacks ----------------------------------------------------- */

static readstat_error_t sas7bdat_begin_data(void *writer_ctx) {
    readstat_writer_t *writer = (readstat_writer_t *)writer_ctx;
    readstat_error_t retval = READSTAT_OK;
    sas7bdat_write_ctx_t *ctx = NULL;

    if ((retval = sas7bdat_write_ctx_init(writer, &ctx)) != READSTAT_OK)
        goto cleanup;

    writer->module_ctx = ctx;

    if (!ctx->compressed) {
        retval = sas7bdat_begin_uncompressed(writer, ctx);
    }

cleanup:
    if (retval != READSTAT_OK && writer->module_ctx) {
        sas7bdat_write_ctx_free(writer->module_ctx);
        writer->module_ctx = NULL;
    }
    return retval;
}

static readstat_error_t sas7bdat_end_data(void *writer_ctx) {
    readstat_writer_t *writer = (readstat_writer_t *)writer_ctx;
    sas7bdat_write_ctx_t *ctx = (sas7bdat_write_ctx_t *)writer->module_ctx;

    if (ctx->compressed) {
        return sas7bdat_end_compressed(writer, ctx);
    }
    return sas7bdat_flush_page(writer, ctx);
}

static void sas7bdat_module_ctx_free(void *module_ctx) {
    sas7bdat_write_ctx_free(module_ctx);
}

static readstat_error_t sas7bdat_write_double(void *row, const readstat_variable_t *var, double value) {
    memcpy(row, &value, sizeof(double));
    return READSTAT_OK;
}

static readstat_error_t sas7bdat_write_float(void *row, const readstat_variable_t *var, float value) {
    return sas7bdat_write_double(row, var, value);
}

static readstat_error_t sas7bdat_write_int32(void *row, const readstat_variable_t *var, int32_t value) {
    return sas7bdat_write_double(row, var, value);
}

static readstat_error_t sas7bdat_write_int16(void *row, const readstat_variable_t *var, int16_t value) {
    return sas7bdat_write_double(row, var, value);
}

static readstat_error_t sas7bdat_write_int8(void *row, const readstat_variable_t *var, int8_t value) {
    return sas7bdat_write_double(row, var, value);
}

static readstat_error_t sas7bdat_write_missing_tagged_raw(void *row, const readstat_variable_t *var, char tag) {
    union {
        double dval;
        char   chars[8];
    } nan_value;

    nan_value.dval = NAN;
    nan_value.chars[machine_is_little_endian() ? 5 : 2] = ~tag;
    return sas7bdat_write_double(row, var, nan_value.dval);
}

static readstat_error_t sas7bdat_write_missing_tagged(void *row, const readstat_variable_t *var, char tag) {
    readstat_error_t error = sas_validate_tag(tag);
    if (error == READSTAT_OK)
        return sas7bdat_write_missing_tagged_raw(row, var, tag);

    return error;
}

static readstat_error_t sas7bdat_write_missing_numeric(void *row, const readstat_variable_t *var) {
    return sas7bdat_write_missing_tagged_raw(row, var, '.');
}

static readstat_error_t sas7bdat_write_string(void *row, const readstat_variable_t *var, const char *value) {
    size_t max_len = readstat_variable_get_storage_width(var);
    if (value == NULL || value[0] == '\0') {
        memset(row, ' ', max_len);
    } else {
        size_t value_len = strlen(value);
        if (value_len > max_len)
            return READSTAT_ERROR_STRING_VALUE_IS_TOO_LONG;

        memcpy(row, value, value_len);
        memset((char *)row + value_len, ' ', max_len - value_len);
    }
    return READSTAT_OK;
}

static readstat_error_t sas7bdat_write_missing_string(void *row, const readstat_variable_t *var) {
    return sas7bdat_write_string(row, var, NULL);
}

/* Rearranges the row into the physical column order */
static void sas7bdat_build_physical_row(readstat_writer_t *writer, sas7bdat_write_ctx_t *ctx, const char *row) {
    int i;
    memset(ctx->phys_row, 0, ctx->row_length);
    for (i=0; i<writer->variables_count; i++) {
        readstat_variable_t *variable = readstat_get_variable(writer, i);
        size_t width = sas7bdat_variable_width(variable->type, variable->storage_width);
        memcpy(&ctx->phys_row[ctx->phys_offsets[i]], &row[variable->offset], width);
    }
}

static readstat_error_t sas7bdat_write_row_uncompressed(readstat_writer_t *writer, sas7bdat_write_ctx_t *ctx) {
    readstat_error_t retval = READSTAT_OK;

    if (ctx->page_rows == ctx->page_max_rows) {
        if ((retval = sas7bdat_flush_page(writer, ctx)) != READSTAT_OK)
            goto cleanup;

        ctx->physical_page++;
        memset(ctx->page, 0, ctx->page_size);
        sas7bdat_set_page_number(ctx, ctx->page, ctx->physical_page);
        ctx->page_is_mix = 0;
        ctx->page_row_offset = ctx->phs;
        ctx->page_rows = 0;
        ctx->page_max_rows = ctx->rows_per_data_page;
    }

    memcpy(&ctx->page[ctx->page_row_offset], ctx->phys_row, ctx->row_length);
    ctx->page_row_offset += ctx->row_length;
    ctx->page_rows++;

cleanup:
    return retval;
}

/* Compressed rows are collected in memory as subheaders and written out at
 * the end, once the page count is known. */
static readstat_error_t sas7bdat_write_row_compressed(readstat_writer_t *writer, sas7bdat_write_ctx_t *ctx) {
    readstat_error_t retval = READSTAT_OK;
    size_t len = ctx->row_length;
    size_t compressed_len = sas_rle_compressed_len(ctx->phys_row, len);

    sas7bdat_subheader_t *subheader = NULL;
    if (compressed_len < len) {
        subheader = sas7bdat_subheader_init(0, compressed_len, 0, 1);
        if (subheader == NULL) {
            retval = READSTAT_ERROR_MALLOC;
            goto cleanup;
        }
        subheader->is_row_data = 1;
        subheader->comp = SAS_COMPRESSION_ROW;
        size_t actual_len = sas_rle_compress(subheader->data, subheader->len, ctx->phys_row, len);
        if (actual_len != compressed_len) {
            retval = READSTAT_ERROR_ROW_WIDTH_MISMATCH;
            goto cleanup;
        }
    } else {
        subheader = sas7bdat_subheader_init(0, len, 0, 1);
        if (subheader == NULL) {
            retval = READSTAT_ERROR_MALLOC;
            goto cleanup;
        }
        subheader->is_row_data = 1;
        subheader->comp = SAS_COMPRESSION_NONE;
        memcpy(subheader->data, ctx->phys_row, len);
    }

    retval = sas7bdat_layout_add(ctx, subheader);

cleanup:
    if (retval != READSTAT_OK)
        sas7bdat_subheader_free(subheader);

    return retval;
}

static readstat_error_t sas7bdat_write_row(void *writer_ctx, void *bytes, size_t len) {
    readstat_writer_t *writer = (readstat_writer_t *)writer_ctx;
    sas7bdat_write_ctx_t *ctx = (sas7bdat_write_ctx_t *)writer->module_ctx;

    sas7bdat_build_physical_row(writer, ctx, bytes);

    if (ctx->compressed) {
        return sas7bdat_write_row_compressed(writer, ctx);
    }
    return sas7bdat_write_row_uncompressed(writer, ctx);
}

static readstat_error_t sas7bdat_metadata_ok(void *writer_ctx) {
    readstat_writer_t *writer = (readstat_writer_t *)writer_ctx;

    if (writer->compression != READSTAT_COMPRESS_NONE &&
            writer->compression != READSTAT_COMPRESS_ROWS)
        return READSTAT_ERROR_UNSUPPORTED_COMPRESSION;

    return READSTAT_OK;
}

readstat_error_t readstat_begin_writing_sas7bdat(readstat_writer_t *writer, void *user_ctx, long row_count) {

    if (writer->version == 0)
        writer->version = SAS_DEFAULT_FILE_VERSION;

    writer->callbacks.metadata_ok = &sas7bdat_metadata_ok;
    writer->callbacks.write_int8 = &sas7bdat_write_int8;
    writer->callbacks.write_int16 = &sas7bdat_write_int16;
    writer->callbacks.write_int32 = &sas7bdat_write_int32;
    writer->callbacks.write_float = &sas7bdat_write_float;
    writer->callbacks.write_double = &sas7bdat_write_double;

    writer->callbacks.write_string = &sas7bdat_write_string;
    writer->callbacks.write_missing_string = &sas7bdat_write_missing_string;
    writer->callbacks.write_missing_number = &sas7bdat_write_missing_numeric;
    writer->callbacks.write_missing_tagged = &sas7bdat_write_missing_tagged;

    writer->callbacks.variable_width = &sas7bdat_variable_width;
    writer->callbacks.variable_ok = &sas_validate_variable;

    writer->callbacks.begin_data = &sas7bdat_begin_data;
    writer->callbacks.end_data = &sas7bdat_end_data;
    writer->callbacks.module_ctx_free = &sas7bdat_module_ctx_free;

    writer->callbacks.write_row = &sas7bdat_write_row;

    return readstat_begin_writing_file(writer, user_ctx, row_count);
}
