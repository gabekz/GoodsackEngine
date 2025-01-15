/*
 * Copyright (c) 2024-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#ifndef __GUI_CANVAS_H__
#define __GUI_CANVAS_H__

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include "core/graphics/material/material.h"
#include "core/graphics/ui/gui_element.h"
#include "core/graphics/ui/gui_text.h"

typedef struct gsk_GuiCanvas
{

    gsk_GuiElement *elements[256];
    int elements_count;

    gsk_Material *p_material;
    vec2 canvas_size;

} gsk_GuiCanvas;

gsk_GuiCanvas
gsk_gui_canvas_create();

void
gsk_gui_canvas_add_element(gsk_GuiCanvas *p_self, gsk_GuiElement *p_element);

void
gsk_gui_canvas_add_text(gsk_GuiCanvas *p_self, gsk_GuiText *p_text);

void
gsk_gui_canvas_draw(gsk_GuiCanvas *p_self);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __GUI_CANVAS_H__
