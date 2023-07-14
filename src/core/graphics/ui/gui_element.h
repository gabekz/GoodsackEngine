#ifndef H_GUI_ELEMENT_H
#define H_GUI_ELEMENT_H

#include <core/drivers/opengl/opengl.h>

#include <core/graphics/material/material.h>
#include <core/graphics/texture/texture.h>

#include <util/maths.h>

typedef struct GuiElement
{
    VAO *vao;
    Texture *texture;
    Material *material;

    vec2 position; // position in pixel-coordinates
    vec2 size;     // size in pixel-coordinates

    ui16 using_texture;

} GuiElement;

GuiElement *
gui_element_create(vec2 position, vec2 size, Texture *p_texture);

void
gui_element_draw(GuiElement *element);

#endif // H_GUI_ELEMENT_H
