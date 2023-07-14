// clang-format off

#include "gui_element.h"

#include <core/drivers/opengl/opengl.h>
#include <core/graphics/material/material.h>
#include <core/graphics/mesh/primitives.h>

#include <util/gfx.h>
#include <util/sysdefs.h>
#include <util/maths.h>

#define USING_SPRITE_SHEET FALSE
#define USING_BLENDING     FALSE

#define S_X 1.0f
#define S_Y 3.0f

#if USING_SPRITE_SHEET
#define S_SH_W 256.0f
#define S_SH_H 256.0f

#define S_SP_W 32.0f
#define S_SP_H 32.0f

#define S_POS_X (((S_X + 1) * S_SP_W) / S_SH_W)
#define S_NEG_X ((S_X * S_SP_W) / S_SH_W)

#define S_POS_Y (((S_Y + 1) * S_SP_H) / S_SH_H)
#define S_NEG_Y ((S_Y * S_SP_H) / S_SH_H)
#else
#define S_POS_X 1
#define S_POS_Y 1
#define S_NEG_X 0 
#define S_NEG_Y 0
#endif


GuiElement *
gui_element_create(vec2 position, vec2 size, Texture *p_texture)
{
    GuiElement *ret = malloc(sizeof(GuiElement));

    glm_vec2_copy(position, ret->position);
    glm_vec2_copy(size, ret->size);

    float pos_x = position[0];
    float pos_y = position[1];

    float size_x_off = size[0] / 2;
    float size_y_off = size[1] / 2;

    ret->vao = vao_create();
    float *rectPos = 
    (float[])
    {
         size_x_off , -size_y_off, S_POS_X, S_NEG_Y,
        -size_x_off , -size_y_off, S_NEG_X, S_NEG_Y,
        -size_x_off ,  size_y_off, S_NEG_X, S_POS_Y,
         size_x_off ,  size_y_off, S_POS_X, S_POS_Y,
         size_x_off , -size_y_off, S_POS_X, S_NEG_Y,
        -size_x_off ,  size_y_off, S_NEG_X, S_POS_Y,
    };

    VBO *vbo = vbo_create(rectPos, (2 * 3 * 4) * sizeof(float));
    vbo_bind(vbo);
    vbo_push(vbo, 2, GL_FLOAT, GL_FALSE);
    vbo_push(vbo, 2, GL_FLOAT, GL_FALSE);
    vao_add_buffer(ret->vao, vbo);

    ret->using_texture = (p_texture != NULL) ? TRUE : FALSE;

    ret->material =
      material_create(NULL, "../res/shaders/canvas2d.shader", 0, NULL);

    if(ret->using_texture) {
        ret->texture = p_texture;
        material_add_texture(ret->material, ret->texture);
    }

    return ret;
}

void
gui_element_draw(GuiElement *self)
{
#if USING_BLENDING
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);
#endif

    material_use(self->material);

    // send position to shader
    glUniform2fv(
      glGetUniformLocation(self->material->shaderProgram->id, "u_position"),
      1,
      (float *)self->position);

    // send texture info
    glUniform1i(
      glGetUniformLocation(self->material->shaderProgram->id, "u_using_texture"),
      self->using_texture);

    vao_bind(self->vao);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}
