/*
 * Copyright (c) 2024, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#include "gui_canvas.h"

gsk_GuiCanvas
gsk_gui_canvas_create()
{
    return (gsk_GuiCanvas) {
      .elements_count = 0,
    };
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
    for (int i = 0; i < p_self->elements_count; i++)
    {
        gsk_gui_element_draw(p_self->elements[i]);
    }
}
