/*
 * Copyright (c) 2023, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

// TODO: Refactor this file in next commit

#ifndef __GSK_RUNTIME_HPP__
#define __GSK_RUNTIME_HPP__

#include "util/sysdefs.h"

#include "core/graphics/renderer/v1/renderer.h"
#include "entity/ecs.h"

// #define RENDERER_2
#define USING_LUA                    1
#define USING_RUNTIME_LOADING_SCREEN 1
#define USING_JOYSTICK_CONTROLLER    1

// Starting cursor state
#define INIT_CURSOR_LOCKED  1
#define INIT_CURSOR_VISIBLE 0

#define GSK_RUNTIME_USE_DEBUG 1

u32
gsk_runtime_setup(const char *project_root, int argc, char *argv[]);

void
gsk_runtime_loop();

gsk_ECS *
gsk_runtime_get_ecs();
gsk_Renderer *
gsk_runtime_get_renderer();

void
gsk_runtime_set_scene(u16 sceneIndex);

#endif // __GSK_RUNTIME_HPP__