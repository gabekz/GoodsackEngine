#ifndef H_GUI_TEXT_H
#define H_GUI_TEXT_H

#include <core/graphics/texture/texture.h>
#include <core/graphics/ui/gui_element.h>

#define GUI_FONT_MAX_CHARS    256 // maximum characters allowed in a font bitmap
#define GUI_FONT_BASE_SPACING 17

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

typedef struct GuiText
{
    ui32 character_count;
    const char *text;      // text of the string
    GuiElement **elements; // individual character GUI elements
    Texture *font_atlas;   // pointer to the font atlas texture

    char char_spacing[GUI_FONT_MAX_CHARS]; // character effective-widths

} GuiText;

GuiText *
gui_text_create(const char *text_string);

void
gui_text_draw(GuiText *self);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // H_GUI_TEXT_H