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

#define DEBUG_MARKERS_FIXED_ID 4

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

typedef struct gsk_DebugPhysicsOptions
{
    u8 selected_entity_only;
    u8 draw_collisions;
    u8 draw_friction;

} gsk_DebugPhysicsOptions;

typedef struct gsk_DebugContext
{
    gsk_GlVertexArray *vaoCube;
    gsk_GlVertexArray *vaoBoundingBox;
    gsk_Material *material;

    gsk_Model *model_sphere;
    gsk_Mesh *mesh_sphere;

    gsk_GlVertexArray *vaoLine; // VAO for debug draw line
    u32 vboLineId;              // Line VBO ID

    ArrayList *markers_list;
    ArrayList *markers_list_fixed;

    gsk_DebugPhysicsOptions physics_options;
    u8 is_active;

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

void
gsk_debug_markers_clear(gsk_DebugContext *p_debug_context, u32 clear_id);

#endif // __DEBUG_CONTEXT_H__
