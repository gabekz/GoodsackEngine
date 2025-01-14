/*
 * Copyright (c) 2024-present, Gabriel Kutuzov
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
#include "asset/gpak/gpak.h"
#include "asset/import/loader_gcfg.h"
#include "io/parse_image.h"
#include "io/serialize_model.h"

// TODO: We don't want to depend on the runtime
#include "runtime/gsk_runtime_wrapper.h"

static u8
__asset_import(gsk_AssetCache *p_cache, const char *str_uri)
{
    gsk_AssetRef *p_ref = gsk_asset_cache_get(p_cache, str_uri);
    if (p_ref == NULL) { LOG_CRITICAL("Failed to get asset (%s)", str_uri); }

    if (p_ref->is_imported == TRUE)
    {
        LOG_CRITICAL("Probably don't want to do this!");
    }

    u32 asset_type  = GSK_ASSET_HANDLE_LIST_NUM(p_ref->asset_handle);
    u32 asset_index = GSK_ASSET_HANDLE_INDEX_NUM(p_ref->asset_handle);

    // pre-allocated import blob data
    gsk_AssetBlob *p_blob = array_list_get_at_index(
      &(p_cache->asset_lists[asset_type].list_data_import), asset_index - 1);

    p_blob->asset_type = (GskAssetType)asset_type;

    // pre-allocated options data
    void *p_options = array_list_get_at_index(
      &(p_cache->asset_lists[asset_type].list_options), asset_index - 1);

    // import from gpak
    if (p_ref->is_baked == TRUE)
    {
        if (asset_type == GskAssetType_Texture)
        {
            *p_blob               = gsk_gpak_reader_import_blob(str_uri);
            p_blob->is_serialized = TRUE;
            if (p_blob == NULL) { return 0; }
        }
    }

    // import from disk
    else
    {
        // Texture import
        if (asset_type == GskAssetType_Texture)
        {
            *p_blob               = parse_image(GSK_PATH(str_uri));
            p_blob->is_serialized = FALSE;
        }

        // Model import
        else if (asset_type == GskAssetType_Model)
        {
            gsk_AssetModelOptions *p_ops = NULL;
            p_ops                        = (gsk_AssetModelOptions *)p_options;

            gsk_Model *p_model = gsk_model_load_from_file(
              GSK_PATH(str_uri), p_ops->scale, p_ops->import_materials);

            p_blob->p_buffer      = p_model;
            p_blob->is_serialized = FALSE;
        }

        if (p_blob == NULL || p_blob->p_buffer == NULL) { return 0; }

        // TODO: Check if we want to serialize HERE
    }

    p_ref->p_data_import = p_blob;
    p_ref->is_imported   = TRUE;
    return 1;

    // TODO: handle path for importing from .gpak
}

static void
__create_gcfg(const char *str_uri, void *p_options, void *p_dest)
{
    // gsk_IO_AssetGCFG asset = gsk_io_import_gcfg(GSK_PATH(str_uri));

    gsk_GCFG gcfg = gsk_load_gcfg(GSK_PATH(str_uri));
    gsk_asset_gcfg_set_config(&gcfg);

    *((gsk_GCFG *)p_dest) = gcfg;
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
    gsk_GCFG gcfg            = gsk_load_gcfg(GSK_PATH(str_uri));
    gsk_Material *p_material = gsk_material_create_from_gcfg(&gcfg);

    ((gsk_Material *)p_dest)->shaderProgram = p_material->shaderProgram;
    ((gsk_Material *)p_dest)->textures      = p_material->textures;
    ((gsk_Material *)p_dest)->texturesCount = p_material->texturesCount;
}

static u8
__load_texture(gsk_AssetRef *p_ref, void *p_options, void *p_dest)
{
    gsk_AssetBlob *p_blob = (gsk_AssetBlob *)p_ref->p_data_import;

    gsk_Texture tex =
      _gsk_texture_create_internal(p_blob, NULL, NULL, p_options);

    if (tex.id == 0) { return 0; }

    *((gsk_Texture *)p_dest) = tex;

    free(p_blob->p_buffer);

    return 1;
}

static u8
__load_model(gsk_AssetRef *p_ref, void *p_options, void *p_dest)
{
    gsk_AssetBlob *p_blob = (gsk_AssetBlob *)p_ref->p_data_import;

    if (p_blob->is_serialized == TRUE)
    {
        LOG_INFO("SERIAL");
        // extract
        // assemble
    }
    // assemble without extraction
    else if (p_blob->is_serialized == FALSE)
    {
        gsk_Model *p_model = (gsk_Model *)p_blob->p_buffer;

        // upload each mesh to the GPU
        for (int i = 0; i < p_model->meshesCount; i++)
        {
            u8 status = gsk_mesh_assemble(p_model->meshes[i]);
            if (status == 0 || p_model->meshes[i]->is_gpu_loaded != TRUE)
            {
                LOG_CRITICAL("Failed to upload");
                return 0;
            }
        }

        ((gsk_Model *)p_dest)->meshes      = p_model->meshes;
        ((gsk_Model *)p_dest)->meshesCount = p_model->meshesCount;
        ((gsk_Model *)p_dest)->fileType    = p_model->fileType;
    }
    return 1;

    free(p_blob->p_buffer);
}

static void *
_asset_load_generic(gsk_AssetCache *p_cache,
                    gsk_AssetRef *p_ref,
                    const char *str_uri,
                    gsk_CreateAssetFptr create_asset_func,
                    gsk_LoadAssetFptr load_asset_func,
                    u32 expected_type)
{

    if ((create_asset_func && load_asset_func) ||
        (create_asset_func == NULL && load_asset_func == NULL))
    {
        LOG_CRITICAL("Failed to get create/load function for asset %s",
                     str_uri);
    }

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
      &(p_cache->asset_lists[asset_list].list_data_active), asset_index - 1);

    void *p_options = array_list_get_at_index(
      &(p_cache->asset_lists[asset_list].list_options), asset_index - 1);

    if (create_asset_func)
    {
        create_asset_func(str_uri, p_options, p_data);
    }

    else if (load_asset_func)
    {
        const u8 load_err = 0;
        u8 load_code      = load_asset_func(p_ref, p_options, p_data);

        if (load_code == load_err)
        {
            p_ref->is_utilized = FALSE;
            return NULL;
        }
    }

    p_ref->is_utilized = TRUE;
    return p_data;
}

gsk_AssetRef *
_gsk_asset_get_internal(gsk_AssetCache *p_cache,
                        const char *str_uri,
                        u8 fetch_mode)
{
    // check if the asset has been added already
    // -- if not, exit

    gsk_AssetRef *p_ref = gsk_asset_cache_get(p_cache, str_uri);
    u8 is_fallback      = FALSE;

    if (p_ref == NULL)
    {
        LOG_ERROR("Failed to get asset (%s)", str_uri);

        gsk_asset_cache_add_by_ext(p_cache, str_uri);
        p_ref = gsk_asset_cache_get(p_cache, str_uri);

        if (p_ref == NULL)
        {
            LOG_CRITICAL("Failed to create intermediate asset");
        }

        u32 type          = GSK_ASSET_HANDLE_LIST_NUM(p_ref->asset_handle);
        p_ref->p_fallback = gsk_runtime_get_fallback_asset(type);

        if (p_ref->p_fallback == NULL)
        {
            LOG_CRITICAL("Failed to retrieve fallback asset for type: %d",
                         type);
        }
    }

    // swap ref with fallback
    if (p_ref->p_fallback)
    {
        p_ref       = (gsk_AssetRef *)p_ref->p_fallback;
        is_fallback = TRUE;
    }

    u32 asset_type  = GSK_ASSET_HANDLE_LIST_NUM(p_ref->asset_handle);
    u32 asset_index = GSK_ASSET_HANDLE_INDEX_NUM(p_ref->asset_handle);

    if (p_ref->is_utilized == TRUE) { return p_ref; }

    LOG_DEBUG("loading asset (%s)", str_uri);

    gsk_CreateAssetFptr p_create_func = NULL;
    gsk_LoadAssetFptr p_load_func     = NULL;

    switch (asset_type)
    {
    // create-functions
    case GskAssetType_GCFG: p_create_func = __create_gcfg; break;
    case GskAssetType_Material: p_create_func = __create_material; break;
    case GskAssetType_Shader: p_create_func = __create_shader; break;
    // load-functions
    case GskAssetType_Texture: p_load_func = __load_texture; break;
    case GskAssetType_Model: p_load_func = __load_model; break;
    // failed
    default:
        p_create_func = NULL;
        p_load_func   = NULL;
        break;
    }

    // None
    if (p_create_func == NULL && p_load_func == NULL)
    {
        LOG_CRITICAL("INVALID asset type %d. Asset handle (%d) is corrupt",
                     asset_type,
                     p_ref->asset_handle);
    }

    if (fetch_mode == GSK_ASSET_FETCH_VALIDATE)
    {
        LOG_DEBUG("Asset handle (%d) validated successfully.",
                  p_ref->asset_handle);
        return p_ref;
    }

    u8 import_code = 1;
    if (p_ref->is_imported == FALSE)
    {
        import_code = __asset_import(p_cache, str_uri);
    }

    if (import_code == 0 || p_ref->is_imported == FALSE)
    {
        // Abort if we can't even import the fallback asset
        if (is_fallback == TRUE)
        {
            LOG_CRITICAL("Failed to import FALLBACK asset. asset_type: (%d).",
                         asset_type);
        }

        LOG_ERROR("Failed to import asset data for (%s).", str_uri);

        // Return fallback
        if (p_ref->p_fallback == NULL)
        {
            p_ref->p_fallback = gsk_runtime_get_fallback_asset(asset_type);
        }
        return p_ref->p_fallback;
    }

    if (fetch_mode == GSK_ASSET_FETCH_IMPORT)
    {
        LOG_DEBUG("Asset (%s) fetched import.", str_uri);
        return p_ref;
    }

    // Utilize/Create data

    p_ref->p_data_active = (void *)_asset_load_generic(
      p_cache, p_ref, str_uri, p_create_func, p_load_func, asset_type);

    if (p_ref->p_data_active == NULL || p_ref->is_utilized == FALSE)
    {
        // Abort if we can't even LOAD the fallback asset
        if (is_fallback == TRUE)
        {
            LOG_CRITICAL("Failed to load FALLBACK asset. asset_type: (%d).",
                         asset_type);
        }

        LOG_ERROR("Probably failed to load asset. This may result in a "
                  "memory leak. (%s)",
                  str_uri);

        // Return fallback
        if (p_ref->p_fallback == NULL)
        {
            p_ref->p_fallback = gsk_runtime_get_fallback_asset(asset_type);
        }
        return p_ref->p_fallback;
    }

    return p_ref;
}