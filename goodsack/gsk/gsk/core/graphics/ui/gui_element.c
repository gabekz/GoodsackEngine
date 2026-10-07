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

#include "runtime/gsk_runtime_wrapper.h"

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

#define _DEFAULT_VERTEX_COUNT 24
#define _DEFAULT_NUM_VERTICES 6

void
gsk_gui_element_push_data(ArrayList *p_element_list, vec2 size, vec2 pos_offset, vec4 tex_coords)
{
    float size_x_off = (size[0] / 2);
    float size_y_off = (size[1] / 2);

    float pos_x = pos_offset[0];
    float pos_y = pos_offset[1];

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

    float *rectangle = 
    (float[])
    {
         // Triangle 1
         size_x_off + pos_x, -size_y_off + pos_y, t_pos_x, t_neg_y,
        -size_x_off + pos_x, -size_y_off + pos_y, t_neg_x, t_neg_y,
        -size_x_off + pos_x,  size_y_off + pos_y, t_neg_x, t_pos_y,

         // Triangle 2
         size_x_off + pos_x,  size_y_off + pos_y, t_pos_x, t_pos_y,
         size_x_off + pos_x, -size_y_off + pos_y, t_pos_x, t_neg_y,
        -size_x_off + pos_x,  size_y_off + pos_y, t_neg_x, t_pos_y,
    };
    LIST_PUSH(p_element_list, rectangle);
}

gsk_GuiElement *
gsk_gui_element_allocate(u32 max_elements)
{
    gsk_GuiElement *ret = malloc(sizeof(gsk_GuiElement));
    ret->list_elements = LIST_INIT(sizeof(f32) * _DEFAULT_VERTEX_COUNT, 1); // TODO: change increment 

    ret->num_vertices = _DEFAULT_NUM_VERTICES;
    ret->max_elements = max_elements;
    ret->needs_refresh = TRUE;

    if(GSK_DEVICE_API_OPENGL)
    {
      ret->vao = gsk_gl_vertex_array_create();
      ret->vbo = gsk_gl_vertex_buffer_create(NULL, ret->max_elements * _DEFAULT_VERTEX_COUNT * sizeof(float), GskOglUsageType_Dynamic);
      gsk_gl_vertex_buffer_bind(ret->vbo);
      gsk_gl_vertex_buffer_push(ret->vbo, 2, GL_FLOAT, GL_FALSE);
      gsk_gl_vertex_buffer_push(ret->vbo, 2, GL_FLOAT, GL_FALSE);
      gsk_gl_vertex_array_add_buffer(ret->vao, ret->vbo);

      //glBufferData(GL_ARRAY_BUFFER, sizeof(f32) * _DEFAULT_VERTEX_COUNT * ret->list_elements.list_next, NULL, GL_DYNAMIC_DRAW);

    }
    else if(GSK_DEVICE_API_VULKAN) {
      ret->vao = NULL;
    }

    return ret;
}

void
gsk_gui_element_set_properties(gsk_GuiElement *p_self, GskGuiElementAnchorType anchor, vec2 position, vec2 size, vec3 color, gsk_Texture *p_texture, vec4 tex_coords)
{
    glm_vec2_copy(position, p_self->position);
    glm_vec2_copy(size, p_self->size);
    glm_vec3_copy(color, p_self->color_rgb);
    glm_vec2_zero(p_self->offset);
    p_self->anchor_type = anchor;

    // TODO: possible set a refresh just in case. Well, probably only for modifying vertex data
    //p_self->needs_refresh = TRUE;

    p_self->using_texture = (p_texture != NULL) ? TRUE : FALSE;
    p_self->material = NULL;

    if(p_self->using_texture)
    {
        p_self->texture = p_texture;
        if(p_self->material) {
          gsk_material_add_texture(p_self->material, p_self->texture);
        }
    }
}

gsk_GuiElement *
gsk_gui_element_create(GskGuiElementAnchorType anchor, vec2 position, vec2 size, vec3 color, gsk_Texture *p_texture, vec4 tex_coords)
{
    gsk_GuiElement *ret = gsk_gui_element_allocate(1); // TODO: be more clear with 1
    gsk_gui_element_set_properties(ret, anchor, position, size, color, p_texture, tex_coords);
    gsk_gui_element_push_data(&ret->list_elements, size, (vec2){0, 0}, tex_coords);

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

  // send texture info
  glUniform1i(
    glGetUniformLocation(shader_id, "u_using_texture"),
    self->using_texture);

  // send color info
  glUniform3fv(
    glGetUniformLocation(shader_id, "u_color"),
    1,
    (float *)self->color_rgb);

#if 1
// TODO: only possible to select one gui button at a time
// TODO: change to 0.0-1.0 range instead of pixel position
    if(TRUE)
    {
        gsk_Renderer *p_renderer = gsk_runtime_get_renderer();

        // TODO: replace 1920x1080 with canvas viewport
        vec2 window_scale = {(f32)p_renderer->windowWidth / self->canvas_viewport[0],
                             (f32)p_renderer->windowHeight / self->canvas_viewport[1]};

        vec2 pos = GLM_VEC2_ZERO_INIT;
        vec2 offs = {self->offset[0],
                     self->offset[1]};

        glm_vec2_add(self->position, offs, pos);

    // TODO: division may not be correct here
    double x_bounds[2] = {pos[0] - self->size[0] / 2.0f,
                          pos[0] + self->size[0] / 2.0f};
  
    // TODO: division may not be correct here
    double y_bounds[2] = {pos[1] - self->size[1] / 2.0f,
                          pos[1] + self->size[1] / 2.0f};

    const gsk_Input input = gsk_device_getInput();
    const double cursor_pos[2] = {
        input.cursor_position[0] / window_scale[0],
        (p_renderer->windowHeight - input.cursor_position[1]) / window_scale[1]
    };

    if((cursor_pos[0] > x_bounds[0] && cursor_pos[0] < x_bounds[1]) &&
      (cursor_pos[1] > y_bounds[0] && cursor_pos[1] < y_bounds[1]))
    {

      // TODO: handle button enter

#if 1
    vec3 col = {0, 1, 0};
    glUniform3fv(
      glGetUniformLocation(shader_id, "u_color"),
      1,
      (float *)col);
#endif

      // TODO: handle button click

    }

    else {
      // TODO: handle button leave
    }
  }
#endif


  if(GSK_DEVICE_API_OPENGL)
  {
    gsk_gl_vertex_array_bind(self->vao);

    if(self->needs_refresh == TRUE)
    {
      gsk_gl_vertex_buffer_bind(self->vbo);
      glBufferData(GL_ARRAY_BUFFER, sizeof(f32) * _DEFAULT_VERTEX_COUNT * self->list_elements.list_next, NULL, GL_DYNAMIC_DRAW);
      glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(f32) * _DEFAULT_VERTEX_COUNT * self->list_elements.list_next, self->list_elements.data.buffer);

      self->needs_refresh = FALSE;
    }

    glDrawArrays(GL_TRIANGLES, 0, self->num_vertices * self->list_elements.list_next);
  }
}
