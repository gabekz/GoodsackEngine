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

#define GCFG_CHECK(v, x) gsk_asset_gcfg_check_type(v, x)

typedef enum GskGCFGItemType_ {
    GskGCFGItemType_None = 0,
    GskGCFGItemType_Number,
    GskGCFGItemType_Bool,
    GskGCFGItemType_String,
} GskGCFGItemType_;

typedef s32 GskGCFGItemType;

typedef struct gsk_GCFGItem
{
    const char *key;
    const char *value;
    GskGCFGItemType type;
} gsk_GCFGItem;

typedef struct gsk_GCFG
{
    // u64 asset_handles[3];
    u32 asset_type;
    ArrayList list_items;

} gsk_GCFG;

#if 0
void
gsk_asset_gcfg_load_all(gsk_AssetCache *p_cache);
#endif

void
gsk_asset_gcfg_set_config(gsk_GCFG *p_gcfg);

u8
gsk_asset_gcfg_check_type(const gsk_GCFGItem *p_item,
                          const GskGCFGItemType type);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __ASSET_GCFG_H__