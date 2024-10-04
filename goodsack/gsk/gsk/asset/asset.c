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
#include "core/graphics/shader/shader.h"
#include "core/graphics/texture/texture.h"

#include "asset/asset_gcfg.h"
#include "asset/import/loader_gcfg.h"

#define TEXTURE_TYPE  0
#define MATERIAL_TYPE 2
#define SHADER_TYPE   3

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
    TextureOptions ops = (TextureOptions) {8, GL_SRGB_ALPHA, TRUE, TRUE};

    // TODO: texture_create without allocating!!
    gsk_Texture tex = texture_create_2(GSK_PATH(str_uri), NULL, ops);

    gsk_Texture *p_data = (gsk_Texture *)array_list_get_at_index(
      &(p_cache->asset_lists[TEXTURE_TYPE].list_data), asset_index - 1);

    *p_data = tex;

    gsk_AssetCacheState *p_state = gsk_asset_cache_get(p_cache, str_uri);
    p_state->is_loaded           = TRUE;
    return p_data;
}

static gsk_ShaderProgram *
_gsk_asset_load_shader(gsk_AssetCache *p_cache,
                       u64 asset_handle,
                       const char *str_uri)
{
    u32 asset_list  = GSK_ASSET_HANDLE_LIST_NUM(asset_handle);
    u32 asset_index = GSK_ASSET_HANDLE_INDEX_NUM(asset_handle);

    if (asset_list != SHADER_TYPE)
    {
        // TODO: better logging
        LOG_CRITICAL("Asset handle is for incorrect asset type.");
    }

    // TODO: need to stop allocating from here.
    gsk_ShaderProgram shader = gsk_shader_program_create(GSK_PATH(str_uri));

    gsk_ShaderProgram *p_data = (gsk_ShaderProgram *)array_list_get_at_index(
      &(p_cache->asset_lists[SHADER_TYPE].list_data), asset_index - 1);

    *p_data = shader;

    gsk_AssetCacheState *p_state = gsk_asset_cache_get(p_cache, str_uri);
    p_state->is_loaded           = TRUE;
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

    gsk_ShaderProgram *p_shader = NULL;

    // need to grab the shader
    for (int i = 0; i < gcfg.list_items.list_next; i++)
    {
        gsk_GCFGItem *item = array_list_get_at_index(&gcfg.list_items, i);
        if (!strcmp(item->key, "shader"))
        {
            if (hash_table_has(&p_cache->asset_table, item->value) == FALSE)
            {
                LOG_ERROR("Shader asset is not cached! (%s)", item->value);
            }

            p_shader = (gsk_ShaderProgram *)gsk_asset_get(p_cache, item->value);
        }
    }
    if (p_shader == NULL)
    {
        LOG_CRITICAL("Material does not have a shader reference. (%s)",
                     str_uri);
    }

    gsk_Material *p_material = gsk_material_create(p_shader, NULL, 0);

    // attach textures
    for (int i = 0; i < gcfg.list_items.list_next; i++)
    {
        gsk_GCFGItem *item = array_list_get_at_index(&gcfg.list_items, i);
        if (!strcmp(item->key, "texture"))
        {
            if (hash_table_has(&p_cache->asset_table, item->value) == FALSE)
            {
                LOG_ERROR("Texture asset is not cached! (%s)", item->value);
            }

            // load and add texture

            gsk_Texture *p_tex =
              (gsk_Texture *)gsk_asset_get(p_cache, item->value);

            gsk_material_add_texture(p_material, p_tex);
        }
    }

    gsk_Material *p_data = (gsk_Material *)array_list_get_at_index(
      &(p_cache->asset_lists[MATERIAL_TYPE].list_data), asset_index - 1);

    p_data->shaderProgram = p_material->shaderProgram;
    p_data->textures      = p_material->textures;
    p_data->texturesCount = p_material->texturesCount;
    // p_data->vulkan.pipelineLayout = p_data->vulkan.pipelineLayout;

    gsk_AssetCacheState *p_state = gsk_asset_cache_get(p_cache, str_uri);
    p_state->is_loaded           = TRUE;

    return p_data;
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

        // asset type
        if (asset_type == TEXTURE_TYPE)
        {
            data_ret =
              _gsk_asset_load_texture(p_cache, p_state->asset_handle, str_uri);
        } else if (asset_type == MATERIAL_TYPE)
        {
            data_ret =
              _gsk_asset_load_material(p_cache, p_state->asset_handle, str_uri);
        } else if (asset_type == SHADER_TYPE)
        {
            data_ret =
              _gsk_asset_load_shader(p_cache, p_state->asset_handle, str_uri);
        } else
        {
            LOG_CRITICAL("INVALID asset type %d", asset_type);
        }

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

void
gsk_asset_get_handle(gsk_AssetCache *p_cache, u64 handle)
{
    return NULL;
}

void *
gsk_asset_alloc(gsk_AssetCache *p_cache, u64 handle)
{
}