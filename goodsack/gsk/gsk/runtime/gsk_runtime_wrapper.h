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

gsk_ECS *
gsk_runtime_get_ecs();

gsk_Renderer *
gsk_runtime_get_renderer();

gsk_AssetCache *
gsk_runtime_get_asset_cache(const char *uri_str);

gsk_AssetRef *
gsk_runtime_get_fallback_asset(GskAssetType type);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __GSK_RUNTIME_WRAPPER_H__