/*
 * Copyright (c) 2023, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

// TODO: Refactor this file in next commit

#ifndef __GSK_RUNTIME_HPP__
#define __GSK_RUNTIME_HPP__

#include "util/sysdefs.h"

#include "asset/asset_cache.h"
#include "core/graphics/renderer/v1/renderer.h"
#include "entity/ecs.h"

// #define RENDERER_2
#define USING_LUA                    1
#define USING_RUNTIME_LOADING_SCREEN 1
#define USING_JOYSTICK_CONTROLLER    0

// Starting cursor state
#define INIT_CURSOR_LOCKED  1
#define INIT_CURSOR_VISIBLE 0

#define GSK_RUNTIME_USE_DEBUG 1

namespace gsk {
namespace runtime {

u32
rt_setup(const char *root_dir, const char *root_scheme, int argc, char *argv[]);

void
rt_loop();

gsk_ECS *
rt_get_ecs();
gsk_Renderer *
rt_get_renderer();
gsk_AssetCache *
rt_get_asset_cache();

void
rt_set_scene(u16 sceneIndex);

} // namespace runtime
} // namespace gsk

#endif // __GSK_RUNTIME_HPP__