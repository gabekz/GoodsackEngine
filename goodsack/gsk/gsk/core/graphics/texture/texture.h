/*
 * Copyright (c) 2022-2023, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#ifndef __TEXTURE_H__
#define __TEXTURE_H__

#include <stdlib.h>

#include "util/gfx.h"
#include "util/sysdefs.h"

#include "core/drivers/vulkan/vulkan.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

typedef struct TextureOptions
{
    float af_range;
    ui32 internal_format;
    ui16 gen_mips, flip_vertically; // bool
} TextureOptions;
// TextureOptions_default = {0, GL_RGB, false, 1};

typedef struct gsk_Texture
{
    const char *filePath;
    si32 bpp;
    si32 width, height;
    ui32 id;
    ui32 activeSlot;

    struct
    {
        VkImage textureImage;
        VkDeviceMemory textureImageMemory;

        VkImageView textureImageView;
        VkSampler textureSampler;
    } vulkan;
} gsk_Texture;

gsk_Texture *
texture_create(const char *path,
               VulkanDeviceContext *vkDevice,
               TextureOptions options);

gsk_Texture *
texture_create_cubemap(ui32 faceCount, ...);
gsk_Texture *
texture_create_hdr(const char *path);

void
texture_bind(gsk_Texture *self, ui32 slot);
void
texture_unbind();

void
texture_cleanup(gsk_Texture *self, VulkanDeviceContext *vkDevice);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __TEXTURE_H__