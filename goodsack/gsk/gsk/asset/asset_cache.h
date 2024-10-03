/*
 * Copyright (c) 2024, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#ifndef __ASSET_CACHE_H__
#define __ASSET_CACHE_H__

#include "util/array_list.h"
#include "util/filesystem.h"
#include "util/hash_table.h"
#include "util/sysdefs.h"

#define GSK_TOTAL_ASSET_TYPES     2
#define GSK_ASSET_CACHE_INCREMENT 32
#define GSK_ASSET_CACHE_TABLE_MAX 1024

#define _GSK_ASSET_CACHE_GET_AT 0

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

typedef enum AssetType { TEXTURE = 0, MESH } AssetType;

typedef struct gsk_AssetCacheState
{
    u8 is_loaded;
    void *p_data;

} gsk_AssetCacheState;

typedef struct gsk_AssetList
{
    ArrayList list_state;   // load state
    ArrayList list_data;    // memory data
    ArrayList list_options; // asset options

} gsk_AssetList;

typedef struct gsk_AssetCache
{
    HashTable asset_table;
    gsk_AssetList asset_lists[GSK_TOTAL_ASSET_TYPES];

} gsk_AssetCache;

gsk_AssetCache
gsk_asset_cache_init();

void *
gsk_asset_cache_add(gsk_AssetCache *p_cache,
                    u32 asset_type,
                    const char *str_uri);

gsk_AssetCacheState *
gsk_asset_cache_get_at(gsk_AssetCache *p_cache,
                       u32 asset_type,
                       u32 asset_index);
gsk_AssetCacheState *
gsk_asset_cache_get(gsk_AssetCache *p_cache,
                    u32 asset_type,
                    const char *str_uri);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __ASSET_CACHE_H__