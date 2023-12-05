/*
 * Copyright (c) 2023, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#ifndef __BILLBOARD_H__
#define __BILLBOARD_H__

#include <core/drivers/opengl/opengl.h>

#include <core/graphics/material/material.h>
#include <core/graphics/texture/texture.h>

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
billboard_draw(Billboard2D *billboard, vec3 position);

#endif // __BILLBOARD_H__