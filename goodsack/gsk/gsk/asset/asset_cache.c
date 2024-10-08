/*
 * Copyright (c) 2024, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#include "asset_cache.h"

#include "string.h"

#include "asset/import/loader_gcfg.h"

#include "util/array_list.h"
#include "util/filesystem.h"
#include "util/hash_table.h"
#include "util/logger.h"
#include "util/sysdefs.h"

#include "core/graphics/material/material.h"
#include "core/graphics/mesh/model.h"
#include "core/graphics/shader/shader.h"
#include "core/graphics/texture/texture.h"

/*--------------------------------------------------------------------*/
gsk_AssetCache
gsk_asset_cache_init()
{
    gsk_AssetCache ret;

    u32 type_sizes[ASSETTYPE_LAST + 1];
    type_sizes[GSK_ASSET_CACHE_GCFG]     = sizeof(gsk_GCFG);
    type_sizes[GSK_ASSET_CACHE_TEXTURE]  = sizeof(gsk_Texture);
    type_sizes[GSK_ASSET_CACHE_MATERIAL] = sizeof(gsk_Material);
    type_sizes[GSK_ASSET_CACHE_SHADER]   = sizeof(gsk_ShaderProgram);
    type_sizes[GSK_ASSET_CACHE_MODEL]    = sizeof(gsk_Model);

    // setup hash table
    // TODO: needs to scale
    ret.asset_table = hash_table_init(GSK_ASSET_CACHE_TABLE_MAX);

    for (int i = 0; i < ASSETTYPE_LAST + 1; i++)
    {
        ret.asset_lists[i].list_state = array_list_init(
          sizeof(gsk_AssetCacheState), GSK_ASSET_CACHE_INCREMENT);
        ret.asset_lists[i].list_data =
          array_list_init(type_sizes[i], GSK_ASSET_CACHE_INCREMENT);
        // TODO: Change
        ret.asset_lists[i].list_options =
          array_list_init(sizeof(TextureOptions), GSK_ASSET_CACHE_INCREMENT);
    }

    // asset uri array
    ret.asset_uri_list = array_list_init(GSK_FS_MAX_PATH, 100);

    return ret;
}
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
void
gsk_asset_cache_add(gsk_AssetCache *p_cache,
                    u32 asset_type,
                    const char *str_uri)
{
    if (asset_type > ASSETTYPE_LAST)
    {
        LOG_CRITICAL("Attempt to access invalid asset type (%u is not valid)",
                     asset_type);
    }

    // ensure asset name is not already cached
    if (hash_table_has(&(p_cache->asset_table), str_uri))
    {
        LOG_CRITICAL(
          "Attempt to add asset to cache when it already exists (%s)", str_uri);
    }

    /*==== Generate file handle ======================================*/

    // TODO: hashmap increment needs to be removed.. should truncate from 0.
    u32 next_index = p_cache->asset_lists[asset_type].list_state.list_next + 1;

    // generate new handle, filled with asset type
    u64 new_handle = 0xFFFFFFFFFFFFFFFF;
    new_handle = (new_handle & 0x00FFFFFFFFFFFFFF) | ((u64)asset_type << 56);
    new_handle = (new_handle & 0xFFFFFFFF00000000) | next_index;

    u32 list_type   = GSK_ASSET_HANDLE_LIST_NUM(new_handle);
    u32 asset_index = GSK_ASSET_HANDLE_INDEX_NUM(new_handle);

    /*==== Populate hashmap with handle ==============================*/

    // add the handle to the hash table (after it is incremented)
    hash_table_add((HashTable *)&(p_cache->asset_table), str_uri, new_handle);

    /*==== Create reference in uri-string array ======================*/
    array_list_push(&(p_cache->asset_uri_list), str_uri);

    /*==== Create asset cache state ==================================*/

#if 0
    char *str;
    str = (char *)array_list_get_at_index(
      &(p_cache->asset_uri_list), p_cache->asset_uri_list.list_next - 1);
#endif

    gsk_AssetCacheState item = {
      .asset_handle    = new_handle,
      .asset_uri_index = p_cache->asset_uri_list.list_next - 1,
      .is_mem_loaded   = FALSE,
      .is_gpu_loaded   = FALSE,
    };

    // add empty data
    array_list_push(&(p_cache->asset_lists[asset_type].list_state), &item);
}
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
void
gsk_asset_cache_add_by_ext(gsk_AssetCache *p_cache, const char *str_uri)
{
    char *ext = strrchr(str_uri, '.');
    if (!ext) { LOG_CRITICAL("Failed to find file extension for %s", str_uri); }

    u32 list_type = 0;

    // gcfg
    if (!strcmp(ext, ".gcfg"))
    {
        list_type = GSK_ASSET_CACHE_GCFG;
    }

    // texture
    else if (!strcmp(ext, ".png") || !strcmp(ext, ".jpg") ||
             !strcmp(ext, ".tga") || !strcmp(ext, ".hdr"))
    {
        list_type = GSK_ASSET_CACHE_TEXTURE;
    }
    // material
    else if (!strcmp(ext, ".material"))
    {
        list_type = GSK_ASSET_CACHE_MATERIAL;
    }
    // shader
    else if (!strcmp(ext, ".shader"))
    {
        list_type = GSK_ASSET_CACHE_SHADER;
    }
    // model
    else if (!strcmp(ext, ".obj") || !strcmp(ext, ".gltf") ||
             !strcmp(ext, ".glb"))
    {
        list_type = GSK_ASSET_CACHE_MODEL;
    }
    // None
    else
    {
        LOG_TRACE("asset format is not available. (%s)", str_uri);
        return;
    }

    gsk_asset_cache_add(p_cache, list_type, str_uri);

    LOG_DEBUG("adding asset by extension (pool: %d, ext: %s)(asset: %s)",
              list_type,
              ext,
              str_uri);
}
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
gsk_AssetCacheState *
gsk_asset_cache_get(gsk_AssetCache *p_cache, const char *str_uri)
{
    /* TODO: Change hashmap so we don't have to truncate from 1 (WTF)
    we have to currently grab (asset_index - 1)
    */

    u64 handle = hash_table_get(&(p_cache->asset_table), str_uri);

    // get references
    u32 asset_type  = GSK_ASSET_HANDLE_LIST_NUM(handle);
    u32 asset_index = GSK_ASSET_HANDLE_INDEX_NUM(handle);

    if (asset_type > ASSETTYPE_LAST)
    {
        LOG_CRITICAL("Attempt to access invalid asset type (%u is not valid)",
                     asset_type);
    }

    gsk_AssetCacheState *p_state; // fetched cache state

#if ASSET_CACHE_GET_AT
    p_state = gsk_asset_cache_get_at(p_cache, asset_type, handle);
#else
    p_state = (gsk_AssetCacheState *)array_list_get_at_index(
      &(p_cache->asset_lists[asset_type].list_state), asset_index - 1);
#endif
    return p_state;
}
/*--------------------------------------------------------------------*/