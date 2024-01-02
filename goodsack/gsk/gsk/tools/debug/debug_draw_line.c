/*
 * Copyright (c) 2023, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#include "debug_draw_line.h"

#include "core/drivers/opengl/opengl.h"
#include "tools/debug/debug_context.h"
#include "util/maths.h"

// NOTE: should take gsk_DebugContext -> contains shader information

// Line start: 0, 0, 0
// Line direction: 0, 1, 0
// Line length: 100
// Line end: 0, 100, 0

void
gsk_debug_draw_line(gsk_DebugContext *debugContext,
                    vec3 start,
                    vec3 end,
                    vec4 color)
{
    float vertices[] = {start[0], start[1], start[2], end[0], end[1], end[2]};

    // TEST DRAW LINE
    gsk_material_use(debugContext->material);
    mat4 bbMat4 = GLM_MAT4_IDENTITY_INIT;
    glUniformMatrix4fv(glGetUniformLocation(
                         debugContext->material->shaderProgram->id, "u_Model"),
                       1,
                       GL_FALSE,
                       (float *)bbMat4);

    // set material color
    glUniform4fv(glGetUniformLocation(debugContext->material->shaderProgram->id,
                                      "u_Color"),
                 1,
                 color);

    gsk_gl_vertex_array_bind(debugContext->vaoLine);
    // Update the line vertices after binding VAO
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), &vertices);
    glDrawArrays(GL_LINES, 0, 2);
}

// TODO: MOVE THIS SOMEWHERE ELSE
void
gsk_debug_draw_ray(gsk_DebugContext *debugContext,
                   vec3 start,
                   vec3 direction,
                   f32 length,
                   vec4 color)
{
    vec3 end; // new end
    glm_vec3_scale(direction, length, end);
    glm_vec3_add(start, end, end); // store the directed ray into end

    // Draw the line
    gsk_material_use(debugContext->material);

    // draw line
    gsk_debug_draw_line(debugContext, start, end, color);

    // ------- Draw arrowhead ------- //

    // arrow matrix
    mat4 matrix_arrow = GLM_MAT4_IDENTITY_INIT;
    vec3 newrot;
    glm_vec3_sub(end, start, newrot);

    glm_translate(matrix_arrow, end);
    mat4 mtest;
    glm_lookat(start, newrot, (vec3) {0, 1, 0}, mtest);
    glm_inv_tr(mtest);
    // glm_mat4_inv(mtest, mtest);
    glm_mat4_mul(matrix_arrow, mtest, matrix_arrow);

#if 0
    glUniform4fv(glGetUniformLocation(debugContext->material->shaderProgram->id,
                                      "u_Color"),
                 1,
                 color);
#endif

    glUniformMatrix4fv(glGetUniformLocation(
                         debugContext->material->shaderProgram->id, "u_Model"),
                       1,
                       GL_FALSE,
                       (float *)matrix_arrow);

    gsk_Mesh *mesh_arrow = debugContext->model_sphere->meshes[0];
    gsk_gl_vertex_array_bind(mesh_arrow->vao);
    glDrawArrays(GL_LINE_LOOP, 0, mesh_arrow->meshData->vertexCount);
}