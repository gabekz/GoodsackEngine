#ifndef H_BILLBOARD
#define H_BILLBOARD

#include <core/api/opengl/opengl.h>

#include <core/texture/texture.h>
#include <model/material.h>

#include <util/maths.h>

typedef struct Billboard2D
{
    VAO *vao;
    Texture *texture;
    Material *material;
} Billboard2D;

Billboard2D *
billboard_create(const char *texturePath, vec2 size);
void
billboard_draw(Billboard2D *billboard);

#endif // H_BILLBOARD
