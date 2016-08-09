// CKHashTable - A simple hash table
// Copyright 2010 Evan Miller (see LICENSE)

#include "CKHashTable.h"
#include <string.h>
#include <stdlib.h>

int ck_str_n_hash_insert(const char *key, size_t keylen, const void *value, ck_hash_table_t *table);
const void *ck_str_n_hash_lookup(const char *key, size_t keylen, ck_hash_table_t *table);
static inline void ck_float2str(float key, char keystr[6]);
static inline void ck_double2str(double key, char keystr[11]);

inline uint64_t ck_hash_str(const char *str)
{
	uint64_t hash = 5381;
	int c;
	
	while ((c = *str++))
		hash = ((hash << 5) + hash) + c;
	
	return hash;
}

static inline void ck_float2str(float key, char keystr[6]) {
    memcpy(keystr, &key, 4);
    keystr[4] = (0xF0 | (keystr[0] & 0x01) | (keystr[1] & 0x02) | (keystr[2] & 0x04) | (keystr[3] & 0x08));
    keystr[0] |= 0x01;
    keystr[1] |= 0x02;
    keystr[2] |= 0x04;
    keystr[3] |= 0x08;
    
    keystr[5] = 0x00;
}

static inline void ck_double2str(double key, char keystr[11]) {
    memcpy(keystr, &key, 8);
    keystr[8] = (0xF0 | (keystr[0] & 0x01) | (keystr[1] & 0x02) | (keystr[2] & 0x04) | (keystr[3] & 0x08));
    keystr[0] |= 0x01;
    keystr[1] |= 0x02;
    keystr[2] |= 0x04;
    keystr[3] |= 0x08;
    
    keystr[9] = (0xF0 | (keystr[4] & 0x01) | (keystr[5] & 0x02) | (keystr[6] & 0x04) | (keystr[7] & 0x08));
    keystr[4] |= 0x01;
    keystr[5] |= 0x02;
    keystr[6] |= 0x04;
    keystr[7] |= 0x08;
    
    keystr[10] = 0x00;
}

const void *ck_float_hash_lookup(float key, ck_hash_table_t *table)
{
    char keystr[6];
    ck_float2str(key, keystr);
    return ck_str_n_hash_lookup(keystr, 5, table);
}

int ck_float_hash_insert(float key, const void *value, ck_hash_table_t *table) {
    char keystr[6];
    ck_float2str(key, keystr);
    return ck_str_n_hash_insert(keystr, 5, value, table);
}

const void *ck_double_hash_lookup(double key, ck_hash_table_t *table)
{
    char keystr[11];
    ck_double2str(key, keystr);
    return ck_str_n_hash_lookup(keystr, 10, table);
}

int ck_double_hash_insert(double key, const void *value, ck_hash_table_t *table) {
    char keystr[11];
    ck_double2str(key, keystr);
    return ck_str_n_hash_insert(keystr, 10, value, table);
}

const void *ck_str_hash_lookup(const char *key, ck_hash_table_t *table) {
    size_t keylen = strlen(key);
    if (keylen >= CK_HASH_KEY_SIZE)
        keylen = CK_HASH_KEY_SIZE-1;
    return ck_str_n_hash_lookup(key, keylen, table);
}

const void *ck_str_n_hash_lookup(const char *key, size_t keylen, ck_hash_table_t *table)
{
	if (table->count == 0)
		return NULL;
    	
	if (keylen == 0 || keylen >= CK_HASH_KEY_SIZE)
		return NULL;
	
	uint64_t hash_key = ck_hash_str(key);
	hash_key %= table->capacity;
	uint64_t end = (hash_key - 1) % table->capacity;
	while (hash_key != end && table->entries[hash_key].key[0] != '\0') {
		if (strncmp(table->entries[hash_key].key, key, keylen + 1) == 0) {
			return table->entries[hash_key].value;
		}
		hash_key++;
		hash_key %= table->capacity;
	}
	return NULL;
}

int ck_str_hash_insert(const char *key, const void *value, ck_hash_table_t *table)
{
    size_t keylen = strlen(key);
    if (keylen >= CK_HASH_KEY_SIZE)
        keylen = CK_HASH_KEY_SIZE-1;
    return ck_str_n_hash_insert(key, keylen, value, table);
}

int ck_str_n_hash_insert(const char *key, size_t keylen, const void *value, ck_hash_table_t *table)
{    
	if (table->capacity == 0)
		return 0;
    	
	if (keylen == 0 || keylen >= CK_HASH_KEY_SIZE)
		return 0;
    
    if (table->count >= 0.75 * table->capacity) {
        if (ck_hash_table_grow(table) == -1) {
            return 0;
        }
    }
	
	uint64_t hash_key = ck_hash_str(key);
	hash_key %= table->capacity;
	uint64_t end = (hash_key - 1) % table->capacity;
	while (hash_key != end) {
        if (table->entries[hash_key].key[0] == '\0') {
            table->count++;
        }
		if (table->entries[hash_key].key[0] == '\0' ||
			strncmp(table->entries[hash_key].key, key, keylen + 1) == 0) {
			memcpy(table->entries[hash_key].key, key, keylen);
            memset(table->entries[hash_key].key + keylen, 0, 1);
			table->entries[hash_key].value = value;
			return 1;
		}
		hash_key++;
		hash_key %= table->capacity;
	}
	return 0;
}

ck_hash_table_t *ck_hash_table_init(size_t size)
{
	ck_hash_table_t *table;
	if ((table = malloc(sizeof(ck_hash_table_t))) == NULL)
		return NULL;
    if ((table->entries = malloc(size * sizeof(ck_hash_entry_t))) == NULL) {
        free(table);
        return NULL;
    }
	table->capacity = size;
    table->count = 0;
    ck_hash_table_wipe(table);
	return table;
}

void ck_hash_table_free(ck_hash_table_t *table) {
    free(table->entries);
    free(table);
}

void ck_hash_table_wipe(ck_hash_table_t *table) {
    memset(table->entries, 0, table->capacity * sizeof(ck_hash_entry_t));
}

int ck_hash_table_grow(ck_hash_table_t *table) {
    ck_hash_entry_t *old_entries = table->entries;
    uint64_t old_capacity = table->capacity;
    uint64_t new_capacity = 2 * table->capacity;
    int i;
    if ((table->entries = calloc(new_capacity, sizeof(ck_hash_entry_t))) == NULL) {
        return -1;
    }
    table->capacity = new_capacity;
    table->count = 0;
    for (i=0; i<old_capacity; i++) {
        if (old_entries[i].key[0] !='\0') {
            if (!ck_str_hash_insert(old_entries[i].key, old_entries[i].value, table))
                return -1;
        }
    }
    free(old_entries);
    return 0;
}
