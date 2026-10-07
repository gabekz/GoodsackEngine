/*
 * Copyright (c) 2023-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#ifndef __GUI_ELEMENT_H__
#define __GUI_ELEMENT_H__

#include "util/maths.h"
#include "util/sysdefs.h"

#include "core/drivers/opengl/opengl.h"

#include "core/graphics/material/material.h"
#include "core/graphics/texture/texture.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

typedef enum GskGuiElementAnchorType_ {
    GskGuiElementAnchorType_None = 0,
    GskGuiElementAnchorType_Center,
    GskGuiElementAnchorType_Top,
    GskGuiElementAnchorType_TopLeft,
} GskGuiElementAnchorType_;

typedef s32 GskGuiElementAnchorType;

typedef struct gsk_GuiElement
{
    gsk_GlVertexArray *vao;
    gsk_GlVertexBuffer *vbo;
    gsk_Texture *texture;
    gsk_Material *material;

    vec3 color_rgb; // element pixel color
    vec2 offset;    // offset in pixel-coordinates
    vec2 position;  // offset position in pixel-coordinates with viewport
    vec2 size;      // size in pixel-coordinates
    vec2 canvas_viewport;

    ArrayList list_elements;
    u32 max_elements;
    u32 num_vertices;

    GskGuiElementAnchorType anchor_type;
    u16 using_texture;

    u8 needs_refresh;
} gsk_GuiElement;

void
gsk_gui_element_push_data(ArrayList *p_element_list,
                          vec2 size,
                          vec2 pos_offset,
                          vec4 tex_coords);

gsk_GuiElement *
gsk_gui_element_allocate(u32 max_elements);

void
gsk_gui_element_set_properties(gsk_GuiElement *p_self,
                               GskGuiElementAnchorType anchor,
                               vec2 position,
                               vec2 size,
                               vec3 color,
                               gsk_Texture *p_texture,
                               vec4 tex_coords);

gsk_GuiElement *
gsk_gui_element_create(GskGuiElementAnchorType anchor,
                       vec2 position,
                       vec2 size,
                       vec3 color,
                       gsk_Texture *p_texture,
                       vec4 tex_coords);

void
gsk_gui_element_draw(gsk_GuiElement *element, u32 shader_id);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __GUI_ELEMENT_H__
