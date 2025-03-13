/*
 * Copyright (c) 2023-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#include "billboard.h"

#include "util/filesystem.h"
#include "util/gfx.h"
#include "util/maths.h"

#include "core/device/device.h"
#include "core/drivers/opengl/opengl.h"
#include "core/graphics/material/material.h"
#include "core/graphics/mesh/primitives.h"

gsk_Billboard2D *
gsk_billboard_create(const char *texturePath, vec2 size)
{
    if (GSK_DEVICE_API_VULKAN)
    {
        LOG_WARN("billboard creation not implemented for vulkan");
        return NULL;
    }

    gsk_Billboard2D *ret = malloc(sizeof(gsk_Billboard2D));
    if (ret == NULL)
    {
        LOG_CRITICAL("failed to allocate memory for billboard");
    }

    ret->vao       = gsk_gl_vertex_array_create();
    float *rectPos = prim_vert_rect();

    gsk_GlVertexBuffer *vbo = gsk_gl_vertex_buffer_create(
      rectPos, (2 * 3 * 4) * sizeof(float), GskOglUsageType_Dynamic);
    gsk_gl_vertex_buffer_bind(vbo);
    gsk_gl_vertex_buffer_push(vbo, 2, GL_FLOAT, GL_FALSE);
    gsk_gl_vertex_buffer_push(vbo, 2, GL_FLOAT, GL_FALSE);
    gsk_gl_vertex_array_add_buffer(ret->vao, vbo);

    ret->texture = texture_create(
      texturePath, NULL, (TextureOptions) {1, GL_RGBA, true, true});

    ret->material = gsk_material_create(
      NULL, GSK_PATH("gsk://shaders/billboard.shader"), 1, ret->texture);

    return ret;
}

void
gsk_billboard_draw(gsk_Billboard2D *self, vec3 position)
{
    if (GSK_DEVICE_API_VULKAN) { return NULL; }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    gsk_material_use(self->material);

    // send world-translation to shader
    glUniform3fv(
      glGetUniformLocation(self->material->shaderProgram->id, "u_Position"),
      1,
      (float *)position);

    gsk_gl_vertex_array_bind(self->vao);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 6);

    glDisable(GL_BLEND);
}
