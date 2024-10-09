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

#include "asset/assetdefs.h"

#define GSK_ASSET_CACHE_INCREMENT 32
#define GSK_ASSET_CACHE_TABLE_MAX 1031

#define GSK_ASSET_HANDLE_LIST_NUM(x)  (u32)((x >> 56) & 0xFF)
#define GSK_ASSET_HANDLE_INDEX_NUM(x) (u32)(x & 0xFFFFFFFF)

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

typedef enum gsk_AssetCacheType {
    GSK_ASSET_CACHE_GCFG = 0,
    GSK_ASSET_CACHE_TEXTURE,
    GSK_ASSET_CACHE_MATERIAL,
    GSK_ASSET_CACHE_SHADER,
    GSK_ASSET_CACHE_MODEL,
} gsk_AssetCacheType;

#define ASSETTYPE_FIRST GSK_ASSET_CACHE_GCFG
#define ASSETTYPE_LAST  GSK_ASSET_CACHE_MODEL

typedef struct gsk_AssetList
{
    ArrayList list_state;   // load state
    ArrayList list_data;    // memory data
    ArrayList list_options; // asset options

} gsk_AssetList;

typedef struct gsk_AssetCache
{
    ArrayList asset_uri_list; // store for URI string pointers
    HashTable asset_table;    // asset handle hashtable
    gsk_AssetList asset_lists[ASSETTYPE_LAST + 1]; // asset data

} gsk_AssetCache;

gsk_AssetCache
gsk_asset_cache_init();

void
gsk_asset_cache_add(gsk_AssetCache *p_cache,
                    u32 asset_type,
                    const char *str_uri);

void
gsk_asset_cache_add_by_ext(gsk_AssetCache *p_cache, const char *str_uri);

gsk_AssetRef *
gsk_asset_cache_get(gsk_AssetCache *p_cache, const char *str_uri);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __ASSET_CACHE_H__