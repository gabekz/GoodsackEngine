/*
 * Copyright (c) 2022-2023, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#include "material.h"

#include <stdarg.h>
#include <stdio.h>

#include "util/gfx.h"
#include "util/logger.h"

#include "core/device/device.h"
#include "core/graphics/shader/shader.h"
#include "core/graphics/texture/texture.h"

gsk_Material *
gsk_material_create(gsk_ShaderProgram *shader,
                    const char *shaderPath,
                    u32 textureCount,
                    ...)
{

    gsk_Material *ret = malloc(sizeof(gsk_Material));
    if (shader)
    {
        ret->shaderProgram = shader;
    } else if (shaderPath != "" || shaderPath != NULL)
    {
        ret->shaderProgram = gsk_shader_program_create(shaderPath);
    } else
    {
        LOG_ERROR(
          "You need to pass either a gsk_ShaderProgram or a valid path");
        return NULL;
    }

    // ret->textures = textures;
    ret->texturesCount = textureCount;

    if (textureCount <= 0)
    {
        ret->texturesCount = 0;
        return ret;
    }

    va_list ap;
    va_start(ap, textureCount);
    gsk_Texture **textures = malloc(textureCount * sizeof(gsk_Texture *));
    for (int i = 0; i < textureCount; i++)
    {
        *(textures + i) = va_arg(ap, gsk_Texture *);
    }
    ret->textures      = textures;
    ret->texturesCount = textureCount;
    va_end(ap);
    return ret;

    // TODO: Create descriptor set
}

void
gsk_material_use(gsk_Material *self)
{
    if (GSK_DEVICE_API_OPENGL)
    {
        if (self->texturesCount > 0)
        {
            for (int i = 0; i < self->texturesCount; i++)
            {
                texture_bind(self->textures[i], i);
            }
        }
        gsk_shader_use(self->shaderProgram);
    } else if (GSK_DEVICE_API_VULKAN)
    {
        LOG_DEBUG("Material not implemented for Vulkan");

        // Bind Pipeline here? Probably.
        // TODO: Bind image descriptor set HERE
    }
}

void
gsk_material_add_texture(gsk_Material *self, gsk_Texture *texture)
{
    if (self->texturesCount == 0)
    {
        self->textures      = malloc(sizeof(gsk_Texture *));
        self->textures[0]   = texture;
        self->texturesCount = 1;
        return;
    }
    self->texturesCount = self->texturesCount += 1;
    self->textures =
      realloc(self->textures, self->texturesCount * sizeof(gsk_Texture *));
    self->textures[self->texturesCount - 1] = texture;
}
