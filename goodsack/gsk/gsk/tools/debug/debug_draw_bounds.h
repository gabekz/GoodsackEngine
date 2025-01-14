/*
 * Copyright (c) 2023-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#ifndef __DEBUG_DRAW_BOUNDS_H__
#define __DEBUG_DRAW_BOUNDS_H__

#include "core/graphics/mesh/mesh.h"
#include "tools/debug/debug_context.h"

void
gsk_debug_draw_bounds(gsk_DebugContext *debugContext,
                      vec3 corners[2],
                      mat4 modelMatrix);

#endif // __DEBUG_DRAW_BOUNDS_H__