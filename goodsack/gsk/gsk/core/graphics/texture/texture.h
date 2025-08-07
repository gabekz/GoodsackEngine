/*
 * Copyright (c) 2022-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#ifndef __TEXTURE_H__
#define __TEXTURE_H__

#include <stdlib.h>

#include "util/gfx.h"
#include "util/sysdefs.h"

#include "core/drivers/vulkan/vulkan.h"

#include "asset/assetdefs.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

typedef struct TextureOptions
{
    float af_range;
    u32 internal_format;
    u8 gen_mips;
    u8 flip_vertically;
} TextureOptions;
// TextureOptions_default = {0, GL_RGB, false, 1};

typedef struct gsk_Texture
{
    u8 is_valid;
    const char *filePath;
    s32 bpp;
    s32 width, height;
    u32 id;
    u32 activeSlot;

    struct
    {
        VkImage textureImage;
        VkDeviceMemory textureImageMemory;

        VkImageView textureImageView;
        VkSampler textureSampler;
    } vulkan;
} gsk_Texture;

gsk_Texture
_gsk_texture_create_internal(gsk_AssetBlob *p_asset_blob,
                             const char *path,
                             VulkanDeviceContext *vkDevice,
                             TextureOptions *p_options);

gsk_Texture *
texture_create(const char *path,
               VulkanDeviceContext *vkDevice,
               TextureOptions options);

gsk_Texture *
texture_create_cubemap(u32 faceCount, ...);
gsk_Texture *
texture_create_hdr(const char *path);

void
texture_bind(gsk_Texture *self, u32 slot);
void
texture_unbind();

void
texture_cleanup(gsk_Texture *self, VulkanDeviceContext *vkDevice);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __TEXTURE_H__