/*
 * Copyright (c) 2022-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#include "debug_draw_sphere.h"

void
gsk_debug_draw_sphere(gsk_DebugContext *p_debug_context,
                      f32 radius,
                      mat4 modelMatrix,
                      vec4 color)
{
    glDepthMask(GL_FALSE);

    gsk_material_use(p_debug_context->material);

    f32 offset = radius + 0.01f;
    vec3 scale = {offset, offset, offset};

    mat4 m4Transform = GLM_MAT4_IDENTITY_INIT;
    glm_mat4_copy(modelMatrix, m4Transform);
    glm_scale(m4Transform, scale);

    u32 shaderId = p_debug_context->material->shaderProgram->id;

    glUniformMatrix4fv(glGetUniformLocation(shaderId, "u_Model"),
                       1,
                       GL_FALSE,
                       (float *)m4Transform);

    glUniform4fv(glGetUniformLocation(shaderId, "u_Color"), 1, color);

    // glDisable(GL_DEPTH_TEST);
    // glEnable(GL_POLYGON_OFFSET_FILL);
    // glEnable(GL_DEPTH_TEST);

    gsk_Mesh *mesh_sphere = p_debug_context->mesh_sphere;
    gsk_gl_vertex_array_bind(mesh_sphere->vao);
    glDrawArrays(GL_LINE_LOOP, 0, mesh_sphere->meshData->vertexCount);
    glDepthMask(GL_TRUE);
}