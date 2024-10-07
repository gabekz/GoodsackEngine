/*
 * Copyright (c) 2024, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#include "asset.h"

#include <string.h>

#include "util/array_list.h"
#include "util/filesystem.h"
#include "util/hash_table.h"
#include "util/logger.h"
#include "util/sysdefs.h"

#include "core/graphics/material/material.h"
#include "core/graphics/mesh/model.h"
#include "core/graphics/shader/shader.h"
#include "core/graphics/texture/texture.h"

#include "asset/asset_cache.h"
#include "asset/asset_gcfg.h"
#include "asset/import/loader_gcfg.h"

static void *
_asset_load_generic(gsk_AssetCache *p_cache,
                    u64 asset_handle,
                    const char *str_uri,
                    LoadAssetFunc create_asset_func,
                    u32 expected_type)
{
    u32 asset_list  = GSK_ASSET_HANDLE_LIST_NUM(asset_handle);
    u32 asset_index = GSK_ASSET_HANDLE_INDEX_NUM(asset_handle);

    if (asset_list != expected_type)
    {
        LOG_CRITICAL("Asset handle is for incorrect asset type.");
    }

    // Get the pre-allocated memory location
    void *p_data = array_list_get_at_index(
      &(p_cache->asset_lists[asset_list].list_data), asset_index - 1);

    // Call the create_asset_func to initialize data at p_data
    create_asset_func(p_cache, str_uri, p_data);

    gsk_AssetCacheState *p_state = gsk_asset_cache_get(p_cache, str_uri);
    p_state->is_loaded           = TRUE;
    return p_data;
}

static void
__create_gcfg(gsk_AssetCache *p_cache, const char *str_uri, void *p_dest)
{
    gsk_GCFG gcfg         = gsk_load_gcfg(GSK_PATH(str_uri));
    *((gsk_GCFG *)p_dest) = gcfg;
}

static void
__create_texture(gsk_AssetCache *p_cache, const char *str_uri, void *p_dest)
{
    TextureOptions ops       = (TextureOptions) {8, GL_SRGB_ALPHA, TRUE, TRUE};
    gsk_Texture tex          = texture_create_2(GSK_PATH(str_uri), NULL, ops);
    *((gsk_Texture *)p_dest) = tex;
}

static void
__create_shader(gsk_AssetCache *p_cache, const char *str_uri, void *p_dest)
{
    gsk_ShaderProgram shader = gsk_shader_program_create(GSK_PATH(str_uri));
    *((gsk_ShaderProgram *)p_dest) = shader;
}

static void
__create_material(gsk_AssetCache *p_cache, const char *str_uri, void *p_dest)
{
    // TODO: Material should not use malloc

    gsk_GCFG gcfg            = gsk_load_gcfg(GSK_PATH(str_uri));
    gsk_Material *p_material = gsk_material_create_from_gcfg(p_cache, &gcfg);

    ((gsk_Material *)p_dest)->shaderProgram = p_material->shaderProgram;
    ((gsk_Material *)p_dest)->textures      = p_material->textures;
    ((gsk_Material *)p_dest)->texturesCount = p_material->texturesCount;
}

static void
__create_model(gsk_AssetCache *p_cache, const char *str_uri, void *p_dest)
{
    // TODO: Material should not use malloc

    gsk_Model *p_model =
      gsk_model_load_from_file(GSK_PATH(str_uri), 1.0f, FALSE);

    ((gsk_Model *)p_dest)->meshes      = p_model->meshes;
    ((gsk_Model *)p_dest)->meshesCount = p_model->meshesCount;
    ((gsk_Model *)p_dest)->fileType    = p_model->fileType;
}

void *
gsk_asset_get(gsk_AssetCache *p_cache, const char *str_uri)
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

        LoadAssetFunc p_create_func = NULL;
        switch (asset_type)
        {
        case GSK_ASSET_CACHE_GCFG: p_create_func = __create_gcfg; break;
        case GSK_ASSET_CACHE_TEXTURE: p_create_func = __create_texture; break;
        case GSK_ASSET_CACHE_MATERIAL: p_create_func = __create_material; break;
        case GSK_ASSET_CACHE_SHADER: p_create_func = __create_shader; break;
        case GSK_ASSET_CACHE_MODEL: p_create_func = __create_model; break;
        default: p_create_func = NULL; break;
        }

        // None
        if (p_create_func == NULL)
        {
            LOG_CRITICAL("INVALID asset type %d", asset_type);
        }

        // get the data
        data_ret = (gsk_GCFG *)_asset_load_generic(
          p_cache, p_state->asset_handle, str_uri, p_create_func, asset_type);

        if (p_state->is_loaded == FALSE)
        {
            LOG_ERROR("Probably failed to load asset. This may result in a "
                      "memory leak. (%s)",
                      str_uri);
            return NULL;
        }

        return data_ret;
    }

    LOG_DEBUG("asset already loaded. referencing (%s)", str_uri);

    return array_list_get_at_index(
      &(p_cache->asset_lists[asset_type].list_data), asset_index - 1);
}