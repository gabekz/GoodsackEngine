/*
 * Copyright (c) 2023-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#ifndef __GUI_TEXT_H__
#define __GUI_TEXT_H__

#include "asset/asset_font.h"

#include "core/graphics/texture/texture.h"
#include "core/graphics/ui/gui_element.h"

#include "util/sysdefs.h"

#define GUI_FONT_MAX_CHARS    256 // maximum characters allowed in a font bitmap
#define GUI_FONT_BASE_SPACING 17

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

typedef struct gsk_GuiText
{
    u32 character_count;
    char *str_text; // text of the string
    u32 max_chars;
    // gsk_GuiElement **elements; // individual character GUI elements
    gsk_GuiElement *p_element;
    gsk_Font *p_font;
    vec2 pos_offset;
} gsk_GuiText;

gsk_GuiText *
gsk_gui_text_create(const char *text_string,
                    GskGuiElementAnchorType anchor_type,
                    vec2 pos_offset,
                    vec3 color,
                    u32 max_chars);

void
gsk_gui_text_modify(gsk_GuiText *self, const char *text_string);

void
gsk_gui_text_draw(gsk_GuiText *self, u32 shader_id);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __GUI_TEXT_H__