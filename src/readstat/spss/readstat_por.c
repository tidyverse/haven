#include <stdlib.h>
#include <math.h>
#include <float.h>
#include <iconv.h>

#include "../readstat.h"
#include "../CKHashTable.h"
#include "../readstat_convert.h"

#include "readstat_spss.h"
#include "readstat_por.h"
#include "readstat_por_parse.h"

int8_t por_ascii_lookup[256] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, '0', '1', '2', '3', '4', '5', 
    '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F', 
    'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 
    'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j',
    'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't',
    'u', 'v', 'w', 'x', 'y', 'z', ' ', '.', '<', '(',
    '+', '|', '&', '[', ']', '!', '$', '*', ')', ';',
    '^', '-', '/', '|', ',', '%', '_', '>', '?', '`',
    ':', '#', '@', '\'', '=', '"', 0, 0, 0, 0,
    0, 0, '~', 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, '{', '}', '\\', 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0 };

/* Unicode code points for the portable character set, indexed by position
 * in the file's 256-byte translation table. See "Portable File Characters"
 * in the PSPP developer documentation: 131 is the solid vertical pipe, 143
 * the broken vertical pipe, 151 the pound sign, and 167-176 are the
 * superscript digits 0-9. */
uint16_t por_unicode_lookup[256] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, '0', '1', '2', '3', '4', '5', 
    '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F', 
    'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 
    'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j',
    'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't',
    'u', 'v', 'w', 'x', 'y', 'z', ' ', '.', '<', '(',
    '+', '|', '&', '[', ']', '!', '$', '*', ')', ';',
    '^', '-', '/', 0x00A6, ',', '%', '_', '>', '?', 0x2018,
    ':', 0x00A3, '@', 0x2019, '=', '"', 0x2264, 0x25A1, 0x00B1, 0x25A0,
    0x00B0, 0x2020, '~', 0x2013, 0x2514, 0x250C, 0x2265, 0x2070, 0x00B9, 0x00B2,
    0x00B3, 0x2074, 0x2075, 0x2076, 0x2077, 0x2078, 0x2079, 0x2518, 0x2510, 0x2260,
    0x2014, 0x207D, 0x207E, 0x2E38, '{', '}', '\\', 0x00A2, 0x2022, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0 };

por_ctx_t *por_ctx_init(void) {
    por_ctx_t *ctx = calloc(1, sizeof(por_ctx_t));

    ctx->space = ' ';
    ctx->base30_precision = 20;
    ctx->var_dict = ck_hash_table_init(1024, 8);
    return ctx;
}

void por_ctx_free(por_ctx_t *ctx) {
    if (ctx->string_buffer)
        free(ctx->string_buffer);
    if (ctx->varinfo) {
        int i;
        for (i=0; i<ctx->var_count; i++) {
            if (ctx->varinfo[i].label)
                free(ctx->varinfo[i].label);
        }
        free(ctx->varinfo);
    }
    if (ctx->variables) {
        int i;
        for (i=0; i<ctx->var_count; i++) {
            if (ctx->variables[i])
                free(ctx->variables[i]);
        }
        free(ctx->variables);
    }
    if (ctx->var_dict)
        ck_hash_table_free(ctx->var_dict);
    if (ctx->converter)
        iconv_close(ctx->converter);
    free(ctx);
}

/* Decodes one UTF-8 sequence. Returns the number of bytes consumed, or 0 if
 * the input does not start with a well-formed sequence. */
static size_t por_utf8_read_char(const unsigned char *input, size_t input_len, uint32_t *out_codepoint) {
    unsigned char lead = input[0];
    uint32_t codepoint = 0;
    size_t char_len = 0;
    size_t i;

    if (lead < 0x80) {
        *out_codepoint = lead;
        return 1;
    } else if ((lead & 0xE0) == 0xC0) {
        char_len = 2;
        codepoint = lead & 0x1F;
    } else if ((lead & 0xF0) == 0xE0) {
        char_len = 3;
        codepoint = lead & 0x0F;
    } else if ((lead & 0xF8) == 0xF0) {
        char_len = 4;
        codepoint = lead & 0x07;
    } else {
        return 0;
    }
    if (input_len < char_len)
        return 0;

    for (i=1; i<char_len; i++) {
        if ((input[i] & 0xC0) != 0x80)
            return 0;
        codepoint = (codepoint << 6) | (input[i] & 0x3F);
    }
    if ((char_len == 2 && codepoint < 0x80) ||
            (char_len == 3 && codepoint < 0x800) ||
            (char_len == 4 && codepoint < 0x10000) ||
            codepoint > 0x10FFFF ||
            (codepoint >= 0xD800 && codepoint <= 0xDFFF)) {
        return 0; /* overlong, surrogate, or out of range */
    }
    *out_codepoint = codepoint;
    return char_len;
}

ssize_t por_utf8_count(const char *input, size_t input_len) {
    const unsigned char *bytes = (const unsigned char *)input;
    size_t offset = 0;
    ssize_t count = 0;
    while (offset < input_len) {
        uint32_t codepoint = 0;
        size_t char_len = por_utf8_read_char(bytes + offset, input_len - offset, &codepoint);
        if (char_len == 0)
            return -1;
        offset += char_len;
        count++;
    }
    return count;
}

ssize_t por_utf8_encode(const unsigned char *input, size_t input_len,
        char *output, size_t output_len, uint16_t lookup[256]) {
    size_t offset = 0;
    size_t i;
    for (i=0; i<input_len; i++) {
        uint16_t codepoint = lookup[input[i]];

        /* Some software (SPSS?) uses an undefined character (zero) if a
         * character can't be encoded. Use Unicode replacement character */
        if (codepoint == 0) {
            codepoint = 0xFFFD;
        }

        if (codepoint < 0x20) {
            return -1;
        } else if (codepoint <= 0x7F) {
            if (offset + 1 > output_len)
                return offset;

            output[offset++] = codepoint;
        } else if (codepoint <= 0x07FF) {
            if (offset + 2 > output_len)
                return offset;

            output[offset++] = 0xC0 | (codepoint >> 6);
            output[offset++] = 0x80 | (codepoint & 0x3F);
        } else {
            if (offset + 3 > output_len)
                return offset;

            output[offset++] = 0xE0 | (codepoint >> 12);
            output[offset++] = 0x80 | ((codepoint >> 6) & 0x3F);
            output[offset++] = 0x80 | (codepoint & 0x3F);
        }
    }
    return offset;
}

ssize_t por_utf8_decode(
        const char *input, size_t input_len,
        char *output, size_t output_len,
        uint8_t *lookup, size_t lookup_len) {
    const unsigned char *bytes = (const unsigned char *)input;
    size_t in_offset = 0;
    size_t out_offset = 0;
    while (in_offset < input_len) {
        uint32_t codepoint = 0;
        size_t char_len = por_utf8_read_char(bytes + in_offset, input_len - in_offset, &codepoint);
        if (char_len == 0)
            return -1;
        if (codepoint >= lookup_len || lookup[codepoint] == 0)
            return -1;
        if (out_offset + 1 > output_len)
            return -1;

        output[out_offset++] = lookup[codepoint];
        in_offset += char_len;
    }
    return out_offset;
}

/* Arbitrary-precision arithmetic for exact base-30 <-> binary conversion.
 *
 * A double is m * 2^e with m an integer below 2^53. For e >= 0 the exact
 * base-30 expansion is that of the integer m * 2^e. For e < 0 it is the
 * expansion of the integer m * 15^(-e) with the radix point -e places from
 * the right, because 1/2 = 15/30. Both are computed with the bignum below,
 * which stores base-30^6 limbs so that the trigesimal digits fall straight
 * out of the limbs. The same bignum lets the reader compare a parsed value
 * against the midpoints between candidate doubles, which is what makes the
 * text -> double conversion correctly rounded. */

#define POR_BIG_LIMB_BASE   729000000u  /* 30^6 */
#define POR_BIG_TRIGS_PER_LIMB 6
#define POR_BIG_LIMBS       200         /* 1200 trigesimals; a denormal needs ~870 */

typedef struct por_bignum_s {
    uint32_t limbs[POR_BIG_LIMBS];  /* little-endian */
    int      n;                     /* limbs in use; zero is n == 0 */
    int      overflow;
} por_bignum_t;

static void por_big_set(por_bignum_t *big, uint64_t value) {
    big->n = 0;
    big->overflow = 0;
    while (value) {
        big->limbs[big->n++] = (uint32_t)(value % POR_BIG_LIMB_BASE);
        value /= POR_BIG_LIMB_BASE;
    }
}

/* big = big * mult + add, with mult < 2^32 and add < 2^32 */
static void por_big_mul_add(por_bignum_t *big, uint32_t mult, uint32_t add) {
    uint64_t carry = add;
    int i;
    for (i=0; i<big->n; i++) {
        uint64_t product = (uint64_t)big->limbs[i] * mult + carry;
        big->limbs[i] = (uint32_t)(product % POR_BIG_LIMB_BASE);
        carry = product / POR_BIG_LIMB_BASE;
    }
    while (carry) {
        if (big->n == POR_BIG_LIMBS) {
            big->overflow = 1;
            return;
        }
        big->limbs[big->n++] = (uint32_t)(carry % POR_BIG_LIMB_BASE);
        carry /= POR_BIG_LIMB_BASE;
    }
}

/* big *= base^exponent, multiplying by the largest power of base that fits
 * in a 32-bit limb multiplier at each step */
static void por_big_mul_pow(por_bignum_t *big, uint32_t base, long exponent) {
    uint32_t chunk_mult = 1;
    long chunk = 0;
    while ((uint64_t)chunk_mult * base <= 0xFFFFFFFFu) {
        chunk_mult *= base;
        chunk++;
    }
    while (exponent >= chunk && !big->overflow) {
        por_big_mul_add(big, chunk_mult, 0);
        exponent -= chunk;
    }
    if (exponent > 0) {
        uint32_t mult = 1;
        while (exponent-- > 0)
            mult *= base;
        por_big_mul_add(big, mult, 0);
    }
}

static int por_big_compare(const por_bignum_t *a, const por_bignum_t *b) {
    int i;
    if (a->n != b->n)
        return a->n > b->n ? 1 : -1;
    for (i=a->n-1; i>=0; i--) {
        if (a->limbs[i] != b->limbs[i])
            return a->limbs[i] > b->limbs[i] ? 1 : -1;
    }
    return 0;
}

int por_double_to_trigs(double value, unsigned char *trigs, int max_trigs, int *out_trig_places) {
    por_bignum_t big;
    int exp2 = 0;
    double significand = frexp(fabs(value), &exp2);
    uint64_t mantissa = (uint64_t)ldexp(significand, 53);
    long exponent = exp2 - 53; /* |value| == mantissa * 2^exponent exactly */
    int n_trigs = 0;
    int i, j;

    if (mantissa == 0)
        return -1;

    while ((mantissa & 1) == 0) {
        mantissa >>= 1;
        exponent++;
    }

    por_big_set(&big, mantissa);
    if (exponent >= 0) {
        por_big_mul_pow(&big, 2, exponent);
    } else {
        por_big_mul_pow(&big, 15, -exponent);
    }
    if (big.overflow)
        return -1;

    for (i=big.n-1; i>=0; i--) {
        uint32_t limb = big.limbs[i];
        unsigned char limb_trigs[POR_BIG_TRIGS_PER_LIMB];
        for (j=POR_BIG_TRIGS_PER_LIMB-1; j>=0; j--) {
            limb_trigs[j] = limb % 30;
            limb /= 30;
        }
        for (j=0; j<POR_BIG_TRIGS_PER_LIMB; j++) {
            if (n_trigs == 0 && limb_trigs[j] == 0)
                continue; /* leading zero */
            if (n_trigs == max_trigs)
                return -1;
            trigs[n_trigs++] = limb_trigs[j];
        }
    }

    if (exponent >= 0) {
        *out_trig_places = n_trigs;
    } else {
        *out_trig_places = n_trigs + exponent;
    }
    return n_trigs;
}

/* Sign of (digits as an integer) * 30^exp30 - odd_mantissa * 2^exp2.
 * Returns 2 if the comparison could not be carried out. */
static int por_compare_with_binary(const unsigned char *digits, size_t n_digits, long exp30,
        uint64_t mantissa, long exp2) {
    por_bignum_t lhs, rhs;
    size_t i;
    uint32_t chunk = 0;
    uint32_t chunk_mult = 1;

    por_big_set(&lhs, 0);
    for (i=0; i<n_digits; i++) {
        chunk = chunk * 30 + digits[i];
        chunk_mult *= 30;
        if (chunk_mult == POR_BIG_LIMB_BASE) {
            por_big_mul_add(&lhs, chunk_mult, chunk);
            chunk = 0;
            chunk_mult = 1;
        }
    }
    if (chunk_mult > 1)
        por_big_mul_add(&lhs, chunk_mult, chunk);

    por_big_set(&rhs, mantissa);

    if (exp30 > 0) {
        por_big_mul_pow(&lhs, 30, exp30);
    } else if (exp30 < 0) {
        por_big_mul_pow(&rhs, 30, -exp30);
    }
    if (exp2 > 0) {
        por_big_mul_pow(&rhs, 2, exp2);
    } else if (exp2 < 0) {
        por_big_mul_pow(&lhs, 2, -exp2);
    }
    if (lhs.overflow || rhs.overflow)
        return 2;

    return por_big_compare(&lhs, &rhs);
}

double por_base30_to_double(const unsigned char *digits, size_t n_digits, long exponent) {
    uint64_t mantissa = 0;
    size_t mantissa_digits = 0;
    long magnitude = 0;
    long scale = 0;
    double value = 0.0;
    int iterations = 0;

    while (n_digits > 0 && digits[0] == 0) {
        digits++;
        n_digits--;
    }
    while (n_digits > 0 && digits[n_digits-1] == 0) {
        n_digits--;
        exponent++;
    }
    if (n_digits == 0)
        return 0.0;

    /* The value lies in [30^(magnitude-1), 30^magnitude) */
    magnitude = (long)n_digits + exponent;
    if (magnitude >= 210) /* 30^209 exceeds DBL_MAX */
        return HUGE_VAL;
    if (magnitude <= -225) /* 30^-225 is below half the smallest denormal */
        return 0.0;

    /* Initial approximation from the leading 13 digits (30^13 < 2^64) */
    while (mantissa_digits < n_digits && mantissa_digits < 13) {
        mantissa = mantissa * 30 + digits[mantissa_digits];
        mantissa_digits++;
    }
    scale = exponent + (long)(n_digits - mantissa_digits);

    if (mantissa <= (1ULL << 53) && scale >= -13 && scale <= 13) {
        /* Both operands are exact, so the single multiplication or division
         * below is correctly rounded (30^13 = 2^13 * 15^13 and 15^13 < 2^53) */
        double pow30 = 1.0;
        long k = scale < 0 ? -scale : scale;
        while (k-- > 0)
            pow30 *= 30.0;
        return scale < 0 ? (double)mantissa / pow30 : (double)mantissa * pow30;
    }

    value = (double)mantissa;
    while (scale > 0) {
        long step = scale > 200 ? 200 : scale;
        value *= pow(30.0, (double)step);
        scale -= step;
    }
    while (scale < 0) {
        long step = scale < -200 ? -200 : scale;
        value *= pow(30.0, (double)step);
        scale -= step;
    }
    if (isinf(value))
        value = DBL_MAX;

    /* Correct the approximation by comparing the exact value against the
     * midpoints between the candidate and its neighbors, moving one ulp at a
     * time. Ties round to even. */
    for (iterations=0; iterations<256; iterations++) {
        uint64_t m = 0;
        long e = -1074;
        uint64_t midpoint_mantissa = 0;
        long midpoint_exponent = 0;
        int cmp = 0;

        if (isinf(value))
            break;

        if (value != 0.0) {
            int exp2 = 0;
            double significand = frexp(value, &exp2);
            m = (uint64_t)ldexp(significand, 53);
            e = exp2 - 53;
            if (e < -1074) {
                /* denormal: the spacing is fixed at 2^-1074 */
                m = (uint64_t)ldexp(value, 1074);
                e = -1074;
            }
        }

        /* midpoint above: (2m + 1) * 2^(e - 1) */
        midpoint_mantissa = 2 * m + 1;
        midpoint_exponent = e - 1;
        cmp = por_compare_with_binary(digits, n_digits, exponent, midpoint_mantissa, midpoint_exponent);
        if (cmp == 2)
            break;
        if (cmp > 0) {
            value = nextafter(value, HUGE_VAL);
            continue;
        }
        if (cmp == 0) {
            if (m & 1)
                value = nextafter(value, HUGE_VAL);
            break;
        }
        if (m == 0)
            break;

        /* midpoint below: the spacing halves just below a power of two */
        if (m == (1ULL << 52) && e > -1074) {
            midpoint_mantissa = 4 * m - 1;
            midpoint_exponent = e - 2;
        } else {
            midpoint_mantissa = 2 * m - 1;
            midpoint_exponent = e - 1;
        }
        cmp = por_compare_with_binary(digits, n_digits, exponent, midpoint_mantissa, midpoint_exponent);
        if (cmp == 2)
            break;
        if (cmp < 0) {
            value = nextafter(value, 0.0);
            continue;
        }
        if (cmp == 0) {
            /* value's mantissa is 2m in the finer grid below a power of two,
             * which is even, so only the ordinary case can move */
            if (midpoint_mantissa == 2 * m - 1 && (m & 1))
                value = nextafter(value, 0.0);
            break;
        }
        break;
    }

    return value;
}
