/*
 * Copyright (c) 2024, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#ifndef __HASH_TABLE_H__
#define __HASH_TABLE_H__

#include "util/array_list.h"
#include "util/filesystem.h"
#include "util/sysdefs.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

typedef struct HashTableAttrib
{
    u64 handle;   // default to 0 when not in use
    char *uri;    // uri of the reference
    void *p_next; // chain pointer to next asset in linked-list
} HashTableAttrib;

typedef struct HashTable
{
    HashTableAttrib *p_attribs;
    u64 total_attribs;
} HashTable;

HashTable
hash_table_init(u64 table_count);

void
hash_table_add(HashTable *p_table, const char *str_key_uri, u64 value);

u64
hash_table_get(HashTable *p_table, const char *str_key_uri);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __HASH_TABLE_H__