/*
 * Copyright (c) 2024, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#include "asset_cache.h"

#include "util/array_list.h"
#include "util/filesystem.h"
#include "util/hash_table.h"
#include "util/logger.h"
#include "util/sysdefs.h"

#include "core/graphics/texture/texture.h"

gsk_AssetCache
gsk_asset_cache_init()
{
    gsk_AssetCache ret;

    // setup hash table
    ret.asset_table = hash_table_init(GSK_ASSET_CACHE_TABLE_MAX);

    // setup  asset lists
    ret.asset_lists[0].list_data =
      array_list_init(sizeof(gsk_Texture), GSK_ASSET_CACHE_INCREMENT);
    ret.asset_lists[0].list_options =
      array_list_init(sizeof(TextureOptions), GSK_ASSET_CACHE_INCREMENT);
    // TODO: Other types
    // TODO: Handle specific file types elsewhere.

    return ret;
}

void *
gsk_asset_cache_get(gsk_AssetCache *p_cache, u32 asset_type, u32 asset_index)
{
    if (asset_type > GSK_TOTAL_ASSET_TYPES)
    {
        LOG_CRITICAL("Attempt to access invalid asset type (%u is not valid)",
                     asset_type);
    }

    u8 not_loaded = TRUE;

    if (not_loaded)
    {
        switch (asset_type)
        {
            // case TEXTURE: texture_create(GSK_PATH())
        }
    }

#if 0
    ArrayList list_data;

    if (p_cache->asset_lists[asset_type]) }
#endif
}

void *
gsk_asset_cache_add(gsk_AssetCache *p_cache,
                    u32 asset_type,
                    const char *str_uri)
{
    if (asset_type > GSK_TOTAL_ASSET_TYPES)
    {
        LOG_CRITICAL("Attempt to access invalid asset type (%u is not valid)",
                     asset_type);
    }

    gsk_Texture *test;

    // add empty data
    array_list_push(&(p_cache->asset_lists[asset_type].list_data), &test);

    // add the handle to the hash table (after it is incremented)
    u64 handle = p_cache->asset_lists[asset_type].list_data.list_next;
    hash_table_add((HashTable *)&(p_cache->asset_table), str_uri, handle);
}