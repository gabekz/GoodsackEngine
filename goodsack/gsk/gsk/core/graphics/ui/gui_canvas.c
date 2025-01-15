/*
 * Copyright (c) 2024-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#include "gui_canvas.h"

#include "core/graphics/material/material.h"
#include "core/graphics/ui/gui_element.h"
#include "runtime/gsk_runtime_wrapper.h"

#define USING_BLENDING  TRUE
#define SCALE_TO_SCREEN TRUE

#define DEFAULT_CANVAS_WIDTH  1920
#define DEFAULT_CANVAS_HEIGHT 1080

gsk_GuiCanvas
gsk_gui_canvas_create()
{
    gsk_Material *p_mat = gsk_material_create(
      NULL, GSK_PATH("gsk://shaders/canvas2d.shader"), 0, NULL);

    vec2 viewport = {DEFAULT_CANVAS_WIDTH, DEFAULT_CANVAS_HEIGHT};

    gsk_GuiCanvas ret = {
      .elements       = NULL,
      .elements_count = 0,
      .p_material     = p_mat,
    };

    glm_vec2_copy(viewport, ret.canvas_size);

    return ret;
}

void
gsk_gui_canvas_add_element(gsk_GuiCanvas *p_self, gsk_GuiElement *p_element)
{
    p_self->elements[p_self->elements_count] = p_element;
    p_self->elements_count++;
}

void
gsk_gui_canvas_add_text(gsk_GuiCanvas *p_self, gsk_GuiText *p_text)
{
    for (int i = 0; i < p_text->character_count; i++)
    {
        p_self->elements[p_self->elements_count] = p_text->elements[i];
        p_self->elements_count++;
    }
}

void
gsk_gui_canvas_draw(gsk_GuiCanvas *p_self)
{
    const gsk_Renderer *p_renderer = gsk_runtime_get_renderer();
    const u32 shader_id            = p_self->p_material->shaderProgram->id;

#if USING_BLENDING
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
#endif

    gsk_material_use(p_self->p_material);

#if SCALE_TO_SCREEN
    vec2 viewport = {p_self->canvas_size[0], p_self->canvas_size[1]};
#else
    vec2 viewport = {p_renderer->windowWidth, p_renderer->windowHeight};
#endif

    glUniform2fv(
      glGetUniformLocation(shader_id, "u_viewport"), 1, (float *)viewport);

    for (int i = 0; i < p_self->elements_count; i++)
    {
        vec2 element_pos = {0, 0};

        switch (p_self->elements[i]->anchor_type)
        {
        case (GskGuiElementAnchorType_Center):
            element_pos[0] = viewport[0] / 2;
            element_pos[1] = viewport[1] / 2;
            break;
        case (GskGuiElementAnchorType_None):
        default: break;
        }

        glm_vec2_add(element_pos, p_self->elements[i]->position, element_pos);

        // send position to shader
        glUniform2fv(glGetUniformLocation(shader_id, "u_position"),
                     1,
                     (float *)element_pos);

        // continue draw
        gsk_gui_element_draw(p_self->elements[i], shader_id);
    }

#if USING_BLENDING
    glDisable(GL_BLEND);
#endif
}
