#ifndef H_UI_IMAGE_H
#define H_UI_IMAGE_H

#include <core/drivers/opengl/opengl.h>

#include <core/graphics/material/material.h>
#include <core/graphics/texture/texture.h>

#include <util/maths.h>

typedef struct Image2D
{
    VAO *vao;
    Texture *texture;
    Material *material;
} Image2D;

Image2D *
ui_image_create(const char *texture_path, vec2 position);
void
ui_image_draw(Image2D *image);

#endif // H_UI_IMAGE_H
