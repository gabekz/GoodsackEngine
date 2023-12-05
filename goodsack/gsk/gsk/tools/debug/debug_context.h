/*
 * Copyright (c) 2023, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#ifndef __DEBUG_CONTEXT_H__
#define __DEBUG_CONTEXT_H__

#include "core/drivers/opengl/opengl.h"
#include "core/graphics/material/material.h"

typedef struct gsk_DebugContext
{
    gsk_GlVertexArray *vaoCube;
    gsk_GlVertexArray *vaoBoundingBox;
    gsk_Material *material;

    gsk_GlVertexArray *vaoLine; // VAO for debug draw line

} gsk_DebugContext;

gsk_DebugContext *
gsk_debug_context_init();

#endif // __DEBUG_CONTEXT_H__
