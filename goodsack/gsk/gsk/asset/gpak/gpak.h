/*
 * Copyright (c) 2024, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#ifndef __GPAK_H__
#define __GPAK_H__

#include "util/array_list.h"
#include "util/filesystem.h"
#include "util/sysdefs.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

typedef struct gsk_GPakAssetRef
{
    u64 handle;   // default to 0 when not in use
    u8 type;      // asset type
    char *uri;    // uri of the reference
    void *p_next; // chain pointer to next asset in linked-list
} gsk_GPakAssetRef;

#if 0
typedef struct gsk_GPakAssetBlob
{
    void *p_data;
    u64 data_size;

} gsk_GPakAssetBlob;
#endif

typedef struct gsk_GPakContainer
{
    gsk_GPakAssetRef *p_refs_table;
    u64 refs_table_count;
} gsk_GPAK;

gsk_GPAK
gsk_gpak_init(u64 table_count);

void
gsk_gpak_write(gsk_GPAK *p_gpak, const char *str_key_uri, u64 value);

u64
gsk_gpak_read(gsk_GPAK *p_gpak, const char *str_uri);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __GPAK_H__