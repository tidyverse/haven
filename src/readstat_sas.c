
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <math.h>
#include <inttypes.h>
#include "readstat_sas.h"
#include "readstat_iconv.h"
#include "readstat_convert.h"

#define SAS_DEFAULT_STRING_ENCODING "WINDOWS-1252"

#define SAS_FILE_FORMAT_UNIX    '1'
#define SAS_FILE_FORMAT_WINDOWS '2'

#define SAS_ALIGNMENT_OFFSET_4  0x33

#define SAS_ENDIAN_BIG       0x00
#define SAS_ENDIAN_LITTLE    0x01

static unsigned char sas7bdat_magic_number[32] = {
    0x00, 0x00, 0x00, 0x00,   0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,   0xc2, 0xea, 0x81, 0x60,
    0xb3, 0x14, 0x11, 0xcf,   0xbd, 0x92, 0x08, 0x00,
    0x09, 0xc7, 0x31, 0x8c,   0x18, 0x1f, 0x10, 0x11
};

static unsigned char sas7bcat_magic_number[32] = {
    0x00, 0x00, 0x00, 0x00,   0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,   0xc2, 0xea, 0x81, 0x63,
    0xb3, 0x14, 0x11, 0xcf,   0xbd, 0x92, 0x08, 0x00,
    0x09, 0xc7, 0x31, 0x8c,   0x18, 0x1f, 0x10, 0x11
};

static readstat_charset_entry_t _charset_table[] = { 
    { .code = 0,     .name = SAS_DEFAULT_STRING_ENCODING },
    { .code = 20,    .name = "UTF-8" },
    { .code = 28,    .name = "US-ASCII" },
    { .code = 29,    .name = "ISO-8859-1" },
    { .code = 30,    .name = "ISO-8859-2" },
    { .code = 31,    .name = "ISO-8859-3" },
    { .code = 34,    .name = "ISO-8859-6" },
    { .code = 35,    .name = "ISO-8859-7" },
    { .code = 36,    .name = "ISO-8859-8" },
    { .code = 39,    .name = "ISO-8859-11" },
    { .code = 40,    .name = "ISO-8859-9" },
    { .code = 60,    .name = "WINDOWS-1250" },
    { .code = 61,    .name = "WINDOWS-1251" },
    { .code = 62,    .name = "WINDOWS-1252" },
    { .code = 63,    .name = "WINDOWS-1253" },
    { .code = 64,    .name = "WINDOWS-1254" },
    { .code = 65,    .name = "WINDOWS-1255" },
    { .code = 66,    .name = "WINDOWS-1256" },
    { .code = 119,   .name = "EUC-TW" },
    { .code = 123,   .name = "BIG-5" },
    { .code = 125,   .name = "EUC-CN" },
    { .code = 134,   .name = "EUC-JP" },
    { .code = 138,   .name = "CP932" }, // is this the right shift-jis?
    { .code = 140,   .name = "EUC-KR" }
};

uint64_t sas_read8(const char *data, int bswap) {
    uint64_t tmp;
    memcpy(&tmp, data, 8);
    return bswap ? byteswap8(tmp) : tmp;
}

uint32_t sas_read4(const char *data, int bswap) {
    uint32_t tmp;
    memcpy(&tmp, data, 4);
    return bswap ? byteswap4(tmp) : tmp;
}

uint16_t sas_read2(const char *data, int bswap) {
    uint16_t tmp;
    memcpy(&tmp, data, 2);
    return bswap ? byteswap2(tmp) : tmp;
}

readstat_error_t sas_read_header(readstat_io_t *io, sas_header_info_t *hinfo, 
        readstat_error_handler error_handler, void *user_ctx) {
    sas_header_start_t  header_start;
    sas_header_end_t    header_end;
    int retval = READSTAT_OK;
    char error_buf[1024];
    if (io->read(&header_start, sizeof(sas_header_start_t), io->io_ctx) < sizeof(sas_header_start_t)) {
        retval = READSTAT_ERROR_READ;
        goto cleanup;
    }
    if (memcmp(header_start.magic, sas7bdat_magic_number, sizeof(sas7bdat_magic_number)) != 0 &&
            memcmp(header_start.magic, sas7bcat_magic_number, sizeof(sas7bcat_magic_number)) != 0) {
        retval = READSTAT_ERROR_PARSE;
        goto cleanup;
    }
    if (header_start.a1 == SAS_ALIGNMENT_OFFSET_4) {
        hinfo->pad1 = 4;
    }
    if (header_start.a2 == SAS_ALIGNMENT_OFFSET_4) {
        hinfo->u64 = 1;
    }
    int bswap = 0;
    if (header_start.endian == SAS_ENDIAN_BIG) {
        bswap = machine_is_little_endian();
        hinfo->little_endian = 0;
    } else if (header_start.endian == SAS_ENDIAN_LITTLE) {
        bswap = !machine_is_little_endian();
        hinfo->little_endian = 1;
    } else {
        retval = READSTAT_ERROR_PARSE;
        goto cleanup;
    }
    int i;
    for (i=0; i<sizeof(_charset_table)/sizeof(_charset_table[0]); i++) {
        if (header_start.encoding == _charset_table[i].code) {
            hinfo->encoding = _charset_table[i].name;
            break;
        }
    }
    if (hinfo->encoding == NULL) {
        if (error_handler) {
            snprintf(error_buf, sizeof(error_buf), "Unsupported character set code: %d\n", header_start.encoding);
            error_handler(error_buf, user_ctx);
        }
        retval = READSTAT_ERROR_UNSUPPORTED_CHARSET;
        goto cleanup;
    }
    if (io->seek(196 + hinfo->pad1, READSTAT_SEEK_SET, io->io_ctx) == -1) {
        retval = READSTAT_ERROR_SEEK;
        if (error_handler) {
            snprintf(error_buf, sizeof(error_buf), "ReadStat: Failed to seek to position %d\n", 196 + hinfo->pad1);
            error_handler(error_buf, user_ctx);
        }
        goto cleanup;
    }

    uint32_t header_size, page_size;

    if (io->read(&header_size, sizeof(uint32_t), io->io_ctx) < sizeof(uint32_t)) {
        retval = READSTAT_ERROR_READ;
        goto cleanup;
    }
    if (io->read(&page_size, sizeof(uint32_t), io->io_ctx) < sizeof(uint32_t)) {
        retval = READSTAT_ERROR_READ;
        goto cleanup;
    }

    hinfo->header_size = bswap ? byteswap4(header_size) : header_size;

    hinfo->page_size = bswap ? byteswap4(page_size) : page_size;

    if (hinfo->header_size < 1024) {
        retval = READSTAT_ERROR_PARSE;
        goto cleanup;
    }

    if (hinfo->u64) {
        uint64_t page_count;
        if (io->read(&page_count, sizeof(uint64_t), io->io_ctx) < sizeof(uint64_t)) {
            retval = READSTAT_ERROR_READ;
            goto cleanup;
        }
        hinfo->page_count = bswap ? byteswap8(page_count) : page_count;
    } else {
        uint32_t page_count;
        if (io->read(&page_count, sizeof(uint32_t), io->io_ctx) < sizeof(uint32_t)) {
            retval = READSTAT_ERROR_READ;
            goto cleanup;
        }
        hinfo->page_count = bswap ? byteswap4(page_count) : page_count;
    }
    
    if (io->seek(8, READSTAT_SEEK_CUR, io->io_ctx) == -1) {
        retval = READSTAT_ERROR_SEEK;
        if (error_handler) {
            snprintf(error_buf, sizeof(error_buf), "ReadStat: Failed to seek forward by %d\n", 8);
            error_handler(error_buf, user_ctx);
        }
        goto cleanup;
    }
    if (io->read(&header_end, sizeof(sas_header_end_t), io->io_ctx) < sizeof(sas_header_end_t)) {
        retval = READSTAT_ERROR_READ;
        goto cleanup;
    }
    if (strncmp(header_end.release, "9.0000M0", sizeof(header_end.release)) == 0) {
        /* A bit of a hack, but most SAS installations are running a minor update */
        hinfo->vendor = READSTAT_VENDOR_STAT_TRANSFER;
    } else {
        hinfo->vendor = READSTAT_VENDOR_SAS;
    }
    if (io->seek(hinfo->header_size, READSTAT_SEEK_SET, io->io_ctx) == -1) {
        retval = READSTAT_ERROR_SEEK;
        if (error_handler) {
            snprintf(error_buf, sizeof(error_buf), 
                    "ReadStat: Failed to seek to position %" PRId64 "\n", hinfo->header_size);
            error_handler(error_buf, user_ctx);
        }
        goto cleanup;
    }

cleanup:
    return retval;
}
