#include "readstat.h"
#include "readstat_iconv.h"
#include "readstat_bits.h"

#pragma pack(push, 1)

// DTA files

typedef struct dta_header_s {
    unsigned char    ds_format;
    unsigned char    byteorder;
    unsigned char    filetype;
    unsigned char    unused;
    int16_t          nvar;
    int32_t          nobs;
} dta_header_t;

typedef struct dta_expansion_field_s {
    unsigned char    data_type;
    int32_t          len;
} dta_expansion_field_t;

typedef struct dta_short_expansion_field_s {
    unsigned char    data_type;
    int16_t          len;
} dta_short_expansion_field_t;

typedef struct dta_gso_header_s {
    char             gso[3];
    uint32_t         v;
    uint32_t         o;
    unsigned char    t;
    int32_t          len;
} dta_gso_header_t;

#pragma pack(pop)

typedef struct dta_ctx_s {
    size_t         data_label_len;
    size_t         data_label_len_len;
    size_t         time_stamp_len;
    char           typlist_version;
    size_t         typlist_entry_len;
    uint16_t      *typlist;
    size_t         typlist_len;
    char          *varlist;
    size_t         varlist_len;
    int16_t       *srtlist;
    size_t         srtlist_len;
    char          *fmtlist;
    size_t         fmtlist_len;
    char          *lbllist;
    size_t         lbllist_len;
    char          *variable_labels;
    size_t         variable_labels_len;

    size_t         variable_name_len;
    size_t         fmtlist_entry_len;
    size_t         lbllist_entry_len;
    size_t         variable_labels_entry_len;
    size_t         expansion_len_len;
    size_t         value_label_table_len_len;
    size_t         value_label_table_labname_len;
    size_t         value_label_table_padding_len;

    off_t          data_offset;
    off_t          strls_offset;
    off_t          value_labels_offset;

    int            nvar;
    int            nobs;
    size_t         record_len;
    int            row_limit;

    int            machine_needs_byte_swap;
    int            machine_is_twos_complement;
    int            file_is_xmlish;
    int            supports_tagged_missing;

    char           max_char;
    int16_t        max_int16;
    int32_t        max_int32;
    int32_t        max_float;
    int64_t        max_double;

    iconv_t        converter;
    readstat_progress_handler progress_handler;
    readstat_variable_handler variable_handler;
    readstat_value_handler value_handler;
    readstat_value_label_handler value_label_handler;
    size_t                    file_size;
    void                     *user_ctx;
    readstat_io_t            *io;
    int                       initialized;
} dta_ctx_t;

#define DTA_HILO  0x01
#define DTA_LOHI  0x02

#define DTA_OLD_MAX_CHAR                     0x7e
#define DTA_OLD_MAX_INT16                  0x7ffe
#define DTA_OLD_MAX_INT32              0x7ffffffe
#define DTA_OLD_MAX_FLOAT              0x7effffff // +1.7e38f
#define DTA_OLD_MAX_DOUBLE     0x7fdfffffffffffffL // +8.9e307

#define DTA_OLD_MISSING_CHAR                 0x7F
#define DTA_OLD_MISSING_INT16              0x7FFF
#define DTA_OLD_MISSING_INT32          0x7FFFFFFF
#define DTA_OLD_MISSING_FLOAT          0x7F000000
#define DTA_OLD_MISSING_DOUBLE 0x7FE0000000000000L

#define DTA_113_MAX_CHAR                    0x64
#define DTA_113_MAX_INT16                 0x7fe4
#define DTA_113_MAX_INT32             0x7fffffe4
#define DTA_113_MAX_FLOAT             0x7effffff // +1.7e38f
#define DTA_113_MAX_DOUBLE    0x7fdfffffffffffffL // +8.9e307

#define DTA_113_MISSING_CHAR                 0x65
#define DTA_113_MISSING_INT16              0x7FE5
#define DTA_113_MISSING_INT32          0x7FFFFFE5
#define DTA_113_MISSING_FLOAT          0x7F000000
#define DTA_113_MISSING_DOUBLE 0x7FE0000000000000L

#define DTA_113_MISSING_CHAR_A    (DTA_113_MISSING_CHAR+1)
#define DTA_113_MISSING_INT16_A   (DTA_113_MISSING_INT16+1)
#define DTA_113_MISSING_INT32_A   (DTA_113_MISSING_INT32+1)
#define DTA_113_MISSING_FLOAT_A   (DTA_113_MISSING_FLOAT+0x0800)
#define DTA_113_MISSING_DOUBLE_A  (DTA_113_MISSING_DOUBLE+0x010000000000)

#define DTA_GSO_TYPE_BINARY        0x81
#define DTA_GSO_TYPE_ASCII         0x82

#define DTA_117_TYPE_CODE_CHAR     0xFFFA
#define DTA_117_TYPE_CODE_INT16    0xFFF9
#define DTA_117_TYPE_CODE_INT32    0xFFF8
#define DTA_117_TYPE_CODE_FLOAT    0xFFF7
#define DTA_117_TYPE_CODE_DOUBLE   0xFFF6
#define DTA_117_TYPE_CODE_STRL     0x8000

#define DTA_111_TYPE_CODE_CHAR     0xFB
#define DTA_111_TYPE_CODE_INT16    0xFC
#define DTA_111_TYPE_CODE_INT32    0xFD
#define DTA_111_TYPE_CODE_FLOAT    0xFE
#define DTA_111_TYPE_CODE_DOUBLE   0xFF

#define DTA_OLD_TYPE_CODE_CHAR     'b'
#define DTA_OLD_TYPE_CODE_INT16    'i'
#define DTA_OLD_TYPE_CODE_INT32    'l'
#define DTA_OLD_TYPE_CODE_FLOAT    'f'
#define DTA_OLD_TYPE_CODE_DOUBLE   'd'

dta_ctx_t *dta_ctx_alloc(readstat_io_t *io);
readstat_error_t dta_ctx_init(dta_ctx_t *ctx, int16_t nvar, int32_t nobs, 
        unsigned char byteorder, unsigned char ds_format,
        const char *input_encoding, const char *output_encoding);
void dta_ctx_free(dta_ctx_t *ctx);

readstat_types_t dta_type_info(uint16_t typecode, size_t *max_len, dta_ctx_t *ctx);
