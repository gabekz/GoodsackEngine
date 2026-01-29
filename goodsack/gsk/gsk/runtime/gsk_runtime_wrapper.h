/*
 * Copyright (c) 2025-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#ifndef __GSK_RUNTIME_WRAPPER_H__
#define __GSK_RUNTIME_WRAPPER_H__

#include "asset/asset_cache.h"
#include "core/graphics/renderer/v1/renderer.h"
#include "entity/ecs.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

u8
gsk_runtime_check_layer_mask(u32 layer_a, u32 layer_b);

gsk_ECS *
gsk_runtime_get_ecs();

gsk_Renderer *
gsk_runtime_get_renderer();

gsk_AssetCache *
gsk_runtime_get_asset_cache_index(u32 index);

gsk_AssetCache *
gsk_runtime_get_asset_cache(const char *uri_str);

gsk_AssetRef *
gsk_runtime_get_fallback_asset(GskAssetType type);

char *
gsk_runtime_get_startup_map();

gsk_EntityId
gsk_runtime_get_hovered_entity_id();

void
gsk_runtime_set_debug_entity_id(gsk_EntityId entity_id);

gsk_EntityId
gsk_runtime_get_debug_entity_id();

void *
gsk_runtime_get_lua_state();

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __GSK_RUNTIME_WRAPPER_H__