/*
 * Copyright (c) 2024, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#ifndef __GSK_ASSET_H__
#define __GSK_ASSET_H__

#include "asset/asset_cache.h"
#include "util/sysdefs.h"

#define GSK_ASSET_FETCH_VALIDATE 2
#define GSK_ASSET_FETCH_IMPORT   3
#define GSK_ASSET_FETCH_ALL      4

#ifdef __cplusplus
#include "runtime/gsk_runtime.hpp"
#define GSK_ASSET(x)                                              \
    _gsk_asset_get_internal(                                      \
      gsk::runtime::rt_get_asset_cache(), x, GSK_ASSET_FETCH_ALL) \
      ->p_data_active
#else
#include "runtime/gsk_runtime_wrapper.h"
#define GSK_ASSET(x)                                         \
    _gsk_asset_get_internal(                                 \
      gsk_runtime_get_asset_cache(), x, GSK_ASSET_FETCH_ALL) \
      ->p_data_active
#endif // __cplusplus

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

typedef void (*gsk_CreateAssetFptr)(const char *uri,
                                    void *p_options,
                                    void *p_dest);

typedef void (*gsk_ImportAssetFptr)(const char *uri,
                                    void *p_options,
                                    void *p_dest);

typedef void (*gsk_LoadAssetFptr)(gsk_AssetBlob *p_blob,
                                  void *p_options,
                                  void *p_dest);

gsk_AssetRef *
_gsk_asset_get_internal(gsk_AssetCache *p_cache,
                        const char *str_uri,
                        u8 fetch_mode);

// void
// gsk_asset_load_all_gcfg(gsk_AssetCache *p_cache);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __GSK_ASSET_H__
