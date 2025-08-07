/*
 * Copyright (c) 2024-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#include "asset_gcfg.h"

#include <stdlib.h>
#include <string.h>

#include "asset/asset_cache.h"
#include "asset/assetdefs.h"
#include "core/graphics/texture/texture.h"
#include "runtime/gsk_runtime_wrapper.h"

static u8
_parse_texture_ops(gsk_GCFGItem *p_item, TextureOptions *p_dest)
{
    if (!strcmp(p_item->key, "is_normal"))
    {
        p_dest->af_range        = 1;
        p_dest->internal_format = GL_RGB;
        p_dest->gen_mips        = TRUE;
        p_dest->flip_vertically = TRUE;
    }

    else if (!strcmp(p_item->key, "flip_vertically"))
    {
        if (GCFG_CHECK(p_item, GskGCFGItemType_Number))
        {
            p_dest->flip_vertically = atof(p_item->value);
        }
    }

    return 1;
}

static u8
_parse_model_ops(gsk_GCFGItem *p_item, gsk_AssetModelOptions *p_dest)
{
    if (!strcmp(p_item->key, "scale"))
    {
        p_dest->scale = atof(p_item->value);
    }

    else if (!strcmp(p_item->key, "import_materials"))
    {
        p_dest->import_materials = 1;
    }

    return 1;
}

void
gsk_asset_gcfg_set_config(gsk_GCFG *p_gcfg)
{
    void *p_options = NULL;
    u32 asset_list  = 0;
    u32 asset_index = 0;

    for (int i = 0; i < p_gcfg->list_items.list_next; i++)
    {
        gsk_AssetRef *p_ref = NULL;

        gsk_GCFGItem *p_item =
          array_list_get_at_index(&(p_gcfg->list_items), i);

        if (!strcmp(p_item->key, "path"))
        {
            LOG_TRACE("GCFG - Setting config for: %s", p_item->value);

            gsk_AssetCache *p_cache =
              gsk_runtime_get_asset_cache(p_item->value);

            // get the asset ref
            p_ref = gsk_asset_cache_get(p_cache, p_item->value);

            if (p_ref == NULL)
            {
                LOG_ERROR("GCFG - Failed to get path (%s)", p_item->value);
                return;
            }

            asset_list  = GSK_ASSET_HANDLE_LIST_NUM(p_ref->asset_handle);
            asset_index = GSK_ASSET_HANDLE_INDEX_NUM(p_ref->asset_handle);

            // Get the pre-allocated memory location from cache
            p_options = array_list_get_at_index(
              &(p_cache->asset_lists[asset_list].list_options),
              asset_index - 1);
        }
    }

    if (p_options == NULL) { return; }

    for (int i = 0; i < p_gcfg->list_items.list_next; i++)
    {
        gsk_GCFGItem *p_item =
          array_list_get_at_index(&(p_gcfg->list_items), i);

        switch (asset_list)
        {
        case (GskAssetType_Texture):
            _parse_texture_ops(p_item, (TextureOptions *)p_options);
            break;
        case (GskAssetType_Model):
            _parse_model_ops(p_item, (TextureOptions *)p_options);
            break;
        default: break;
        }
    }
}

u8
gsk_asset_gcfg_check_type(const gsk_GCFGItem *p_item,
                          const GskGCFGItemType type)
{
    if (p_item->type == type) { return 1; }
    return 0;
}