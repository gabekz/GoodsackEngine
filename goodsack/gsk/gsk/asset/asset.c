/*
 * Copyright (c) 2024, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#include "asset.h"

#include "util/filesystem.h"
#include "util/logger.h"
#include "util/sysdefs.h"

#include "core/graphics/material/material.h"
#include "core/graphics/texture/texture.h"

#include "asset/asset_gcfg.h"
#include "asset/import/loader_gcfg.h"

#define TEXTURE_TYPE  0
#define MATERIAL_TYPE 2

static gsk_Texture *
_gsk_asset_load_texture(gsk_AssetCache *p_cache,
                        u64 asset_handle,
                        const char *str_uri)
{
    u32 asset_list  = GSK_ASSET_HANDLE_LIST_NUM(asset_handle);
    u32 asset_index = GSK_ASSET_HANDLE_INDEX_NUM(asset_handle);

    if (asset_list != TEXTURE_TYPE)
    {
        // TODO: better logging
        LOG_CRITICAL("Asset handle is for incorrect asset type.");
    }

    // TODO: fetch options somehow
    TextureOptions ops = (TextureOptions) {1, GL_RGB, TRUE, TRUE};

    // TODO: texture_create without allocating!!
    gsk_Texture *tex = texture_create(GSK_PATH(str_uri), NULL, ops);

    gsk_Texture *p_data = (gsk_Texture *)array_list_get_at_index(
      &(p_cache->asset_lists[TEXTURE_TYPE].list_data), asset_index - 1);

    p_data->id         = tex->id;
    p_data->width      = tex->width;
    p_data->activeSlot = tex->activeSlot;
    p_data->bpp        = tex->bpp;
    p_data->filePath   = NULL;

    return p_data;
}

static gsk_Material *
_gsk_asset_load_material(gsk_AssetCache *p_cache,
                         u64 asset_handle,
                         const char *str_uri)
{
    u32 asset_list  = GSK_ASSET_HANDLE_LIST_NUM(asset_handle);
    u32 asset_index = GSK_ASSET_HANDLE_INDEX_NUM(asset_handle);

    if (asset_list != MATERIAL_TYPE)
    {
        // TODO: better logging
        LOG_CRITICAL("Asset handle is for incorrect asset type.");
    }

    gsk_GCFG gcfg = gsk_load_gcfg(GSK_PATH(str_uri));

    return NULL;
}

static void *
_gsk_asset_get(gsk_AssetCache *p_cache, const char *str_uri)
{
    // check if the asset has been added already
    // -- if not, exit

    gsk_AssetCacheState *p_state = gsk_asset_cache_get(p_cache, str_uri);
    void *data_ret;

    if (p_state == NULL) { LOG_CRITICAL("Failed to get asset (%s)", str_uri); }

    u32 asset_type  = GSK_ASSET_HANDLE_LIST_NUM(p_state->asset_handle);
    u32 asset_index = GSK_ASSET_HANDLE_INDEX_NUM(p_state->asset_handle);

    if (p_state->is_loaded == FALSE)
    {
        LOG_DEBUG("asset not yet loaded. loading asset (%s)", str_uri);

        // asset type
        if (asset_type == TEXTURE_TYPE)
        {
            data_ret =
              _gsk_asset_load_texture(p_cache, p_state->asset_handle, str_uri);
        } else if (asset_type == MATERIAL_TYPE)
        {
            data_ret =
              _gsk_asset_load_material(p_cache, p_state->asset_handle, str_uri);
        }

        return data_ret;

        // if we are hot-loaded, load it from the address on disk
    }

    return array_list_get_at_index(
      &(p_cache->asset_lists[asset_type].list_data), asset_index - 1);

    // check if the asset has been loaded
    // -- if not, load it

    // return the asset
}

void *
gsk_asset_get_str(gsk_AssetCache *p_cache, const char *str_uri)
{
    return _gsk_asset_get(p_cache, str_uri);
}

void
gsk_asset_get_handle(gsk_AssetCache *p_cache, u64 handle)
{
    return NULL;
}