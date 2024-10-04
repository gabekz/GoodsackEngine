/*
 * Copyright (c) 2024, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#include "asset_cache.h"

#include "string.h"

#include "util/array_list.h"
#include "util/filesystem.h"
#include "util/hash_table.h"
#include "util/logger.h"
#include "util/sysdefs.h"

#include "core/graphics/material/material.h"
#include "core/graphics/shader/shader.h"
#include "core/graphics/texture/texture.h"

/*--------------------------------------------------------------------*/
gsk_AssetCache
gsk_asset_cache_init()
{
    gsk_AssetCache ret;

    // setup hash table
    ret.asset_table = hash_table_init(GSK_ASSET_CACHE_TABLE_MAX);

    // setup  asset lists
    ret.asset_lists[0].list_state =
      array_list_init(sizeof(gsk_AssetCacheState), GSK_ASSET_CACHE_INCREMENT);
    ret.asset_lists[0].list_data =
      array_list_init(sizeof(gsk_Texture), GSK_ASSET_CACHE_INCREMENT);
    ret.asset_lists[0].list_options =
      array_list_init(sizeof(TextureOptions), GSK_ASSET_CACHE_INCREMENT);

    // setup  asset lists
    ret.asset_lists[2].list_state =
      array_list_init(sizeof(gsk_AssetCacheState), GSK_ASSET_CACHE_INCREMENT);
    ret.asset_lists[2].list_data =
      array_list_init(sizeof(gsk_Material), GSK_ASSET_CACHE_INCREMENT);
    ret.asset_lists[2].list_options =
      array_list_init(sizeof(TextureOptions), GSK_ASSET_CACHE_INCREMENT);

    // setup  asset lists
    ret.asset_lists[3].list_state =
      array_list_init(sizeof(gsk_AssetCacheState), GSK_ASSET_CACHE_INCREMENT);
    ret.asset_lists[3].list_data =
      array_list_init(sizeof(gsk_ShaderProgram), GSK_ASSET_CACHE_INCREMENT);
    ret.asset_lists[3].list_options =
      array_list_init(sizeof(TextureOptions), GSK_ASSET_CACHE_INCREMENT);
    // TODO: Other types
    // TODO: Handle specific file types elsewhere.

    return ret;
}
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
void
gsk_asset_cache_add(gsk_AssetCache *p_cache,
                    u32 asset_type,
                    const char *str_uri)
{
    if (asset_type > GSK_TOTAL_ASSET_TYPES)
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

    /*==== Create asset cache state ==================================*/

    gsk_AssetCacheState item = {.is_loaded = FALSE, .asset_handle = new_handle};

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

    // texture
    if (!strcmp(ext, ".png") || !strcmp(ext, ".jpg"))
    {
        list_type = 0;
    }
    // model
    else if (!strcmp(ext, ".obj") || !strcmp(ext, ".gltf"))
    {
        list_type = 1;
    }
    // material
    else if (!strcmp(ext, ".material"))
    {
        list_type = 2;
    }
    // shader
    else if (!strcmp(ext, ".shader"))
    {
        list_type = 3;
    }

    gsk_asset_cache_add(p_cache, list_type, str_uri);

    LOG_DEBUG("adding asset by extension (pool: %d, ext: %s)(asset: %s)",
              list_type,
              ext,
              str_uri);
}
/*--------------------------------------------------------------------*/

#if ASSET_CACHE_GET_AT
/*--------------------------------------------------------------------*/
gsk_AssetCacheState *
gsk_asset_cache_get_at(gsk_AssetCache *p_cache, u32 asset_type, u32 asset_index)
{
    if (asset_type > GSK_TOTAL_ASSET_TYPES)
    {
        LOG_CRITICAL("Attempt to access invalid asset type (%u is not valid)",
                     asset_type);
    }

    gsk_AssetCacheState *p_state =
      (gsk_AssetCacheState *)array_list_get_at_index(
        &(p_cache->asset_lists[asset_type].list_state), asset_index - 1);

    // LOG_INFO("%d", p_state->is_loaded);
    return p_state;
}
/*--------------------------------------------------------------------*/
#endif

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

    if (asset_type > GSK_TOTAL_ASSET_TYPES)
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

/*--------------------------------------------------------------------*/
void
gsk_asset_cache_fill_by_directory(gsk_AssetCache *p_cache, const char *root_uri)
{
}
/*--------------------------------------------------------------------*/