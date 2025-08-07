/*
 * Copyright (c) 2023-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#include "debug_draw_bounds.h"

#if 0
static void
_draw_aabb(BoundingBox *box, mat4 *modelMatrix)
{
    // Draw Bounding Box

    mat4 bbMat4 = GLM_MAT4_ZERO_INIT;

    glm_aabb_transform(box->corners, *modelMatrix, box->corners);
    vec3 center2;
    vec3 size2 = {
      box->corners[1][0] - box->corners[0][0],
      box->corners[1][1] - box->corners[0][1],
      box->corners[1][2] - box->corners[0][2],
    };
    glm_aabb_center(box->corners, center2);

    mat4 m4Transform = GLM_MAT4_IDENTITY_INIT;
    // glm_mat4_mul(transform->mvp.model, m4Transform, bbMat4);
    glm_translate(m4Transform, center2);
    glm_scale(m4Transform, size2);

    // glm_mat4_mul(transform->mvp.model, m4Transform, bbMat4);
    gsk_shader_use(bbox_shader_static);

    glUniformMatrix4fv(glGetUniformLocation(bbox_shader_static->id, "u_Model"),
                       1,
                       GL_FALSE,
                       (float *)m4Transform);

    vec4 color = {0, 0, 1, 1};
    glUniform4fv(
      glGetUniformLocation(bbox_shader_static->id, "u_Color"), 1, color);

    glEnable(GL_POLYGON_OFFSET_FILL);
    vao_bind(bbox_vao_static);
    glDrawElements(GL_LINE_LOOP, 4, GL_UNSIGNED_INT, 0);
    glDrawElements(
      GL_LINE_LOOP, 4, GL_UNSIGNED_INT, (GLvoid *)(4 * sizeof(u32)));
    glDrawElements(GL_LINES, 8, GL_UNSIGNED_INT, (GLvoid *)(8 * sizeof(u32)));
}
#endif

void
gsk_debug_draw_bounds(gsk_DebugContext *debugContext,
                      vec3 corners[2],
                      mat4 modelMatrix,
                      vec4 color)
{
    gsk_gl_vertex_array_bind(debugContext->vaoBoundingBox);
    gsk_material_use(debugContext->material);

    // Draw OBB

    mat4 bbMat4 = GLM_MAT4_ZERO_INIT;

    vec3 center;
    vec3 size = {
      corners[1][0] - corners[0][0],
      corners[1][1] - corners[0][1],
      corners[1][2] - corners[0][2],
    };
    glm_aabb_center(corners, center);

    mat4 m4Transform = GLM_MAT4_IDENTITY_INIT;
    glm_mat4_mul(modelMatrix, m4Transform, bbMat4);
    glm_translate(m4Transform, center);
    glm_scale(m4Transform, size);

    glm_mat4_mul(modelMatrix, m4Transform, bbMat4);

    u32 shaderId = debugContext->material->shaderProgram->id;

    glUniformMatrix4fv(
      glGetUniformLocation(shaderId, "u_Model"), 1, GL_FALSE, (float *)bbMat4);

    glUniform4fv(glGetUniformLocation(shaderId, "u_Color"), 1, color);

    // glDisable(GL_DEPTH_TEST);
    glEnable(GL_POLYGON_OFFSET_FILL);
    gsk_gl_vertex_array_bind(debugContext->vaoBoundingBox);
    glDrawElements(GL_LINE_LOOP, 4, GL_UNSIGNED_INT, 0);
    glDrawElements(
      GL_LINE_LOOP, 4, GL_UNSIGNED_INT, (GLvoid *)(4 * sizeof(u32)));
    glDrawElements(GL_LINES, 8, GL_UNSIGNED_INT, (GLvoid *)(8 * sizeof(u32)));
    // glEnable(GL_DEPTH_TEST);
}
