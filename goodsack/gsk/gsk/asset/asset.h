/*
 * Copyright (c) 2024, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#ifndef __ASSET_H__
#define __ASSET_H__

#include "asset/asset_cache.h"

#ifdef __cplusplus
#include "runtime/gsk_runtime.hpp"
#define GSK_ASSET(x) gsk_asset_get(gsk::runtime::rt_get_asset_cache(), x)
#else
#include "runtime/gsk_runtime_wrapper.h"
#define GSK_ASSET(x) gsk_asset_get(gsk_runtime_get_asset_cache(), x)
#endif // __cplusplus

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

void *
gsk_asset_get(gsk_AssetCache *p_cache, const char *str_uri);

void
gsk_asset_get_handle(gsk_AssetCache *p_cache, u64 handle);

void *
gsk_asset_alloc(gsk_AssetCache *p_cache, u64 handle);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __ASSET_H__
