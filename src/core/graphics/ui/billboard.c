#include "billboard.h"

#include <core/drivers/opengl/opengl.h>
#include <core/graphics/material/material.h>
#include <core/graphics/mesh/primitives.h>

#include <util/gfx.h>
#include <util/maths.h>

Billboard2D *
billboard_create(const char *texturePath, vec2 size)
{
    Billboard2D *ret = malloc(sizeof(Billboard2D));

    ret->vao       = vao_create();
    float *rectPos = prim_vert_rect();

    VBO *vbo = vbo_create(rectPos, (2 * 3 * 4) * sizeof(float));
    vbo_bind(vbo);
    vbo_push(vbo, 2, GL_FLOAT, GL_FALSE);
    vbo_push(vbo, 2, GL_FLOAT, GL_FALSE);
    vao_add_buffer(ret->vao, vbo);

    ret->texture = texture_create("../res/textures/gizmo/light.png",
                                  NULL,
                                  (TextureOptions) {1, GL_RGBA, true, true});

    ret->material =
      material_create(NULL, "../res/shaders/billboard.shader", 1, ret->texture);

    return ret;
}

void
billboard_draw(Billboard2D *self, vec3 position)
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);

    material_use(self->material);

    // send world-translation to shader
    glUniform3fv(
      glGetUniformLocation(self->material->shaderProgram->id, "u_Position"),
      1,
      (float *)position);

    vao_bind(self->vao);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 6);
}
