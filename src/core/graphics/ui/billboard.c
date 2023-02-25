#include "billboard.h"

#include <core/drivers/opengl/opengl.h>
#include <core/graphics/material/material.h>
#include <core/graphics/model/primitives.h>

#include <util/gfx.h>

Billboard2D *
billboard_create(const char *texturePath, vec2 size)
{
    Billboard2D *ret = malloc(sizeof(Billboard2D));

    ret->vao       = vao_create();
    float *rectPos = prim_vert_rect();
    VBO *vbo       = vbo_create(rectPos, (2 * 3 * 4) * sizeof(float));
    vbo_bind(vbo);
    vbo_push(vbo, 2, GL_FLOAT, GL_FALSE);
    vbo_push(vbo, 2, GL_FLOAT, GL_FALSE);
    vao_add_buffer(ret->vao, vbo);

    ret->texture = texture_create(
      "../res/textures/gizmo/light.png", GL_RGBA, false, 1, NULL);

    ret->material =
      material_create(NULL, "../res/shaders/billboard.shader", 1, ret->texture);

    return ret;
}

void
billboard_draw(Billboard2D *self)
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    material_use(self->material);
    vao_bind(self->vao);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 6);
}
