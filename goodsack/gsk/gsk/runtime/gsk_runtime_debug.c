/*
 * Copyright (c) 2022-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */
#include "gsk_runtime_debug.h"

#include "gsk_runtime_wrapper.h"

#include "util/maths.h"
#include "util/sysdefs.h"

void
_gsk_debug_draw_internal(
  u8 type, vec4 pos_start, vec4 pos_end, f32 length, vec4 color)
{

    gsk_ECS *p_ecs = gsk_runtime_get_ecs();

    gsk_DebugContext *p_debug_context = p_ecs->renderer->debugContext;

    s32 debug_id = (p_ecs->current_event == ECS_FIXED_UPDATE ||
                    p_ecs->current_event == ECS_ON_COLLIDE)
                     ? DEBUG_MARKERS_FIXED_ID
                     : 0;

    gsk_debug_markers_push(p_debug_context,
                           type,
                           debug_id,
                           pos_start,
                           pos_end,
                           length,
                           color,
                           FALSE);
}