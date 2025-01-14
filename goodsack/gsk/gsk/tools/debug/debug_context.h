/*
 * Copyright (c) 2023-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#ifndef __DEBUG_CONTEXT_H__
#define __DEBUG_CONTEXT_H__

#include "util/array_list.h"
#include "util/maths.h"
#include "util/sysdefs.h"

#include "core/drivers/opengl/opengl.h"
#include "core/graphics/material/material.h"
#include "core/graphics/mesh/model.h"

typedef enum GskDebugMarkerType {
    MARKER_POINT = 0,
    MARKER_LINE,
    MARKER_RAY,
} GskDebugMarkerType;

typedef struct gsk_DebugMarker
{
    u32 type, id;
    vec3 position;
    vec4 color;
    u8 persist;
    struct
    {
        vec3 end_pos, direction;
        f32 length;
    } line;

} gsk_DebugMarker;

typedef struct gsk_DebugContext
{
    gsk_GlVertexArray *vaoCube;
    gsk_GlVertexArray *vaoBoundingBox;
    gsk_Material *material;

    gsk_Model *model_sphere;

    gsk_GlVertexArray *vaoLine; // VAO for debug draw line
    u32 vboLineId;              // Line VBO ID

    ArrayList *markers_list;

} gsk_DebugContext;

gsk_DebugContext *
gsk_debug_context_init();

void
gsk_debug_markers_push(gsk_DebugContext *p_debug_context,
                       u8 type,
                       u32 id,
                       vec3 position,
                       vec3 pos_end,
                       f32 length,
                       vec4 color,
                       u8 persist);

void
gsk_debug_markers_render(gsk_DebugContext *p_debug_context);

#endif // __DEBUG_CONTEXT_H__
