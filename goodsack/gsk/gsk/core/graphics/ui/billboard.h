/*
 * Copyright (c) 2023, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#ifndef __BILLBOARD_H__
#define __BILLBOARD_H__

#include "util/maths.h"

#include "core/drivers/opengl/opengl.h"
#include "core/graphics/material/material.h"
#include "core/graphics/texture/texture.h"

typedef struct gsk_Billboard2D
{
    VAO *vao;
    gsk_Texture *texture;
    gsk_Material *material;
} gsk_Billboard2D;

gsk_Billboard2D *
gsk_billboard_create(const char *texturePath, vec2 size);
void
gsk_billboard_draw(gsk_Billboard2D *billboard, vec3 position);

#endif // __BILLBOARD_H__