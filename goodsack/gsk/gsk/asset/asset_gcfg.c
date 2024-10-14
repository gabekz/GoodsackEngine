/*
 * Copyright (c) 2024, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#include "asset_gcfg.h"

#include <stdlib.h>
#include <string.h>

#include "asset/asset_cache.h"
#include "asset/assetdefs.h"
#include "core/graphics/texture/texture.h"
#include "runtime/gsk_runtime_wrapper.h"

void
gsk_asset_gcfg_set_config(gsk_GCFG *p_gcfg)
{
    void *p_options = NULL;

    for (int i = 0; i < p_gcfg->list_items.list_next; i++)
    {
        gsk_AssetRef *p_ref = NULL;

        gsk_GCFGItem *p_item =
          array_list_get_at_index(&(p_gcfg->list_items), i);

        if (!strcmp(p_item->key, "path"))
        {
            LOG_DEBUG("GCFG - Setting config for: %s", p_item->value);

            gsk_AssetCache *p_cache = gsk_runtime_get_asset_cache();

            // get the asset ref
            p_ref = gsk_asset_cache_get(p_cache, p_item->value);

            u32 asset_list  = GSK_ASSET_HANDLE_LIST_NUM(p_ref->asset_handle);
            u32 asset_index = GSK_ASSET_HANDLE_INDEX_NUM(p_ref->asset_handle);

            if (asset_list == GSK_ASSET_CACHE_TEXTURE)
            {
                // Get the pre-allocated memory location from cache
                p_options = array_list_get_at_index(
                  &(p_cache->asset_lists[asset_list].list_options),
                  asset_index - 1);
            }
        }
    }

    if (p_options != NULL)
    {
        for (int i = 0; i < p_gcfg->list_items.list_next; i++)
        {
            gsk_GCFGItem *p_item =
              array_list_get_at_index(&(p_gcfg->list_items), i);

            TextureOptions *p_ops = (TextureOptions *)p_options;

            if (!strcmp(p_item->key, "is_normal"))
            {
                p_ops->af_range        = 1;
                p_ops->internal_format = GL_RGB;
                p_ops->gen_mips        = TRUE;
                p_ops->flip_vertically = TRUE;
            }
        }
    }
}