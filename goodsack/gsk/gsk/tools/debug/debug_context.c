/*
 * Copyright (c) 2023, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#include "debug_context.h"

#include "util/filesystem.h"

#include "core/device/device.h"
#include "core/drivers/opengl/opengl.h"

#include "core/graphics/material/material.h"
#include "core/graphics/mesh/primitives.h"

gsk_DebugContext *
gsk_debug_context_init()
{
    gsk_DebugContext *ret = malloc(sizeof(gsk_DebugContext));

    if (GSK_DEVICE_API_OPENGL) {
        ret->vaoCube = gsk_gl_vertex_array_create();

        float *vertices = PRIM_ARR_V_CUBE;
        for (int i = 0; i < PRIM_SIZ_V_CUBE; i++) {
            vertices[i] *= 0.02f;
        }

        gsk_GlVertexBuffer *vboCube = gsk_gl_vertex_buffer_create(
          vertices, PRIM_SIZ_V_CUBE * sizeof(float));
        gsk_gl_vertex_buffer_push(vboCube, 3, GL_FLOAT, GL_FALSE);
        gsk_gl_vertex_array_add_buffer(ret->vaoCube, vboCube);

        gsk_GlIndexBuffer *ibo = gsk_gl_index_buffer_create(
          PRIM_ARR_I_CUBE, PRIM_SIZ_I_CUBE * sizeof(u32));
        gsk_gl_index_buffer_bind(ibo);

        ret->material =
          gsk_material_create(NULL, GSK_PATH("gsk://shaders/white.shader"), 0);

        // Bounding box
        ret->vaoBoundingBox = gsk_gl_vertex_array_create();
        gsk_gl_vertex_array_bind(ret->vaoBoundingBox);
        gsk_GlVertexBuffer *vboBoundingBox = gsk_gl_vertex_buffer_create(
          PRIM_ARR_V_CUBE2, PRIM_SIZ_V_CUBE2 * sizeof(float));
        gsk_gl_vertex_buffer_bind(vboBoundingBox);
        gsk_gl_vertex_buffer_push(vboBoundingBox, 4, GL_FLOAT, GL_FALSE);
        gsk_gl_vertex_array_add_buffer(ret->vaoBoundingBox, vboBoundingBox);

        gsk_GlIndexBuffer *iboBoundingBox = gsk_gl_index_buffer_create(
          PRIM_ARR_I_CUBE2, PRIM_SIZ_I_CUBE2 * sizeof(unsigned int));
        gsk_gl_index_buffer_bind(iboBoundingBox);

        // Line VAO
        vec3 lineStart    = GLM_VEC3_ZERO_INIT;
        vec3 lineEnd      = GLM_VEC3_ZERO_INIT;
        float lineverts[] = {lineStart[0],
                             lineStart[1],
                             lineStart[2],
                             lineEnd[0],
                             lineEnd[1],
                             lineEnd[2]};
        ret->vaoLine      = gsk_gl_vertex_array_create();
        gsk_GlVertexBuffer *lineVbo =
          gsk_gl_vertex_buffer_create(lineverts, 6 * sizeof(float));
        gsk_gl_vertex_buffer_push(lineVbo, 3, GL_FLOAT, GL_FALSE);
        gsk_gl_vertex_array_add_buffer(ret->vaoLine, lineVbo);

        // OpenGL Line smoothing
        glLineWidth(1.0f);
        glEnable(GL_LINE_SMOOTH);
        glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    }

    return ret;
}
