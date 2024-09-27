/*
 * Copyright (c) 2024, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#include "hash_table.h"

#include "stdlib.h"
#include "string.h"

#include "util/array_list.h"
#include "util/filesystem.h"
#include "util/logger.h"
#include "util/sysdefs.h"

#define _HASHFN _hash_djb2

static u64
_hash_djb2(unsigned char *str)
{
    unsigned long hash = 5381;
    int c;

    while (c = *str++)
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

    return hash;
}

HashTable
hash_table_init(u64 table_count)
{
    HashTable ret;
    ret.total_attribs = table_count;
    ret.p_attribs     = malloc(sizeof(HashTableAttrib) * ret.total_attribs);

    for (u64 i = 0; i < ret.total_attribs; i++)
    {
        ((HashTableAttrib *)&ret.p_attribs[i])->handle = 0;
        ((HashTableAttrib *)&ret.p_attribs[i])->p_next = NULL;
    }

    return ret;
}

void
hash_table_add(HashTable *p_table, const char *str_key, u64 value)
{
    u64 hash = _HASHFN(str_key);
    u64 idx  = hash % p_table->total_attribs;

    HashTableAttrib attrib = {
      .handle = value,
      .p_next = NULL,
    };
    attrib.uri = strdup(str_key);

    // check for chaining
    if (p_table->p_attribs[idx].handle != 0)
    {
        HashTableAttrib *p_last = (HashTableAttrib *)&p_table->p_attribs[idx];
        HashTableAttrib *p_next_loc = NULL;

        p_next_loc = ((HashTableAttrib *)&p_table->p_attribs[idx])->p_next;

        while (p_next_loc != NULL)
        {
            p_last     = p_next_loc;
            p_next_loc = p_next_loc->p_next;
        }

        HashTableAttrib *p_chain = malloc(sizeof(HashTableAttrib));
        *p_chain                 = attrib;
        p_last->p_next           = p_chain;
        return;
    }

    p_table->p_attribs[idx] = attrib;
}

u64
hash_table_get(HashTable *p_table, const char *str_key)
{
    u64 hash = _HASHFN(str_key);
    u64 idx  = hash % p_table->total_attribs;

    if (p_table->p_attribs[idx].handle == 0)
    {
        LOG_CRITICAL("Entry in hashtable does not exist! (%s)", str_key);
    }

    if (p_table->p_attribs[idx].handle != 0)
    {
        HashTableAttrib *p_last = (HashTableAttrib *)&p_table->p_attribs[idx];
        HashTableAttrib *p_next = p_last->p_next;

        while (p_next != NULL)
        {
            if (!strcmp(p_last->uri, str_key)) { return p_last->handle; }

            // ensure that the next item is valid
            if (p_next->handle == 0)
            {
                LOG_ERROR("Failed to reference chain in hashtable!");
            }

            p_last = p_next;
            p_next = p_last->p_next;
        }

        // one last check
        if (strcmp(p_last->uri, str_key))
        {
            LOG_ERROR("Failed to find attrib (%s) in hash_table", str_key);
            return 1;
        }

        // return the handle
        return p_last->handle;
    }
}