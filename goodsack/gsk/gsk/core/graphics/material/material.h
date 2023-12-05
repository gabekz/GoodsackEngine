/*
 * Copyright (c) 2022-2023, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#ifndef __MATERIAL_H__
#define __MATERIAL_H__

#include "util/sysdefs.h"

#include "core/graphics/shader/shader.h"
#include "core/graphics/texture/texture.h"


typedef struct gsk_Material
{
    gsk_ShaderProgram *shaderProgram;
    gsk_Texture **textures;
    u32 texturesCount;

    struct
    {
        VkPipelineLayout *pipelineLayout;
    } vulkan;
} gsk_Material;

gsk_Material *
gsk_material_create(gsk_ShaderProgram *shader,
                const char *shaderPath,
                u32 textureCount,
                ...);
void
gsk_material_use(gsk_Material *self);

void
gsk_material_add_texture(gsk_Material *self, gsk_Texture *texture);

#endif // __MATERIAL_H__