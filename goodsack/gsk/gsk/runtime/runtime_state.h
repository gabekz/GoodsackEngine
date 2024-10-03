/*
 * Copyright (c) 2024, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#ifndef __GSK_RUNTIME_STATE_H__
#define __GSK_RUNTIME_STATE_H__

#include "asset/asset_cache.h"
#include "core/graphics/renderer/v1/renderer.h"
#include "entity/ecs.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

typedef struct gsk_RuntimeState
{
    gsk_ECS *p_ecs;
    gsk_Renderer *p_renderer;
    gsk_AssetCache *p_asset_cache;
} gsk_RuntimeState;

gsk_RuntimeState
gsk_runtime_state_init();

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __GSK_RUNTIME_STATE_H__