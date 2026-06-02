/*
 * Copyright (c) 2022-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#ifndef __GSK_RUNTIME_DEBUG_H__
#define __GSK_RUNTIME_DEBUG_H__

#include "tools/debug/debug_context.h"

#include "util/maths.h"
#include "util/sysdefs.h"

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

#define GSK_DEBUG_DRAW_LINE(start, end, color) \
    _gsk_debug_draw_internal(MARKER_LINE, start, end, 0, color)

#define GSK_DEBUG_DRAW_RAY(start, direction, length, color) \
    _gsk_debug_draw_internal(MARKER_RAY, start, direction, length, color)

#define GSK_DEBUG_DRAW_POINT(pos, size, color) \
    _gsk_debug_draw_internal(MARKER_POINT, pos, (vec3) {0, 0, 0}, size, color)

void
_gsk_debug_draw_internal(
  u8 type, vec4 pos_start, vec4 pos_end, f32 length, vec4 color);

#ifdef __cplusplus
}
#endif //__cplusplus

#endif // __GSK_RUNTIME_DEBUG_H__