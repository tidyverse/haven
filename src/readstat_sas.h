
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
    char          encoding;
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

