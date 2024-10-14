/*
 * Copyright (c) 2024, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#ifndef __ASSET_GCFG_H__
#define __ASSET_GCFG_H__

#include "asset/asset_cache.h"

#include "util/array_list.h"
#include "util/sysdefs.h"

#define GCFG_KEY_MAX_LEN   256
#define GCFG_VALUE_MAX_LEN 256

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

typedef struct gsk_GCFGItem
{
    const char *key;
    const char *value;
} gsk_GCFGItem;

typedef struct gsk_GCFG
{
    // u64 asset_handles[3];
    u32 asset_type;
    ArrayList list_items;

} gsk_GCFG;

void
gsk_asset_gcfg_load_all(gsk_AssetCache *p_cache);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __ASSET_GCFG_H__