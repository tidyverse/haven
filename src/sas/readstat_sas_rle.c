
#include <sys/types.h>
#include <string.h>
#include <stdio.h>

#include "readstat_sas_rle.h"

#define SAS_RLE_COMMAND_COPY64          0
#define SAS_RLE_COMMAND_INSERT_BYTE18   4
#define SAS_RLE_COMMAND_INSERT_AT17     5
#define SAS_RLE_COMMAND_INSERT_BLANK17  6
#define SAS_RLE_COMMAND_INSERT_ZERO17   7
#define SAS_RLE_COMMAND_COPY1           8
#define SAS_RLE_COMMAND_COPY17          9
#define SAS_RLE_COMMAND_COPY33         10
#define SAS_RLE_COMMAND_COPY49         11
#define SAS_RLE_COMMAND_INSERT_BYTE3   12
#define SAS_RLE_COMMAND_INSERT_AT2     13
#define SAS_RLE_COMMAND_INSERT_BLANK2  14
#define SAS_RLE_COMMAND_INSERT_ZERO2   15

size_t sas_rle_decompress(void *output_buf, size_t output_len, 
        const void *input_buf, size_t input_len) {
    /* TODO bounds checking */
    unsigned char *buffer = (unsigned char *)output_buf;
    unsigned char *output = buffer;

    const unsigned char *input = (const unsigned char *)input_buf;

    while (input < (const unsigned char *)input_buf + input_len) {
        unsigned char control = *input++;
        unsigned char command = (control & 0xF0) >> 4;
        unsigned char length = (control & 0x0F);
        int copy_len = 0;
        int insert_len = 0;
        unsigned char insert_byte = '\0';
        switch (command) {
            case SAS_RLE_COMMAND_COPY64:
                copy_len = (*input++) + 64 + length * 256;
                break;
            case SAS_RLE_COMMAND_INSERT_BYTE18:
                insert_len = (*input++) + 18 + length * 16;
                insert_byte = *input++;
                break;
            case SAS_RLE_COMMAND_INSERT_AT17:
                insert_len = (*input++) + 17 + length * 256;
                insert_byte = '@';
                break;
            case SAS_RLE_COMMAND_INSERT_BLANK17:
                insert_len = (*input++) + 17 + length * 256;
                insert_byte = ' ';
                break;
            case SAS_RLE_COMMAND_INSERT_ZERO17:
                insert_len = (*input++) + 17 + length * 256;
                insert_byte = '\0';
                break;
            case SAS_RLE_COMMAND_COPY1:  copy_len = length + 1; break;
            case SAS_RLE_COMMAND_COPY17: copy_len = length + 17; break;
            case SAS_RLE_COMMAND_COPY33: copy_len = length + 33; break;
            case SAS_RLE_COMMAND_COPY49: copy_len = length + 49; break;
            case SAS_RLE_COMMAND_INSERT_BYTE3:
                insert_byte = *input++;
                insert_len = length + 3;
                break;
            case SAS_RLE_COMMAND_INSERT_AT2:
                insert_byte = '@';
                insert_len = length + 2;
                break;
            case SAS_RLE_COMMAND_INSERT_BLANK2:
                insert_byte = ' ';
                insert_len = length + 2;
                break;
            case SAS_RLE_COMMAND_INSERT_ZERO2:
                insert_byte = '\0';
                insert_len = length + 2;
                break;
            default:
                /* error out here? */
                break;
        }
        if (copy_len) {
            memcpy(output, input, copy_len);
            input += copy_len;
            output += copy_len;
        }
        if (insert_len) {
            memset(output, insert_byte, insert_len);
            output += insert_len;
        }
    }

    return output - buffer;
}

static size_t sas_rle_measure_copy_run(size_t copy_run) {
    if (copy_run > 64) {
        return 2 + copy_run;
    }
    if (copy_run > 0) {
        return 1 + copy_run;
    }
    return 0;
}

static size_t sas_rle_measure_insert_run(unsigned char last_byte, size_t insert_run) {
    if (last_byte == '@' || last_byte == ' ' || last_byte == '\0') {
        if (insert_run > 17) {
            return 2;
        }
        return 1;
    }
    if (insert_run > 18) {
        return 3;
    }
    return 2;
}

static int sas_rle_is_insert_run(unsigned char last_byte, size_t insert_run) {
    if (last_byte == '@' || last_byte == ' ' || last_byte == '\0')
        return (insert_run > 1);

    return (insert_run > 2);
}

size_t sas_rle_compressed_len(const void *bytes, size_t len) {
    const unsigned char *p = (const unsigned char *)bytes;
    const unsigned char *pe = (const unsigned char *)bytes + len;
    size_t copy_run = 0;
    size_t insert_run = 0;
    char last_byte = 0;
    size_t rle_len = 0;
    while (p < pe) {
        unsigned char c = *p;
        if (insert_run == 0) {
            insert_run = 1;
        } else if (c == last_byte) {
            insert_run++;
        } else {
            if (sas_rle_is_insert_run(last_byte, insert_run)) {
                rle_len += sas_rle_measure_copy_run(copy_run);
                rle_len += sas_rle_measure_insert_run(last_byte, insert_run);
                copy_run = 0;
            } else {
                copy_run += insert_run;
            }
            insert_run = 1;
        }
        last_byte = c;
        p++;
    }
    if (sas_rle_is_insert_run(last_byte, insert_run)) {
        rle_len += sas_rle_measure_copy_run(copy_run);
        rle_len += sas_rle_measure_insert_run(last_byte, insert_run);
    } else {
        rle_len += sas_rle_measure_copy_run(copy_run + insert_run);
    }
    return rle_len;
}

size_t sas_rle_copy_run(unsigned char *output_buf, const unsigned char *copy, size_t copy_run) {
    unsigned char *out = output_buf;
    if (copy_run > 64) {
        int length = (copy_run - 64) / 256;
        unsigned char rem = (copy_run - 64) % 256;
        *out++ = (SAS_RLE_COMMAND_COPY64 << 4) + (length & 0x0F);
        *out++ = rem;
    } else if (copy_run >= 49) {
        *out++ = (SAS_RLE_COMMAND_COPY49 << 4) + (copy_run - 49);
    } else if (copy_run >= 33) {
        *out++ = (SAS_RLE_COMMAND_COPY33 << 4) + (copy_run - 33);
    } else if (copy_run >= 17) {
        *out++ = (SAS_RLE_COMMAND_COPY17 << 4) + (copy_run - 17);
    } else if (copy_run >= 1) {
        *out++ = (SAS_RLE_COMMAND_COPY1 << 4) + (copy_run - 1);
    }
    memcpy(out, copy, copy_run);
    out += copy_run;
    return out - output_buf;
}

size_t sas_rle_insert_run(unsigned char *output_buf, unsigned char last_byte, size_t insert_run) {
    unsigned char *out = output_buf;
    if (last_byte == '@' || last_byte == ' ' || last_byte == '\0') {
        if (insert_run > 17) {
            int length = (insert_run - 17) / 256;
            unsigned char rem = (insert_run - 17) % 256;
            if (last_byte == '@') {
                *out++ = (SAS_RLE_COMMAND_INSERT_AT17 << 4) + (length & 0x0F);
            } else if (last_byte == ' ') {
                *out++ = (SAS_RLE_COMMAND_INSERT_BLANK17 << 4) + (length & 0x0F);
            } else if (last_byte == '\0') {
                *out++ = (SAS_RLE_COMMAND_INSERT_ZERO17 << 4) + (length & 0x0F);
            }
            *out++ = rem;
        } else if (insert_run >= 2) {
            if (last_byte == '@') {
                *out++ = (SAS_RLE_COMMAND_INSERT_AT2 << 4) + (insert_run - 2);
            } else if (last_byte == ' ') {
                *out++ = (SAS_RLE_COMMAND_INSERT_BLANK2 << 4) + (insert_run - 2);
            } else if (last_byte == '\0') {
                *out++ = (SAS_RLE_COMMAND_INSERT_ZERO2 << 4) + (insert_run - 2);
            }
        }
    } else {
        if (insert_run > 18) {
            int length = (insert_run - 18) / 256;
            unsigned char rem = (insert_run - 18) % 256;
            *out++ = (SAS_RLE_COMMAND_INSERT_BYTE18 << 4) + (length & 0x0F);
            *out++ = rem;
            *out++ = last_byte;
        } else if (insert_run >= 3) {
            *out++ = (SAS_RLE_COMMAND_INSERT_BYTE3 << 4) + (insert_run - 3);
            *out++ = last_byte;
        }
    }
    return out - output_buf;
}

size_t sas_rle_compress(void *output_buf, size_t output_len,
        const void *input_buf, size_t input_len) {
    /* TODO bounds check */
    const unsigned char *p = (const unsigned char *)input_buf;
    const unsigned char *pe = p + input_len;
    const unsigned char *copy = p;

    unsigned char *out = (unsigned char *)output_buf;

    size_t insert_run = 0;
    size_t copy_run = 0;
    unsigned char last_byte = 0;

    while (p < pe) {
        unsigned char c = *p;
        if (insert_run == 0) {
            insert_run = 1;
        } else if (c == last_byte) {
            insert_run++;
        } else {
            if (sas_rle_is_insert_run(last_byte, insert_run)) {
                out += sas_rle_copy_run(out, copy, copy_run);
                out += sas_rle_insert_run(out, last_byte, insert_run);
                copy_run = 0;
                copy = p;
            } else {
                copy_run += insert_run;
            }
            insert_run = 1;
        }
        last_byte = c;
        p++;
    }

    if (sas_rle_is_insert_run(last_byte, insert_run)) {
        out += sas_rle_copy_run(out, copy, copy_run);
        out += sas_rle_insert_run(out, last_byte, insert_run);
    } else {
        out += sas_rle_copy_run(out, copy, copy_run + insert_run);
    }

    return out - (unsigned char *)output_buf;
}
