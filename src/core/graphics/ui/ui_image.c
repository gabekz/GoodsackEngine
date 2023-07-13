#include "ui_image.h"

#include <core/drivers/opengl/opengl.h>
#include <core/graphics/material/material.h>
#include <core/graphics/mesh/primitives.h>

#include <util/gfx.h>
#include <util/maths.h>

#define UI_EL_WIDTH  30
#define UI_EL_HEIGHT 30

#define UI_EL_POS_X (1920 / 2)
#define UI_EL_POS_Y (1080 / 2)

#define UI_X_OFF (UI_EL_WIDTH / 2)
#define UI_Y_OFF (UI_EL_HEIGHT / 2)

// LR, LL, TL,
// TR, LR, LL

// clang-format off
#define UI_V_ARRAY_PLANE                                                     \
    (float[])                                                                \
    {                                                                        \
        UI_EL_POS_X + UI_X_OFF, UI_EL_POS_Y - UI_Y_OFF, 1.0f, 0.0f, \
        UI_EL_POS_X - UI_X_OFF, UI_EL_POS_Y - UI_Y_OFF, 0.0f, 0.0f, \
        UI_EL_POS_X - UI_X_OFF, UI_EL_POS_Y + UI_Y_OFF, 0.0f, 1.0f, \
        UI_EL_POS_X + UI_X_OFF, UI_EL_POS_Y + UI_Y_OFF, 1.0f, 1.0f, \
        UI_EL_POS_X + UI_X_OFF, UI_EL_POS_Y - UI_Y_OFF, 1.0f, 0.0f, \
        UI_EL_POS_X - UI_X_OFF, UI_EL_POS_Y + UI_Y_OFF, 0.0f, 1.0f, \
    }
#define UI_V_ARRAY_PLANE_COUNT 24

Image2D *
ui_image_create(const char *texture_path, vec2 size)
{
    Image2D *ret = malloc(sizeof(Image2D));

    ret->vao = vao_create();
    float *rectPos = UI_V_ARRAY_PLANE;

    VBO *vbo = vbo_create(rectPos, (2 * 3 * 4) * sizeof(float));
    vbo_bind(vbo);
    vbo_push(vbo, 2, GL_FLOAT, GL_FALSE);
    vbo_push(vbo, 2, GL_FLOAT, GL_FALSE);
    vao_add_buffer(ret->vao, vbo);

    ret->texture = texture_create("../res/textures/gizmo/crosshair.png",
                                  NULL,
                                  (TextureOptions) {0, GL_RGBA, false, true});

    ret->material =
      material_create(NULL, "../res/shaders/canvas2d.shader", 1, ret->texture);

    return ret;
}

void
ui_image_draw(Image2D *self)
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);

    material_use(self->material);

#if 0
    // send world-translation to shader
    glUniform3fv(
      glGetUniformLocation(self->material->shaderProgram->id, "u_Position"),
      1,
      (float *)position);
#endif

    vao_bind(self->vao);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}
