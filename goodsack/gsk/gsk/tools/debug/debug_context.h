/*
 * Copyright (c) 2023, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#ifndef __DEBUG_CONTEXT_H__
#define __DEBUG_CONTEXT_H__

#include "core/drivers/opengl/opengl.h"
#include "core/graphics/material/material.h"

typedef struct DebugContext
{
    VAO *vaoCube;
    VAO *vaoBoundingBox;
    Material *material;

    VAO *vaoLine; // VAO for debug draw line

} DebugContext;

DebugContext *
debug_context_init();

#endif // __DEBUG_CONTEXT_H__
