/*
 * Copyright (c) 2022-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#ifndef __GSK_DEBUG_DRAW_SPHERE_H__
#define __GSK_DEBUG_DRAW_SPHERE_H__

#include "core/graphics/mesh/mesh.h"
#include "tools/debug/debug_context.h"

void
gsk_debug_draw_sphere(gsk_DebugContext *p_debug_context,
                      f32 radius,
                      mat4 modelMatrix,
                      vec4 color);

#endif // __GSK_DEBUG_DRAW_SPHERE_H__