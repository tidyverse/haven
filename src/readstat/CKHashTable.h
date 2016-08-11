// CKHashTable - A simple hash table
// Copyright 2010 Evan Miller (see LICENSE)

#include <stdint.h>
#include <sys/types.h>

#define CK_HASH_KEY_SIZE     128

typedef struct ck_hash_entry_s {
	char key[CK_HASH_KEY_SIZE];
	const void *value;
} ck_hash_entry_t;

typedef struct ck_hash_table_s {
	uint64_t capacity;
    uint64_t count;
	ck_hash_entry_t *entries;
} ck_hash_table_t;

int ck_str_hash_insert(const char *key, const void *value, ck_hash_table_t *table);
const void *ck_str_hash_lookup(const char *key, ck_hash_table_t *table);

int ck_float_hash_insert(float key, const void *value, ck_hash_table_t *table);
const void *ck_float_hash_lookup(float key, ck_hash_table_t *table);

int ck_double_hash_insert(double key, const void *value, ck_hash_table_t *table);
const void *ck_double_hash_lookup(double key, ck_hash_table_t *table);

ck_hash_table_t *ck_hash_table_init(size_t size);
void ck_hash_table_wipe(ck_hash_table_t *table);
int ck_hash_table_grow(ck_hash_table_t *table);
void ck_hash_table_free(ck_hash_table_t *table);
uint64_t ck_hash_str(const char *str);
