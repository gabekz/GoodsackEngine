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

typedef struct gsk_AssetRef
{
    u64 asset_handle;
    u32 asset_uri_index; // index to uri in cache uri-array
    u8 is_imported;      // is asset-data imported
    u8 is_utilized;      // is asset-data utilized by the runtime
    void *p_data_active; // pointer to full asset data

} gsk_AssetRef;

typedef struct gsk_AssetBlob
{
    void *p_buffer;
    u32 buffer_len;
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