/*
 * Copyright (c) 2024, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#include "asset_cache.h"

#include <string.h>

#include "asset/assetdefs.h"
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

    u32 sizes_data[ASSETTYPE_LAST + 1];
    u32 sizes_ops[ASSETTYPE_LAST + 1];

    sizes_data[GSK_ASSET_CACHE_GCFG]     = sizeof(gsk_GCFG);
    sizes_data[GSK_ASSET_CACHE_TEXTURE]  = sizeof(gsk_Texture);
    sizes_data[GSK_ASSET_CACHE_MATERIAL] = sizeof(gsk_Material);
    sizes_data[GSK_ASSET_CACHE_SHADER]   = sizeof(gsk_ShaderProgram);
    sizes_data[GSK_ASSET_CACHE_MODEL]    = sizeof(gsk_Model);

    sizes_ops[GSK_ASSET_CACHE_GCFG]     = 1;
    sizes_ops[GSK_ASSET_CACHE_TEXTURE]  = sizeof(TextureOptions);
    sizes_ops[GSK_ASSET_CACHE_MATERIAL] = 1;
    sizes_ops[GSK_ASSET_CACHE_SHADER]   = 1;
    sizes_ops[GSK_ASSET_CACHE_MODEL]    = sizeof(gsk_AssetModelOptions);

    // setup hash table
    // TODO: needs to scale
    ret.asset_table = hash_table_init(GSK_ASSET_CACHE_TABLE_MAX);

    for (int i = 0; i < ASSETTYPE_LAST + 1; i++)
    {
        ret.asset_lists[i].list_state =
          array_list_init(sizeof(gsk_AssetRef), GSK_ASSET_CACHE_INCREMENT);
        ret.asset_lists[i].list_data =
          array_list_init(sizes_data[i], GSK_ASSET_CACHE_INCREMENT);
        ret.asset_lists[i].list_options =
          array_list_init(sizes_ops[i], GSK_ASSET_CACHE_INCREMENT);
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

    gsk_AssetRef item = {
      .asset_handle    = new_handle,
      .asset_uri_index = p_cache->asset_uri_list.list_next - 1,
      .is_imported     = FALSE,
      .is_utilized     = FALSE,
      .p_data_import   = NULL,
      .p_data_active   = NULL,
    };

    array_list_push(&(p_cache->asset_lists[asset_type].list_state), &item);

    // add empty data -- we might want to may array_list act as a regular
    // buffer, too.
    array_list_push(&(p_cache->asset_lists[asset_type].list_data), NULL);

    /*==== Create default asset options ==============================*/

    TextureOptions default_tex = {
      .af_range        = 1,
      .internal_format = GL_SRGB_ALPHA,
      .gen_mips        = FALSE,
      .flip_vertically = TRUE,
    };

    gsk_AssetModelOptions default_model = {
      .scale            = 1.0f,
      .import_materials = FALSE,
    };

    void *p_options = NULL;
    switch (list_type)
    {
    case GSK_ASSET_CACHE_MODEL: p_options = &default_model; break;
    case GSK_ASSET_CACHE_TEXTURE: p_options = &default_tex; break;
    default: break;
    }

    // TODO: Should not push anything at all if null. Check references.
    array_list_push(&(p_cache->asset_lists[asset_type].list_options),
                    p_options);
}
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
// a.k.a, gsk_asset_cache_add_hot(cache, uri)
// we also want gsk_asset_cache_add_explicit(cache, uri, handle) a.k.a. gpak

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

    LOG_DEBUG("adding asset by extension (pool: %d, ext: %s)(asset: %s)",
              list_type,
              ext,
              str_uri);

    gsk_asset_cache_add(p_cache, list_type, str_uri);
}
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
gsk_AssetRef *
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

    gsk_AssetRef *p_ref; // fetched cache state

#if ASSET_CACHE_GET_AT
    p_ref = gsk_asset_cache_get_at(p_cache, asset_type, handle);
#else
    p_ref = (gsk_AssetRef *)array_list_get_at_index(
      &(p_cache->asset_lists[asset_type].list_state), asset_index - 1);
#endif
    return p_ref;
}
/*--------------------------------------------------------------------*/