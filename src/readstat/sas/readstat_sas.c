
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <limits.h>
#include <inttypes.h>

#include "readstat_sas.h"
#include "../readstat_iconv.h"
#include "../readstat_convert.h"
#include "../readstat_writer.h"

#define SAS_FILE_HEADER_SIZE_32BIT 1024
#define SAS_FILE_HEADER_SIZE_64BIT 8192
#define SAS_DEFAULT_PAGE_SIZE      4096

#define SAS_DEFAULT_STRING_ENCODING "WINDOWS-1252"

unsigned char sas7bdat_magic_number[32] = {
    0x00, 0x00, 0x00, 0x00,   0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,   0xc2, 0xea, 0x81, 0x60,
    0xb3, 0x14, 0x11, 0xcf,   0xbd, 0x92, 0x08, 0x00,
    0x09, 0xc7, 0x31, 0x8c,   0x18, 0x1f, 0x10, 0x11
};

unsigned char sas7bcat_magic_number[32] = {
    0x00, 0x00, 0x00, 0x00,   0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,   0xc2, 0xea, 0x81, 0x63,
    0xb3, 0x14, 0x11, 0xcf,   0xbd, 0x92, 0x08, 0x00,
    0x09, 0xc7, 0x31, 0x8c,   0x18, 0x1f, 0x10, 0x11
};

/* This table is cobbled together from extant files and:
 * https://support.sas.com/documentation/cdl/en/nlsref/61893/HTML/default/viewer.htm#a002607278.htm
 * https://support.sas.com/documentation/onlinedoc/dfdmstudio/2.6/dmpdmsug/Content/dfU_Encodings_SAS.html
 *
 * Discrepancies form the official documentation are noted with a comment. It
 * appears that in some instances that SAS software uses a newer encoding than
 * what's listed in the docs. In these cases the encoding used by ReadStat 
 * represents the author's best guess.
 */
static readstat_charset_entry_t _charset_table[] = { 
    { .code = 0,     .name = SAS_DEFAULT_STRING_ENCODING },
    { .code = 20,    .name = "UTF-8" },
    { .code = 28,    .name = "US-ASCII" },
    { .code = 29,    .name = "ISO-8859-1" },
    { .code = 30,    .name = "ISO-8859-2" },
    { .code = 31,    .name = "ISO-8859-3" },
    { .code = 32,    .name = "ISO-8859-4" },
    { .code = 33,    .name = "ISO-8859-5" },
    { .code = 34,    .name = "ISO-8859-6" },
    { .code = 35,    .name = "ISO-8859-7" },
    { .code = 36,    .name = "ISO-8859-8" },
    { .code = 37,    .name = "ISO-8859-9" },
    { .code = 38,    .name = "ISO-8859-10" },
    { .code = 39,    .name = "ISO-8859-11" },
    { .code = 40,    .name = "ISO-8859-15" },
    { .code = 41,    .name = "CP437" },
    { .code = 42,    .name = "CP850" },
    { .code = 43,    .name = "CP852" },
    { .code = 44,    .name = "CP857" },
    { .code = 45,    .name = "CP858" },
    { .code = 46,    .name = "CP862" },
    { .code = 47,    .name = "CP864" },
    { .code = 48,    .name = "CP865" },
    { .code = 49,    .name = "CP866" },
    { .code = 50,    .name = "CP869" },
    { .code = 51,    .name = "CP874" },
    { .code = 52,    .name = "CP921" },
    { .code = 53,    .name = "CP922" },
    { .code = 54,    .name = "CP1129" },
    { .code = 55,    .name = "CP720" },
    { .code = 56,    .name = "CP737" },
    { .code = 57,    .name = "CP775" },
    { .code = 58,    .name = "CP860" },
    { .code = 59,    .name = "CP863" },
    { .code = 60,    .name = "WINDOWS-1250" },
    { .code = 61,    .name = "WINDOWS-1251" },
    { .code = 62,    .name = "WINDOWS-1252" },
    { .code = 63,    .name = "WINDOWS-1253" },
    { .code = 64,    .name = "WINDOWS-1254" },
    { .code = 65,    .name = "WINDOWS-1255" },
    { .code = 66,    .name = "WINDOWS-1256" },
    { .code = 67,    .name = "WINDOWS-1257" },
    { .code = 68,    .name = "WINDOWS-1258" },
    { .code = 69,    .name = "MACROMAN" },
    { .code = 70,    .name = "MACARABIC" },
    { .code = 71,    .name = "MACHEBREW" },
    { .code = 72,    .name = "MACGREEK" },
    { .code = 73,    .name = "MACTHAI" },
    { .code = 75,    .name = "MACTURKISH" },
    { .code = 76,    .name = "MACUKRAINE" },
    { .code = 118,   .name = "CP950" },
    { .code = 119,   .name = "EUC-TW" },
    { .code = 123,   .name = "BIG-5" },
    { .code = 125,   .name = "GB18030" }, // "euc-cn" in SAS
    { .code = 126,   .name = "WINDOWS-936" }, // "zwin"
    { .code = 128,   .name = "CP1381" }, // "zpce"
    { .code = 134,   .name = "EUC-JP" },
    { .code = 136,   .name = "CP932" }, // "ms-932" in SAS
    { .code = 137,   .name = "CP942" },
    { .code = 138,   .name = "CP932" }, // "shift-jis" in SAS
    { .code = 140,   .name = "EUC-KR" },
    { .code = 141,   .name = "CP949" }, // "kpce"
    { .code = 142,   .name = "CP949" }, // "kwin"
    { .code = 163,   .name = "MACICELAND" },
    { .code = 167,   .name = "ISO-2022-JP" },
    { .code = 168,   .name = "ISO-2022-KR" },
    { .code = 169,   .name = "ISO-2022-CN" },
    { .code = 172,   .name = "ISO-2022-CN-EXT" },
    { .code = 204,   .name = SAS_DEFAULT_STRING_ENCODING }, // "any" in SAS
    { .code = 205,   .name = "GB18030" },
    { .code = 227,   .name = "ISO-8859-14" },
    { .code = 242,   .name = "ISO-8859-13" },
    { .code = 245,   .name = "MACCROATIAN" },
    { .code = 246,   .name = "MACCYRILLIC" },
    { .code = 247,   .name = "MACROMANIA" },
    { .code = 248,   .name = "SHIFT_JISX0213" },
};

static time_t sas_epoch(void) {
    return - 3653 * 86400; // seconds between 01-01-1960 and 01-01-1970
}

static time_t sas_convert_time(double time, double time_diff, time_t epoch) {
    time -= time_diff;
    time += epoch;
    if (isnan(time))
        return 0;
    if (time > (double)LONG_MAX)
        return LONG_MAX;
    if (time < (double)LONG_MIN)
        return LONG_MIN;
    return time;
}

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

size_t sas_subheader_remainder(size_t len, size_t signature_len) {
    return len - (4+2*signature_len);
}

readstat_error_t sas_read_header(readstat_io_t *io, sas_header_info_t *hinfo, 
        readstat_error_handler error_handler, void *user_ctx) {
    sas_header_start_t  header_start;
    sas_header_end_t    header_end;
    int retval = READSTAT_OK;
    char error_buf[1024];
    time_t epoch = sas_epoch();

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
            snprintf(error_buf, sizeof(error_buf), "Unsupported character set code: %d", header_start.encoding);
            error_handler(error_buf, user_ctx);
        }
        retval = READSTAT_ERROR_UNSUPPORTED_CHARSET;
        goto cleanup;
    }
    memcpy(hinfo->table_name, header_start.table_name, sizeof(header_start.table_name));
    if (io->seek(hinfo->pad1, READSTAT_SEEK_CUR, io->io_ctx) == -1) {
        retval = READSTAT_ERROR_SEEK;
        goto cleanup;
    }

    double creation_time, modification_time, creation_time_diff, modification_time_diff;

    if (io->read(&creation_time, sizeof(double), io->io_ctx) < sizeof(double)) {
        retval = READSTAT_ERROR_READ;
        goto cleanup;
    }
    if (bswap)
        creation_time = byteswap_double(creation_time);

    if (io->read(&modification_time, sizeof(double), io->io_ctx) < sizeof(double)) {
        retval = READSTAT_ERROR_READ;
        goto cleanup;
    }
    if (bswap)
        modification_time = byteswap_double(modification_time);

    if (io->read(&creation_time_diff, sizeof(double), io->io_ctx) < sizeof(double)) {
        retval = READSTAT_ERROR_READ;
        goto cleanup;
    }
    if (bswap)
        creation_time_diff = byteswap_double(creation_time_diff);
    
    if (io->read(&modification_time_diff, sizeof(double), io->io_ctx) < sizeof(double)) {
        retval = READSTAT_ERROR_READ;
        goto cleanup;
    }
    if (bswap)
        modification_time_diff = byteswap_double(modification_time_diff);
    
    hinfo->creation_time = sas_convert_time(creation_time, creation_time_diff, epoch);
    hinfo->modification_time = sas_convert_time(modification_time, modification_time_diff, epoch);

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

    if (hinfo->header_size < 1024 || hinfo->page_size < 1024) {
        retval = READSTAT_ERROR_PARSE;
        goto cleanup;
    }
    if (hinfo->header_size > (1<<24) || hinfo->page_size > (1<<24)) {
        retval = READSTAT_ERROR_PARSE;
        goto cleanup;
    }

    if (hinfo->u64) {
        hinfo->page_header_size = SAS_PAGE_HEADER_SIZE_64BIT;
        hinfo->subheader_pointer_size = SAS_SUBHEADER_POINTER_SIZE_64BIT;
    } else {
        hinfo->page_header_size = SAS_PAGE_HEADER_SIZE_32BIT;
        hinfo->subheader_pointer_size = SAS_SUBHEADER_POINTER_SIZE_32BIT;
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
    if (hinfo->page_count > (1<<24)) {
        retval = READSTAT_ERROR_PARSE;
        goto cleanup;
    }
    
    if (io->seek(8, READSTAT_SEEK_CUR, io->io_ctx) == -1) {
        retval = READSTAT_ERROR_SEEK;
        if (error_handler) {
            snprintf(error_buf, sizeof(error_buf), "ReadStat: Failed to seek forward by %d", 8);
            error_handler(error_buf, user_ctx);
        }
        goto cleanup;
    }
    if (io->read(&header_end, sizeof(sas_header_end_t), io->io_ctx) < sizeof(sas_header_end_t)) {
        retval = READSTAT_ERROR_READ;
        goto cleanup;
    }
    char major, revision_tag;
    int minor, revision;
    /* The release field is not NUL-terminated in the file; copy it into a
     * terminated buffer before handing it to sscanf, which calls strlen on it. */
    char release[sizeof(header_end.release)+1];
    memcpy(release, header_end.release, sizeof(header_end.release));
    release[sizeof(header_end.release)] = '\0';
    if (sscanf(release, "%c.%04d%c%1d", &major, &minor, &revision_tag, &revision) != 4) {
        retval = READSTAT_ERROR_PARSE;
        goto cleanup;
    }

    if (major >= '1' && major <= '9') {
        hinfo->major_version = major - '0';
    } else if (major == 'V') {
        // It appears that SAS Visual Forecaster reports the major version as "V"
        // Treat it as version 9 for all intents and purposes
        hinfo->major_version = 9;
    } else {
        retval = READSTAT_ERROR_PARSE;
        goto cleanup;
    }
    // revision_tag is usually M, but J has been observed in the wild (not created with SAS?)
    if ((major == '8' || major == '9') && revision_tag != 'M' && revision_tag != 'J') {
        retval = READSTAT_ERROR_PARSE;
        goto cleanup;
    }
    hinfo->minor_version = minor;
    hinfo->revision = revision;

    if ((major == '8' || major == '9') && minor == 0 && revision == 0) {
        /* A bit of a hack, but most SAS installations are running a minor update */
        hinfo->vendor = READSTAT_VENDOR_STAT_TRANSFER;
    } else {
        hinfo->vendor = READSTAT_VENDOR_SAS;
    }
    if (io->seek(hinfo->header_size, READSTAT_SEEK_SET, io->io_ctx) == -1) {
        retval = READSTAT_ERROR_SEEK;
        if (error_handler) {
            snprintf(error_buf, sizeof(error_buf), 
                    "ReadStat: Failed to seek to position %" PRId64, hinfo->header_size);
            error_handler(error_buf, user_ctx);
        }
        goto cleanup;
    }

cleanup:
    return retval;
}

/* Seconds between local time and UTC at the given instant (local - UTC).
 * SAS stores its timestamps in local time and records this offset next to them. */
static double sas_local_utc_offset(time_t timestamp) {
    struct tm *gmt = gmtime(&timestamp);
    if (gmt == NULL)
        return 0;
    struct tm utc_as_local = *gmt;
    utc_as_local.tm_isdst = -1;
    time_t reinterpreted = mktime(&utc_as_local);
    if (reinterpreted == (time_t)-1)
        return 0;
    return difftime(timestamp, reinterpreted);
}

static void sas_write_padded_ascii(char *dst, size_t dst_len, const char *src, char pad) {
    size_t len = strlen(src);
    if (len > dst_len)
        len = dst_len;
    memcpy(dst, src, len);
    memset(dst + len, pad, dst_len - len);
}

/* The layout of the file header follows the sas7bdat specification
 * (Shotwell & Yu, as updated by FredHutch/sas7bdat-specification).
 * Bytes with no known meaning are set to the constant values observed
 * in every file written by SAS 9. */
readstat_error_t sas_write_header(readstat_writer_t *writer, sas_header_info_t *hinfo, sas_header_start_t header_start) {
    readstat_error_t retval = READSTAT_OK;
    time_t epoch = sas_epoch();
    int u64 = hinfo->u64;
    int a1 = (header_start.a1 == SAS_ALIGNMENT_OFFSET_4) ? 4 : 0;
    int a2 = u64 ? 4 : 0;
    size_t off = 0;
    unsigned char *header = calloc(1, hinfo->header_size);
    if (header == NULL)
        return READSTAT_ERROR_MALLOC;

    unsigned char encoding = header_start.encoding ? header_start.encoding : 20; /* UTF-8 */

    memcpy(header, &header_start, sizeof(sas_header_start_t));

    header[32] = u64 ? SAS_ALIGNMENT_OFFSET_4 : SAS_ALIGNMENT_OFFSET_0;
    header[33] = 0x22;
    header[34] = 0x00;
    header[35] = header_start.a1;
    header[36] = a1 ? 0x33 : 0x22;
    header[37] = header_start.endian;
    header[38] = 0x02;
    header[39] = header_start.file_format;
    header[40] = u64 ? 0x01 : 0x04;
    memset(&header[41], 0, 6);
    header[47] = encoding;
    header[48] = 0x00;
    header[49] = 0x00;
    header[50] = 0x03;
    header[51] = 0x01;
    header[52] = 0x18;
    header[53] = 0x1F;
    header[54] = 0x10;
    header[55] = 0x11;
    memcpy(&header[56], &header[32], 8);
    header[64] = u64 ? 0x01 : 0x04;
    header[65] = u64 ? 0x33 : 0x32;
    header[66] = 0x01;
    header[67] = u64 ? 0x23 : 0x22;
    header[68] = u64 ? 0x33 : 0x22;
    header[69] = 0x00;
    header[70] = encoding;
    header[71] = encoding;
    header[72] = 0x00;
    header[73] = u64 ? 0x20 : 0x10;
    header[74] = 0x03;
    header[75] = 0x01;
    memset(&header[76], 0, 8);

    memcpy(&header[84], header_start.file_type, sizeof(header_start.file_type));

    /* Dataset name: 64 bytes, space padded */
    if (writer->table_name[0]) {
        sas_write_padded_ascii((char *)&header[92], 64, writer->table_name, ' ');
    } else {
        sas_write_padded_ascii((char *)&header[92], 64, "DATASET", ' ');
    }

    memcpy(&header[156], header_start.file_info, sizeof(header_start.file_info));

    double utc_offset = sas_local_utc_offset(writer->timestamp);
    double creation_time = (double)(hinfo->creation_time - epoch) + utc_offset;
    double modification_time = (double)(hinfo->modification_time - epoch) + utc_offset;

    off = 164 + a1;
    memcpy(&header[off], &creation_time, sizeof(double));
    memcpy(&header[off+8], &modification_time, sizeof(double));
    memcpy(&header[off+16], &utc_offset, sizeof(double));
    memcpy(&header[off+24], &utc_offset, sizeof(double));

    uint32_t header_size = hinfo->header_size;
    uint32_t page_size = hinfo->page_size;
    memcpy(&header[off+32], &header_size, sizeof(uint32_t));
    memcpy(&header[off+36], &page_size, sizeof(uint32_t));
    if (u64) {
        uint64_t page_count = hinfo->page_count;
        memcpy(&header[off+40], &page_count, sizeof(uint64_t));
    } else {
        uint32_t page_count = hinfo->page_count;
        memcpy(&header[off+40], &page_count, sizeof(uint32_t));
    }

    off = 216 + a1 + a2;

    char release[9];
    if (writer->version == 9) {
        snprintf(release, sizeof(release), "9.0401M2");
    } else {
        snprintf(release, sizeof(release), "%1d.0202M0", (unsigned int)writer->version % 10);
    }
    memcpy(&header[off], release, 8);
    sas_write_padded_ascii((char *)&header[off+8], 16, "Linux", '\0');   /* host */
    sas_write_padded_ascii((char *)&header[off+24], 16, "", '\0');       /* OS version */
    sas_write_padded_ascii((char *)&header[off+40], 16, "", '\0');       /* OS vendor */
    sas_write_padded_ascii((char *)&header[off+56], 16, "x86_64", '\0'); /* OS name */

    /* Bytes 288-303 are involved in the READ-password check. SAS derives them
     * from the creation date (and the password, if any); the derivation is not
     * known, so use a pair of values copied from an unencrypted SAS dataset,
     * which SAS accepts. */
    uint32_t pattern1 = 0xD4C8C038;
    uint32_t pattern2 = 0xB1A78E74;
    memcpy(&header[off+72], &pattern1, sizeof(uint32_t));
    memcpy(&header[off+76], &pattern2, sizeof(uint32_t));
    memcpy(&header[off+80], &pattern2, sizeof(uint32_t));
    memcpy(&header[off+84], &pattern2, sizeof(uint32_t));

    /* 304-319: zeros */

    /* 320: page number mask (the "initial page sequence number") */
    memcpy(&header[off+104], &hinfo->page_number_mask, sizeof(uint32_t));

    /* 328: creation timestamp again */
    memcpy(&header[off+112], &creation_time, sizeof(double));

    retval = readstat_write_bytes(writer, header, hinfo->header_size);

    free(header);

    return retval;
}

sas_header_info_t *sas_header_info_init(readstat_writer_t *writer, int is_64bit) {
    sas_header_info_t *hinfo = calloc(1, sizeof(sas_header_info_t));
    hinfo->creation_time = writer->timestamp;
    hinfo->modification_time = writer->timestamp;
    hinfo->page_size = SAS_DEFAULT_PAGE_SIZE;
    hinfo->u64 = !!is_64bit;

    if (hinfo->u64) {
        hinfo->header_size = SAS_FILE_HEADER_SIZE_64BIT;
        hinfo->page_header_size = SAS_PAGE_HEADER_SIZE_64BIT;
        hinfo->subheader_pointer_size = SAS_SUBHEADER_POINTER_SIZE_64BIT;
    } else {
        hinfo->header_size = SAS_FILE_HEADER_SIZE_32BIT;
        hinfo->page_header_size = SAS_PAGE_HEADER_SIZE_32BIT;
        hinfo->subheader_pointer_size = SAS_SUBHEADER_POINTER_SIZE_32BIT;
    }

    return hinfo;
}

readstat_error_t sas_fill_page(readstat_writer_t *writer, sas_header_info_t *hinfo) {
    if ((writer->bytes_written - hinfo->header_size) % hinfo->page_size) {
        size_t num_zeros = (hinfo->page_size -
                (writer->bytes_written - hinfo->header_size) % hinfo->page_size);
        return readstat_write_zeros(writer, num_zeros);
    }
    return READSTAT_OK;
}

readstat_error_t sas_validate_name(const char *name, size_t max_len) {
    int j;
    for (j=0; name[j]; j++) {
        if (name[j] != '_' &&
                !(name[j] >= 'a' && name[j] <= 'z') &&
                !(name[j] >= 'A' && name[j] <= 'Z') &&
                !(name[j] >= '0' && name[j] <= '9')) {
            return READSTAT_ERROR_NAME_CONTAINS_ILLEGAL_CHARACTER;
        }
    }
    char first_char = name[0];

    if (!first_char)
        return READSTAT_ERROR_NAME_IS_ZERO_LENGTH;

    if (first_char != '_' &&
            !(first_char >= 'a' && first_char <= 'z') &&
            !(first_char >= 'A' && first_char <= 'Z')) {
        return READSTAT_ERROR_NAME_BEGINS_WITH_ILLEGAL_CHARACTER;
    }
    if (strcmp(name, "_N_") == 0 || strcmp(name, "_ERROR_") == 0 ||
            strcmp(name, "_NUMERIC_") == 0 || strcmp(name, "_CHARACTER_") == 0 ||
            strcmp(name, "_ALL_") == 0) {
        return READSTAT_ERROR_NAME_IS_RESERVED_WORD;
    }

    if (strlen(name) > max_len)
        return READSTAT_ERROR_NAME_IS_TOO_LONG;

    return READSTAT_OK;
}

readstat_error_t sas_validate_variable(const readstat_variable_t *variable) {
    return sas_validate_name(readstat_variable_get_name(variable), 32);
}

readstat_error_t sas_validate_tag(char tag) {
    if (tag == '_' || (tag >= 'A' && tag <= 'Z'))
        return READSTAT_OK;

    return READSTAT_ERROR_TAGGED_VALUE_IS_OUT_OF_RANGE;
}

void sas_assign_tag(readstat_value_t *value, uint8_t tag) {
    /* We accommodate two tag schemes. In the first, the tag is an ASCII code
     * given by uint8_t tag above. System missing is represented by an ASCII
     * period. In the second scheme, (tag-2) is an offset from 'A', except when
     * tag == 0, in which case it represents an underscore, or tag == 1, in
     * which case it represents system-missing.
     */
    if (tag == 0) {
        tag = '_';
    } else if (tag >= 2 && tag < 28) {
        tag = 'A' + (tag - 2);
    }
    if (sas_validate_tag(tag) == READSTAT_OK) {
        value->tag = tag;
        value->is_tagged_missing = 1;
    } else {
        value->tag = 0;
        value->is_system_missing = 1;
    }
}
