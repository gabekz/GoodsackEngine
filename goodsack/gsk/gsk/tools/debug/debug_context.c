/*
 * Copyright (c) 2023, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#include "debug_context.h"

#include "util/array_list.h"
#include "util/filesystem.h"

#include "core/device/device.h"
#include "core/drivers/opengl/opengl.h"

#include "tools/debug/debug_draw_line.h"

#include "core/graphics/material/material.h"
#include "core/graphics/mesh/primitives.h"

#include "asset/asset.h"

#define DRAW_MESH_ONLY 0

gsk_DebugContext *
gsk_debug_context_init()
{
    gsk_DebugContext *ret = malloc(sizeof(gsk_DebugContext));

    // Create debug markers list
    ret->markers_list = malloc(sizeof(ArrayList));
    *(ArrayList *)ret->markers_list =
      array_list_init(sizeof(gsk_DebugMarker), 64);

    if (GSK_DEVICE_API_OPENGL)
    {
        // Assets
        ret->material     = GSK_ASSET("gsk://shaders/basic_unlit.shader");
        ret->model_sphere = GSK_ASSET("gsk://models/pyramid.obj");

        // VAO Cube
        ret->vaoCube    = gsk_gl_vertex_array_create();
        float *vertices = PRIM_ARR_V_CUBE;
        for (int i = 0; i < PRIM_SIZ_V_CUBE; i++)
        {
            vertices[i] *= 0.02f;
        }
        gsk_GlVertexBuffer *vboCube = gsk_gl_vertex_buffer_create(
          vertices, PRIM_SIZ_V_CUBE * sizeof(float));
        gsk_gl_vertex_buffer_push(vboCube, 3, GL_FLOAT, GL_FALSE);
        gsk_gl_vertex_array_add_buffer(ret->vaoCube, vboCube);

        gsk_GlIndexBuffer *ibo = gsk_gl_index_buffer_create(
          PRIM_ARR_I_CUBE, PRIM_SIZ_I_CUBE * sizeof(u32));
        gsk_gl_index_buffer_bind(ibo);

        // VAO Bounding box
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

        // VAO Line
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

void
gsk_debug_markers_push(gsk_DebugContext *p_debug_context,
                       u8 type,
                       u32 id,
                       vec3 position,
                       vec3 pos_end,
                       f32 length,
                       vec4 color,
                       u8 persist)
{
    gsk_DebugMarker marker = {
      .type        = type,
      .id          = id,
      .persist     = persist,
      .line.length = length,
    };

    glm_vec3_copy(position, marker.position);
    glm_vec3_copy(pos_end, marker.line.direction);
    glm_vec4_copy(color, marker.color);

    for (u32 i = 0; i < p_debug_context->markers_list->list_next; i++)
    {

        gsk_DebugMarker *cnt_marker =
          &((gsk_DebugMarker *)p_debug_context->markers_list->data.buffer)[i];

        if (cnt_marker->id == id)
        {
            if (!cnt_marker->persist)
            {
                glm_vec3_copy(position, cnt_marker->position);      // HACK
                glm_vec3_copy(pos_end, cnt_marker->line.direction); // HACK
                glm_vec4_copy(color, cnt_marker->color);            // HACK
            }
            return;
        }
    }

    array_list_push(p_debug_context->markers_list, &marker);
}

void
gsk_debug_markers_render(gsk_DebugContext *p_debug_context)
{
    for (u32 i = 0; i < p_debug_context->markers_list->list_next; i++)
    {

        gsk_DebugMarker *cnt_marker =
          &((gsk_DebugMarker *)p_debug_context->markers_list->data.buffer)[i];

        if (cnt_marker->type == MARKER_RAY)
        {
            gsk_debug_draw_ray(p_debug_context,
                               cnt_marker->position,
                               cnt_marker->line.direction,
                               cnt_marker->line.length,
                               cnt_marker->color);
            continue;
        }

        // TODO: Move to gsk_debug_draw_point()

#if DRAW_MESH_ONLY
        gsk_Mesh *mesh = p_debug_context->model_sphere->meshes[0];
        gsk_gl_vertex_array_bind(mesh->vao);
#else
        gsk_gl_vertex_array_bind(p_debug_context->vaoCube);
#endif // DRAW_MESH_ONLY

        gsk_material_use(p_debug_context->material);

        mat4 model = GLM_MAT4_IDENTITY_INIT;
        glm_translate(model, cnt_marker->position);
        glm_scale(model, (vec3) {5.0f, 5.0f, 5.0f});

        glUniformMatrix4fv(
          glGetUniformLocation(p_debug_context->material->shaderProgram->id,
                               "u_Model"),
          1,
          GL_FALSE,
          (float *)model);

        vec4 color = {0, 1, 0, 1};
        glm_vec4_copy(cnt_marker->color, color);
        glUniform4fv(glGetUniformLocation(
                       p_debug_context->material->shaderProgram->id, "u_Color"),
                     1,
                     color);

#if DRAW_MESH_ONLY
        // glDrawArrays(GL_TRIANGLES, 0, mesh->meshData->vertexCount);
        glDrawArrays(GL_LINE_STRIP_ADJACENCY, 0, mesh->meshData->vertexCount);
#else
        glDrawElements(
          GL_TRIANGLE_STRIP, PRIM_SIZ_I_CUBE, GL_UNSIGNED_INT, NULL);
#endif // DRAW_MESH_ONLY
    }
}
