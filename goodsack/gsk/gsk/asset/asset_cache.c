/*
 * Copyright (c) 2024, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#include "asset_cache.h"

#include "util/array_list.h"
#include "util/filesystem.h"
#include "util/logger.h"
#include "util/sysdefs.h"

gsk_AssetCache
gsk_asset_cache_init()
{
    gsk_AssetCache ret;
    for (int i = 0; i < GSK_TOTAL_ASSET_TYPES; i++)
    {
        // TODO: Check size for asset types

        ret.asset_lists[i].list_data =
          array_list_init(sizeof(u32), GSK_ASSET_CACHE_INCREMENT);

        ret.asset_lists[i].list_options =
          array_list_init(sizeof(u32), GSK_ASSET_CACHE_INCREMENT);
    }

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