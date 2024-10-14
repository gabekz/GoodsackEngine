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
#include "asset/assetdefs.h"
#include "asset/import/loader_gcfg.h"
#include "io/parse_image.h"

#if 0
static gsk_AssetRef
_gsk_asset_import(gsk_AssetCache *p_cache, const char *str_uri)
{
    gsk_AssetRef ret = {0};

    gsk_AssetRef *p_ref = gsk_asset_cache_get(p_cache, str_uri);
    if (p_ref == NULL) { LOG_CRITICAL("Failed to get asset (%s)", str_uri); }

    if (p_ref->is_imported == TRUE)
    {
        LOG_CRITICAL("Probably don't want to do this!");
    }

    u32 asset_type  = GSK_ASSET_HANDLE_LIST_NUM(p_ref->asset_handle);
    u32 asset_index = GSK_ASSET_HANDLE_INDEX_NUM(p_ref->asset_handle);

    // TODO: handle path for importing hot
    if (asset_type == GSK_ASSET_CACHE_TEXTURE)
    {
        ret = parse_image(GSK_PATH(str_uri));
    }

    p_ref->is_imported = TRUE;
    return ret;

    // TODO: handle path for importing from .gpak
}
#endif

static void
__create_gcfg(const char *str_uri, void *p_options, void *p_dest)
{
    // gsk_IO_AssetGCFG asset = gsk_io_import_gcfg(GSK_PATH(str_uri));

    gsk_GCFG gcfg = gsk_load_gcfg(GSK_PATH(str_uri));
    gsk_asset_gcfg_set_config(&gcfg);

    *((gsk_GCFG *)p_dest) = gcfg;
}

static void
__create_texture(const char *str_uri, void *p_options, void *p_dest)
{
    gsk_AssetBlob asset_source = parse_image(GSK_PATH(str_uri));

    // TextureOptions ops = (TextureOptions) {8, GL_SRGB_ALPHA, TRUE, TRUE};
    gsk_Texture tex =
      texture_create_2(&asset_source, NULL, *(TextureOptions *)p_options);
    *((gsk_Texture *)p_dest) = tex;

    free(asset_source.p_buffer); // TODO: change
}

static void
__create_shader(const char *str_uri, void *p_options, void *p_dest)
{
    gsk_ShaderProgram shader = gsk_shader_program_create(GSK_PATH(str_uri));
    *((gsk_ShaderProgram *)p_dest) = shader;
}

static void
__create_material(const char *str_uri, void *p_options, void *p_dest)
{
    // TODO: Material should not use malloc

    gsk_GCFG gcfg            = gsk_load_gcfg(GSK_PATH(str_uri));
    gsk_Material *p_material = gsk_material_create_from_gcfg(&gcfg);

    ((gsk_Material *)p_dest)->shaderProgram = p_material->shaderProgram;
    ((gsk_Material *)p_dest)->textures      = p_material->textures;
    ((gsk_Material *)p_dest)->texturesCount = p_material->texturesCount;
}

static void
__create_model(const char *str_uri, void *p_options, void *p_dest)
{
    gsk_Model *p_model =
      gsk_model_load_from_file(GSK_PATH(str_uri), 1.0f, FALSE);

    ((gsk_Model *)p_dest)->meshes      = p_model->meshes;
    ((gsk_Model *)p_dest)->meshesCount = p_model->meshesCount;
    ((gsk_Model *)p_dest)->fileType    = p_model->fileType;
}

static void *
_asset_load_generic(gsk_AssetCache *p_cache,
                    gsk_AssetRef *p_ref,
                    const char *str_uri,
                    LoadAssetFunc create_asset_func,
                    u32 expected_type)
{

    if (p_ref->is_imported == FALSE)
    {
        LOG_CRITICAL("attempting to load asset that is not imported!");
        return NULL;
    }

    if (p_ref->is_utilized == TRUE)
    {
        LOG_CRITICAL("attemping to load an already active asset!");
        return NULL;
    }

    u32 asset_list  = GSK_ASSET_HANDLE_LIST_NUM(p_ref->asset_handle);
    u32 asset_index = GSK_ASSET_HANDLE_INDEX_NUM(p_ref->asset_handle);

    if (asset_list != expected_type)
    {
        LOG_CRITICAL("asset handle is for incorrect asset type.");
    }

    // Get the pre-allocated memory location from cache
    void *p_data = array_list_get_at_index(
      &(p_cache->asset_lists[asset_list].list_data), asset_index - 1);

    void *p_options = array_list_get_at_index(
      &(p_cache->asset_lists[asset_list].list_options), asset_index - 1);

    create_asset_func(str_uri, p_options, p_data);

    p_ref->is_utilized = TRUE;
    return p_data;
}

gsk_AssetRef *
_gsk_asset_get_internal(gsk_AssetCache *p_cache, const char *str_uri)
{
    // check if the asset has been added already
    // -- if not, exit

    gsk_AssetRef *p_ref = gsk_asset_cache_get(p_cache, str_uri);

    if (p_ref == NULL) { LOG_CRITICAL("Failed to get asset (%s)", str_uri); }

    u32 asset_type  = GSK_ASSET_HANDLE_LIST_NUM(p_ref->asset_handle);
    u32 asset_index = GSK_ASSET_HANDLE_INDEX_NUM(p_ref->asset_handle);

    if (p_ref->is_utilized == TRUE) { return p_ref; }

    LOG_DEBUG("loading asset (%s)", str_uri);

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

    // TODO: Import asset here
    // also clear..
    p_ref->is_imported = TRUE; // should be replaced with import function

    // get the data
    p_ref->p_data_active = (void *)_asset_load_generic(
      p_cache, p_ref, str_uri, p_create_func, asset_type);

    if (p_ref->is_imported == FALSE)
    {
        LOG_ERROR("Failed to import asset data for (%s).", str_uri);
    }

    if (p_ref->is_utilized == FALSE)
    {
        LOG_ERROR("Probably failed to load asset. This may result in a "
                  "memory leak. (%s)",
                  str_uri);
        return NULL;
    }

    return p_ref;

    // return array_list_get_at_index(
    //  &(p_cache->asset_lists[asset_type].list_data), asset_index - 1);
}