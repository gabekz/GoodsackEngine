/*
 * Copyright (c) 2023, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#ifndef __GUI_ELEMENT_H__
#define __GUI_ELEMENT_H__

#include "util/maths.h"

#include "core/drivers/opengl/opengl.h"

#include "core/graphics/material/material.h"
#include "core/graphics/texture/texture.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

typedef struct gsk_GuiElement
{
    gsk_GlVertexArray *vao;
    gsk_Texture *texture;
    gsk_Material *material;

    vec2 position; // position in pixel-coordinates
    vec2 size;     // size in pixel-coordinates

    u16 using_texture;

} gsk_GuiElement;

gsk_GuiElement *
gsk_gui_element_create(vec2 position,
                       vec2 size,
                       gsk_Texture *p_texture,
                       vec4 tex_coords);

void
gsk_gui_element_draw(gsk_GuiElement *element);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __GUI_ELEMENT_H__
