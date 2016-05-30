
#include "readstat.h"
#include "readstat_bits.h"

#pragma pack(push, 1)

typedef struct sas_header_start_s {
    unsigned char magic[32];
    unsigned char a2;
    unsigned char mystery1[2];
    unsigned char a1;
    unsigned char mystery2[1];
    unsigned char endian;
    unsigned char mystery3[1];
    char          file_format;
    unsigned char mystery4[30];
    unsigned char encoding;
    unsigned char mystery5[13];
    char          type[8];
    char          dataset[64];
} sas_header_start_t;

typedef struct sas_header_end_s {
    char          release[8];
    char          host[16];
    char          version[16];
    char          os_vendor[16];
    char          os_name[16];
    char          extra[48];
} sas_header_end_t;

#pragma pack(pop)

typedef struct sas_header_info_s {
    int      little_endian;
    int      u64;
    int      vendor;
    int      pad1;
    int64_t  page_size;
    int64_t  page_count;
    int64_t  header_size;
    char    *encoding;
} sas_header_info_t;

enum {
    READSTAT_VENDOR_STAT_TRANSFER,
    READSTAT_VENDOR_SAS
};

uint64_t sas_read8(const char *data, int bswap);
uint32_t sas_read4(const char *data, int bswap);
uint16_t sas_read2(const char *data, int bswap);
readstat_error_t sas_read_header(readstat_io_t *io, sas_header_info_t *ctx, readstat_error_handler error_handler, void *user_ctx);
