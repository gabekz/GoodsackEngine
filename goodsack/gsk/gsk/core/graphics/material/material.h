/*
 * Copyright (c) 2022-2023, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#ifndef __MATERIAL_H__
#define __MATERIAL_H__

#include "util/sysdefs.h"

#include "core/graphics/shader/shader.h"
#include "core/graphics/texture/texture.h"

typedef struct _material Material;

struct _material
{
    ShaderProgram *shaderProgram;
    Texture **textures;
    ui32 texturesCount;

    struct
    {
        VkPipelineLayout *pipelineLayout;
    } vulkan;
};

Material *
material_create(ShaderProgram *shader,
                const char *shaderPath,
                ui32 textureCount,
                ...);
void
material_use(Material *self);

void
material_add_texture(Material *self, Texture *texture);

#endif // __MATERIAL_H__