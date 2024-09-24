/*
 * Copyright (c) 2024, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#include "gpak.h"

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

gsk_GPAK
gsk_gpak_init(u64 table_count)
{
    gsk_GPAK ret;
    ret.refs_table_count = table_count;
    ret.p_refs_table = malloc(sizeof(gsk_GPakAssetRef) * ret.refs_table_count);

    for (u64 i = 0; i < ret.refs_table_count; i++)
    {
        (gsk_GPakAssetRef *)ret.p_refs_table[i].handle = 0;
        (gsk_GPakAssetRef *)ret.p_refs_table[i].p_next = NULL;
    }

    return ret;
}

void
gsk_gpak_write(gsk_GPAK *p_gpak, const char *str_key_uri, u64 value)
{
    u64 hash = _HASHFN(str_key_uri);
    u64 idx  = hash % p_gpak->refs_table_count;

    gsk_GPakAssetRef asset = {
      .handle = value,
      .p_next = NULL,
      .uri    = strdup(str_key_uri),
      .type   = 0,
    };

#if 1
    // check for chaining
    if (p_gpak->p_refs_table[idx].handle != 0)
    {
        gsk_GPakAssetRef *p_chain        = malloc(sizeof(gsk_GPakAssetRef));
        *p_chain                         = asset;
        p_gpak->p_refs_table[idx].p_next = p_chain;
        return;
    }
#endif

    p_gpak->p_refs_table[idx] = asset;
}

u64
gsk_gpak_read(gsk_GPAK *p_gpak, const char *str_uri)
{
    u64 hash = _HASHFN(str_uri);
    u64 idx  = hash % p_gpak->refs_table_count;

    if (p_gpak->p_refs_table[idx].handle == 0)
    {
        LOG_CRITICAL("Entry in hashtable does not exist! (%s)", str_uri);
    }

    if (p_gpak->p_refs_table[idx].handle != 0)
    {
        gsk_GPakAssetRef *p_next = p_gpak->p_refs_table[idx].p_next;
        if (p_next == NULL)
        {
            LOG_CRITICAL("Failed to reference chain in hashtable!");
        }
        return p_next->handle;
    }

    return (gsk_GPakAssetRef *)p_gpak->p_refs_table[idx].handle;
}