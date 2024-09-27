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

typedef struct gsk_GPakAsset
{
    u64 buffer_size;
    u8 is_on_ram, is_on_gpu;
    void *p_ram_buff, *p_gpu_buff;
    u8 type;

} gsk_GPakAsset;

#if 0
typedef struct gsk_GPakHeader
{
    byte_t signature[4];
    byte_t version[4];
    byte_t build_version[2];
    byte_t total_assets[4];
    byte_t file_date;

} gsk_GPakHeader;
#endif

// NOTE: Might want to have separate lists so that we can bundle same kinds of
// data?
typedef struct gsk_GPakAssetCollection
{
    ArrayList list_textures;

} gsk_GPakAssetCollection;

typedef struct gsk_GPakContainer
{
    gsk_GPakAssetRef *p_refs_table;
    u64 refs_table_count;

    u64 total_assets;

} gsk_GPAK;

// initialize the container cache
gsk_GPAK
gsk_gpak_init(u64 table_count);

// gsk_gpak_fill_from_directory() -- add assets recursively from a directory

// gsk_gpak_add_asset() -- add asset from C++ side to cache

// gsk_gpak_get_asset() -- get an asset from the container

// gsk_gpak_write_to_disk() -- write the entire Container to disk

/* NOTE: FileHandle should reference to some blob info. I.E., start_index,
    buffer_length, etc. */

// gsk_gpak_add_ref() (add_handle)
void
gsk_gpak_write(gsk_GPAK *p_gpak, const char *str_key_uri, u64 value);

// gsk_gpak_get_ref() (get_handle)
u64
gsk_gpak_read(gsk_GPAK *p_gpak, const char *str_uri);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __GPAK_H__