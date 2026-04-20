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

#include "core/audio/audio_clip.h"
#include "core/device/device.h"
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

#include "asset/gpak/gpak_archive.h"

// TODO: We don't want to depend on the runtime
#include "runtime/gsk_runtime_wrapper.h"

#define _DISABLE_AUDIO_ARCHIVE FALSE
#define _DISABLE_MODEL_ARCHIVE FALSE

static inline const char *
_fetch_mode_str(u8 fetch_mode)
{
    switch (fetch_mode)
    {
    case GSK_ASSET_FETCH_ALL: return "FETCH_ALL";
    case GSK_ASSET_FETCH_IMPORT: return "FETCH_IMPORT";
    case GSK_ASSET_FETCH_VALIDATE: return "FETCH_VALIDATE";
    default: return "";
    }
}

static inline const char *
_asset_type_str(GskAssetType asset_type)
{
    switch (asset_type)
    {
    case GskAssetType_GCFG: return "GCFG";
    case GskAssetType_Texture: return "Texture";
    case GskAssetType_Material: return "Material";
    case GskAssetType_Shader: return "Shader";
    case GskAssetType_Audio: return "Audio";
    case GskAssetType_Model: return "Model";
    default: return "";
    }
}

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

    p_blob->asset_type    = (GskAssetType)asset_type;
    p_blob->is_serialized = FALSE;

    // pre-allocated options data
    void *p_options = array_list_get_at_index(
      &(p_cache->asset_lists[asset_type].list_options), asset_index - 1);

    // import from gpak
    if (p_ref->is_baked == TRUE)
    {
        gsk_AssetBlob blob_import = gsk_gpak_reader_import_blob(str_uri);
        *p_blob                   = blob_import;
        p_blob->is_serialized     = (p_blob->p_buffer == NULL) ? FALSE : TRUE;

        if (p_blob == NULL)
        {
            LOG_ERROR("null blob");
            return 0;
        }
        if (p_blob->p_buffer == NULL) { return 0; }
    }

    // import from disk
    else
    {
        // Texture import
        if (asset_type == GskAssetType_Texture)
        {
            *p_blob               = parse_image(GSK_PATH(str_uri));
            p_blob->is_serialized = TRUE;
        }
        // Audio import
        else if (asset_type == GskAssetType_Audio)
        {
            gsk_AudioClip *p_clip = malloc(sizeof(gsk_AudioClip));
            *p_clip               = gsk_audio_clip_import_from_file(str_uri);

#if _DISABLE_AUDIO_ARCHIVE

            p_blob->p_buffer      = p_clip;
            p_blob->buffer_len    = sizeof(gsk_AudioClip);
            p_blob->asset_type    = GskAssetType_Audio;
            p_blob->is_serialized = FALSE;
#else
            gsk_audio_clip_archive(GskArchiveMode_Write, p_clip, p_blob);
#endif // _DISABLE_MODEL_ARCHIVE
        }

        // Model import
        else if (asset_type == GskAssetType_Model)
        {
            gsk_AssetModelOptions *p_ops = NULL;
            p_ops                        = (gsk_AssetModelOptions *)p_options;

            gsk_Model *p_model = gsk_model_load_from_file(
              GSK_PATH(str_uri), p_ops->scale, p_ops->import_materials);

            if (p_model)
            {
#if _DISABLE_MODEL_ARCHIVE
                p_blob->p_buffer      = p_model;
                p_blob->is_serialized = FALSE;
#else
                gsk_model_archive(GskArchiveMode_Write, p_model, p_blob);
#endif // _DISABLE_MODEL_ARCHIVE
            }

        }

        // Shader import
        else if (asset_type == GskAssetType_GCFG)
        {
            gsk_GCFG *p_gcfg = malloc(sizeof(gsk_GCFG));
            *p_gcfg          = gsk_load_gcfg(GSK_PATH(str_uri));

#if 0
            p_blob->p_buffer      = p_gcfg;
            p_blob->buffer_len    = sizeof(gsk_GCFG);
            p_blob->asset_type    = GskAssetType_GCFG;
            p_blob->is_serialized = FALSE;
#else
            gsk_asset_gcfg_archive(GskArchiveMode_Write, p_gcfg, p_blob);
            p_blob->is_serialized = TRUE;
#endif
        }

        // Shader import
        else if (asset_type == GskAssetType_Shader)
        {
            gsk_ShaderProgram *p_shader = malloc(sizeof(gsk_ShaderProgram));
            *p_shader = gsk_shader_program_import_from_file(GSK_PATH(str_uri));

#if 0
            p_blob->p_buffer      = p_shader;
            p_blob->buffer_len    = sizeof(gsk_ShaderProgram);
            p_blob->asset_type    = GskAssetType_Shader;
            p_blob->is_serialized = FALSE;
#else
            gsk_shader_archive(GskArchiveMode_Write, p_shader, p_blob);
#endif
        }

        // Material import
        else if (asset_type == GskAssetType_Material)
        {
            // gsk_AssetMaterialOptions *p_ops = NULL;
            // p_ops = (gsk_AssetMaterialOptions *)p_options;

            gsk_GCFG *p_gcfg = malloc(sizeof(gsk_GCFG));
            *p_gcfg          = gsk_load_gcfg(GSK_PATH(str_uri));

            LOG_INFO("Material import");

#if 0
            if (p_gcfg->is_valid == TRUE)
            {
                p_blob->p_buffer      = p_gcfg;
                p_blob->buffer_len    = sizeof(gsk_GCFG);
                p_blob->asset_type    = GskAssetType_Material;
                p_blob->is_serialized = FALSE;
            }

#else

            gsk_asset_gcfg_archive(GskArchiveMode_Write, p_gcfg, p_blob);
            p_blob->is_serialized = TRUE;
#endif
        }

        if (p_blob == NULL) { return 0; }
        if (p_blob->p_buffer == NULL) { return 0; }

        // TODO: Check if we want to serialize HERE
    }

    p_ref->p_data_import = p_blob;
    p_ref->is_imported   = TRUE;
    return 1;

    // TODO: handle path for importing from .gpak
}

static u8
__load_gcfg(gsk_AssetRef *p_ref, void *p_options, void *p_dest)
{
    gsk_AssetBlob *p_blob = (gsk_AssetBlob *)p_ref->p_data_import;
    gsk_GCFG *p_gcfg      = NULL;

    if (p_blob->is_serialized == TRUE)
    {
        p_gcfg = (gsk_GCFG *)(p_dest);
        gsk_asset_gcfg_archive(GskArchiveMode_Read, p_gcfg, p_blob);
    } else
    {
        p_gcfg = (gsk_GCFG *)p_blob->p_buffer;
    }

    if (p_blob == NULL) { return 0; }
    if (p_blob->p_buffer == NULL) { return 0; }

    gsk_asset_gcfg_set_config(p_gcfg);
    *((gsk_GCFG *)p_dest) = *p_gcfg;

    return 1;
}

static u8
__load_shader(gsk_AssetRef *p_ref, void *p_options, void *p_dest)
{
    gsk_AssetBlob *p_blob       = (gsk_AssetBlob *)p_ref->p_data_import;
    gsk_ShaderProgram *p_shader = NULL;

    if (p_blob->is_serialized == TRUE)
    {
        p_shader = (gsk_ShaderProgram *)(p_dest);
        gsk_shader_archive(GskArchiveMode_Read, p_shader, p_blob);
    } else
    {
        p_shader = (gsk_ShaderProgram *)p_blob->p_buffer;
    }

    gsk_shader_program_load(p_shader);

    ((gsk_ShaderProgram *)p_dest)->id           = p_shader->id;
    ((gsk_ShaderProgram *)p_dest)->id_skinned   = p_shader->id_skinned;
    ((gsk_ShaderProgram *)p_dest)->shaderSource = p_shader->shaderSource;

    return 1;
}

static u8
__load_material(gsk_AssetRef *p_ref, void *p_options, void *p_dest)
{
    gsk_AssetBlob *p_blob = (gsk_AssetBlob *)p_ref->p_data_import;
    // gsk_Material *p_material = (gsk_Material *)p_dest;
    gsk_Material *p_material = NULL;

    if (p_blob->is_serialized == TRUE)
    {
        gsk_GCFG *p_gcfg = malloc(sizeof(gsk_GCFG));
        gsk_asset_gcfg_archive(GskArchiveMode_Read, p_gcfg, p_blob);
        p_material = gsk_material_create_from_gcfg(p_gcfg);
        free(p_gcfg);
    }

    else
    {
        p_material =
          gsk_material_create_from_gcfg((gsk_GCFG *)p_blob->p_buffer);
    }

    ((gsk_Material *)p_dest)->shaderProgram = p_material->shaderProgram;
    ((gsk_Material *)p_dest)->textures      = p_material->textures;
    ((gsk_Material *)p_dest)->texturesCount = p_material->texturesCount;

    return 1;
}

static u8
__load_texture(gsk_AssetRef *p_ref, void *p_options, void *p_dest)
{
    void *p_vk_device =
      (GSK_DEVICE_API_VULKAN) ? gsk_runtime_get_renderer()->vulkanDevice : NULL;

    gsk_AssetBlob *p_blob = (gsk_AssetBlob *)p_ref->p_data_import;

    gsk_Texture tex =
      _gsk_texture_create_internal(p_blob, NULL, p_vk_device, p_options);

    if (tex.is_valid != TRUE) { return 0; }

    *((gsk_Texture *)p_dest) = tex;

    free(p_blob->p_buffer);

    return 1;
}

static u8
__load_audio(gsk_AssetRef *p_ref, void *p_options, void *p_dest)
{
    gsk_AssetBlob *p_blob = (gsk_AssetBlob *)p_ref->p_data_import;
    if (p_blob->p_buffer == NULL) { return 0; }

    gsk_AudioClip *p_clip = NULL;

    if (p_blob->is_serialized == TRUE)
    {
        p_clip = (gsk_AudioClip *)p_dest;
        gsk_audio_clip_archive(GskArchiveMode_Read, p_clip, p_blob);
    }

    else
    {
        p_clip = (gsk_AudioClip *)p_blob->p_buffer;
    }

    u8 load_status = gsk_audio_clip_load(p_clip);
    if (load_status != 1) { return 0; }

    *((gsk_AudioClip *)p_dest) = *(gsk_AudioClip *)p_clip;
    // free(p_blob->p_buffer);

    return 1;
}

static u8
__load_model(gsk_AssetRef *p_ref, void *p_options, void *p_dest)
{
    gsk_AssetBlob *p_blob = (gsk_AssetBlob *)p_ref->p_data_import;
    gsk_Model *p_model    = (gsk_Model *)p_dest;

    if (p_model == NULL) { LOG_CRITICAL("model asset destination corrupted"); }

    if (p_blob->is_serialized == TRUE)
    {
        gsk_model_archive(GskArchiveMode_Read, p_model, p_blob);
    }
    // assemble without extraction
    else if (p_blob->is_serialized == FALSE)
    {
        p_model = (gsk_Model *)p_blob->p_buffer;
    }

    // upload each mesh to the GPU
    for (int i = 0; i < p_model->meshesCount; i++)
    {
        u8 status = gsk_mesh_assemble(p_model->meshes[i]);
        if (status == 0 || p_model->meshes[i]->is_gpu_loaded != TRUE)
        {
            LOG_ERROR("Failed to upload mesh");
            return 0;
        }
    }

    ((gsk_Model *)p_dest)->meshes      = p_model->meshes;
    ((gsk_Model *)p_dest)->meshesCount = p_model->meshesCount;
    ((gsk_Model *)p_dest)->fileType    = p_model->fileType;

    return 1;

    // free(p_blob->p_buffer);
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

#if 0
static void
__set_asset_fallback(gsk_AssetRef *p_ref)
{
    if (p_ref == NULL) { LOG_CRITICAL("checking on null asset ref"); }
    p_ref->p_fallback = gsk_runtime_get_fallback_asset(
      GSK_ASSET_HANDLE_LIST_NUM(p_ref->asset_handle));
}

static u8
__check_fallback(const gsk_AssetRef *p_ref,
                 gsk_AssetRef **p_ref_out,
                 gsk_AssetCache **p_cache_out)
{
    u8 status = FALSE;

    if (p_ref == NULL) { LOG_CRITICAL("checking on null asset ref"); }

    if (p_ref->p_fallback == NULL) { return FALSE; }

    // set fallback
    *p_ref_out = (gsk_AssetRef *)p_ref->p_fallback;
    *p_cache_out =
      gsk_runtime_get_asset_cache_index(GSK_ASSET_FALLBACK_CACHE_INDEX);

    return TRUE;
}
#endif

gsk_AssetRef *
_gsk_asset_get_internal(const gsk_AssetCache *p_cache,
                        const char *str_uri,
                        u8 fetch_mode)
{
    gsk_AssetCache *p_cache_safe = p_cache;
    u8 is_fallback               = FALSE;

    gsk_AssetRef *p_ref = gsk_asset_cache_get(p_cache_safe, str_uri);

    if (p_ref == NULL)
    {
        LOG_ERROR("Failed to get asset (%s)", str_uri);

        gsk_asset_cache_add_by_ext(p_cache_safe, str_uri);
        p_ref = gsk_asset_cache_get(p_cache_safe, str_uri);
        if (p_ref == NULL)
        {
            LOG_CRITICAL("Failed to create intermediate asset");
        }

        // set fallback
        p_ref->p_fallback = gsk_runtime_get_fallback_asset(
          GSK_ASSET_HANDLE_LIST_NUM(p_ref->asset_handle));
    }

#if 1
    // swap ref with fallback
    if (p_ref->p_fallback)
    {
        is_fallback = TRUE;

        p_ref = (gsk_AssetRef *)p_ref->p_fallback;

        // NOTE: must swap the asset cache to the one which contains the default
        // fallback assets
        p_cache_safe =
          gsk_runtime_get_asset_cache_index(GSK_ASSET_FALLBACK_CACHE_INDEX);
    }
#endif

    GskAssetType asset_type = GSK_ASSET_HANDLE_LIST_NUM(p_ref->asset_handle);
    u32 asset_index         = GSK_ASSET_HANDLE_INDEX_NUM(p_ref->asset_handle);

    if (p_ref->is_utilized == TRUE) { return p_ref; }

    if (is_fallback == FALSE)
    {
        LOG_DEBUG(
          "getting asset (%s) - (%s)", str_uri, _fetch_mode_str(fetch_mode));
    } else
    {
        LOG_DEBUG("getting FALLBACK asset for type: %s (%p)",
                  _asset_type_str(asset_type),
                  p_ref);
    }

    gsk_CreateAssetFptr p_create_func = NULL;
    gsk_LoadAssetFptr p_load_func     = NULL;

    switch (asset_type)
    {
    // load-functions
    case GskAssetType_GCFG: p_load_func = __load_gcfg; break;
    case GskAssetType_Material: p_load_func = __load_material; break;
    case GskAssetType_Shader: p_load_func = __load_shader; break;
    case GskAssetType_Texture: p_load_func = __load_texture; break;
    case GskAssetType_Audio: p_load_func = __load_audio; break;
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
        import_code = __asset_import(p_cache_safe, str_uri);
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

        // TODO: check this
        p_ref->p_fallback = gsk_runtime_get_fallback_asset(asset_type);
        // swap ref with fallback
        if (p_ref->p_fallback)
        {
            is_fallback = TRUE;

            p_ref = (gsk_AssetRef *)p_ref->p_fallback;

            // NOTE: must swap the asset cache to the one which contains the
            // default fallback assets
            p_cache_safe =
              gsk_runtime_get_asset_cache_index(GSK_ASSET_FALLBACK_CACHE_INDEX);
        }

        if (p_ref == NULL) { LOG_CRITICAL("FUCK!!"); }
    }

    // stop if we are only importing or the asset is already utilized/loaded
    if (fetch_mode == GSK_ASSET_FETCH_IMPORT || p_ref->is_utilized == TRUE)
    {
        return p_ref;
    }

    // Utilize/Create data

    p_ref->p_data_active = (void *)_asset_load_generic(
      p_cache_safe, p_ref, str_uri, p_create_func, p_load_func, asset_type);

    if (p_ref->p_data_active == NULL || p_ref->is_utilized == FALSE)
    {
        // Abort if we can't even LOAD the fallback asset
        if (is_fallback == TRUE)
        {
            LOG_CRITICAL("Failed to load FALLBACK asset. asset_type: (%s).",
                         _asset_type_str(asset_type));
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