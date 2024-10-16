/*
 * Copyright (c) 2023, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#ifndef __DEBUG_DRAW_LINE_H__
#define __DEBUG_DRAW_LINE_H__

#include "tools/debug/debug_context.h"
#include "util/maths.h"

void
gsk_debug_draw_line(gsk_DebugContext *debugContext,
                    vec3 start,
                    vec3 end,
                    vec4 color);

void
gsk_debug_draw_ray(gsk_DebugContext *debugContext,
                   vec3 start,
                   vec3 direction,
                   f32 length,
                   vec4 color);

#endif // __DEBUG_DRAW_LINE_H__
