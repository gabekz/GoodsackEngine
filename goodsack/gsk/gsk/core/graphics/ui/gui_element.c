/*
 * Copyright (c) 2023-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

// clang-format off

#include "gui_element.h"

#include "util/gfx.h"
#include "util/sysdefs.h"
#include "util/maths.h"
#include "util/filesystem.h"

#include "core/drivers/opengl/opengl.h"
#include "core/graphics/material/material.h"
#include "core/graphics/mesh/primitives.h"

#include "core/device/device.h"

#define USING_SPRITE_SHEET FALSE

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

gsk_GuiElement *
gsk_gui_element_create(vec2 position, vec2 size, vec3 color, gsk_Texture *p_texture, vec4 tex_coords)
{
    gsk_GuiElement *ret = malloc(sizeof(gsk_GuiElement));

    glm_vec2_copy(position, ret->position);
    glm_vec2_copy(size, ret->size);
    glm_vec3_copy(color, ret->color_rgb);

    float pos_x = position[0];
    float pos_y = position[1];

    float size_x_off = size[0] / 2;
    float size_y_off = size[1] / 2;

    float t_pos_x = S_POS_X;
    float t_neg_x = S_NEG_X;
    float t_pos_y = S_POS_Y;
    float t_neg_y = S_NEG_Y;

    if(tex_coords != NULL) {
        t_pos_x = tex_coords[0];
        t_neg_x = tex_coords[1];
        t_pos_y = tex_coords[2];
        t_neg_y = tex_coords[3];
    }

    ret->vao = gsk_gl_vertex_array_create();
    float *rectPos = 
    (float[])
    {
         size_x_off , -size_y_off, t_pos_x, t_neg_y,
        -size_x_off , -size_y_off, t_neg_x, t_neg_y,
        -size_x_off ,  size_y_off, t_neg_x, t_pos_y,
         size_x_off ,  size_y_off, t_pos_x, t_pos_y,
         size_x_off , -size_y_off, t_pos_x, t_neg_y,
        -size_x_off ,  size_y_off, t_neg_x, t_pos_y,
    };

    gsk_GlVertexBuffer *vbo = gsk_gl_vertex_buffer_create(rectPos, (2 * 3 * 4) * sizeof(float));
    gsk_gl_vertex_buffer_bind(vbo);
    gsk_gl_vertex_buffer_push(vbo, 2, GL_FLOAT, GL_FALSE);
    gsk_gl_vertex_buffer_push(vbo, 2, GL_FLOAT, GL_FALSE);
    gsk_gl_vertex_array_add_buffer(ret->vao, vbo);

    ret->using_texture = (p_texture != NULL) ? TRUE : FALSE;

#if 0
    ret->material =
      gsk_material_create(NULL, GSK_PATH("gsk://shaders/canvas2d.shader"), 0, NULL);
#else
      ret->material = NULL;
#endif

    if(ret->using_texture)
    {
        ret->texture = p_texture;

        if(ret->material) {
          gsk_material_add_texture(ret->material, ret->texture);
        }
    }

    return ret;
}

void
gsk_gui_element_draw(gsk_GuiElement *self, u32 shader_id)
{

#if 0
    if(self->material != NULL) {
      gsk_material_use(self->material);
    }
#endif

  if(self->using_texture == TRUE && self->texture != NULL) {
    texture_bind(self->texture, 0);
  }

  // send position to shader
  glUniform2fv(
    glGetUniformLocation(shader_id, "u_position"),
    1,
    (float *)self->position);

  // send texture info
  glUniform1i(
    glGetUniformLocation(shader_id, "u_using_texture"),
    self->using_texture);

  // send color info
  glUniform3fv(
    glGetUniformLocation(shader_id, "u_color"),
    1,
    (float *)self->color_rgb);

#if 0
  if(self->size[0] == 10 && self->size[1] == 10) {
  
    double x_bounds[2] = {self->position[0] - self->size[0],
                          self->position[0] + self->size[0]};
  
    double y_bounds[2] = {self->position[1] - self->size[1],
                          self->position[1] + self->size[1]};
  
    const gsk_Input input = gsk_device_getInput();
    const double cursor_pos[2] = {
      input.cursor_position[0],
      input.cursor_position[1]
      };
  
    if((cursor_pos[0] > x_bounds[0] && cursor_pos[0] < x_bounds[1]) &&
      (cursor_pos[1] > y_bounds[0] && cursor_pos[1] < y_bounds[1])) {

        vec3 col = {0, 1, 0};

    glUniform3fv(
      glGetUniformLocation(self->material->shaderProgram->id, "u_color"),
      1,
      (float *)col);

    }
  }
#endif


  gsk_gl_vertex_array_bind(self->vao);
  glDrawArrays(GL_TRIANGLES, 0, 6);

}
