/*
 * Copyright (c) 2024, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#ifndef __ASSET_H__
#define __ASSET_H__

#include "asset/asset_cache.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

void *
gsk_asset_get_str(gsk_AssetCache *p_cache, const char *str_uri);

void
gsk_asset_get_handle(gsk_AssetCache *p_cache, u64 handle);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __ASSET_H__
