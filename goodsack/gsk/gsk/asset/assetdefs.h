/*
 * Copyright (c) 2024, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#ifndef __ASSETDEFS_H__
#define __ASSETDEFS_H__

#include "util/sysdefs.h"

#ifdef __cplusplus
extern "C" {
#endif //_cplusplus

typedef enum GskAssetType_ {
    GskAssetType_GCFG = 0,
    GskAssetType_Texture,
    GskAssetType_Material,
    GskAssetType_Shader,
    GskAssetType_Model,
} GskAssetType_;

#define ASSETTYPE_FIRST GskAssetType_GCFG
#define ASSETTYPE_LAST  GskAssetType_Model

typedef s32 GskAssetType;

typedef struct gsk_AssetBlocInfo
{
    u32 bloc_offset;
    u32 bloc_length;
    u8 bloc_pages[2];
} gsk_AssetBlocInfo;

typedef struct gsk_AssetRef
{
    u64 asset_handle;
    u32 asset_uri_index; // index to uri in cache uri-array
    u8 is_imported;      // is asset-data imported
    u8 is_utilized;      // is asset-data utilized by the runtime
    u8 is_baked;         // when baked, bloc_info required.
    void *p_data_import; // pointer to raw imported data
    void *p_data_active; // pointer to full asset data
    void *p_fallback;    // pointer to fallback asset
    gsk_AssetBlocInfo bloc_info;
} gsk_AssetRef;

typedef struct gsk_AssetBlob
{
    u8 is_serialized;
    void *p_buffer;
    u32 buffer_len;
    GskAssetType asset_type;
} gsk_AssetBlob;

typedef struct gsk_AssetData
{
    void *buff_source; // buffer for imported data
    void *buff_active; // buffer for utilized data
    u32 buff_source_len;
    u32 buff_active_len;
} gsk_AssetData;

typedef struct gsk_AssetModelOptions
{
    f32 scale;
    u8 import_materials;

} gsk_AssetModelOptions;

#ifdef __cplusplus
}
#endif //_cplusplus

#endif // __ASSETDEFS_H__