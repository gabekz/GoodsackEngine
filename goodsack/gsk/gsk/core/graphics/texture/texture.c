/*
 * Copyright (c) 2022-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#include "texture.h"

#include <stdarg.h>

#include "util/logger.h"
#include "util/sysdefs.h"

#include "core/device/device.h"
#include "core/drivers/vulkan/vulkan.h"

#define STB_IMAGE_IMPLEMENTATION
// TODO: Move to thirdparty directive - gkutuzov/GoodsackEngine#19
#include "stb_image.h"

#define TEXTURE_WRAPPING GL_REPEAT

gsk_Texture
_gsk_texture_create_internal(gsk_AssetBlob *p_asset_blob,
                             const char *path,
                             VulkanDeviceContext *vkDevice,
                             TextureOptions *p_options)
{
    gsk_Texture ret = {0};
    ret.id          = 0;

    unsigned char *localBuffer = NULL;

    stbi_set_flip_vertically_on_load(p_options->flip_vertically);

    if (p_asset_blob != NULL)
    {
        localBuffer = stbi_load_from_memory(p_asset_blob->p_buffer,
                                            p_asset_blob->buffer_len,
                                            &ret.width,
                                            &ret.height,
                                            &ret.bpp,
                                            STBI_rgb_alpha);
    } else if (path != NULL)
    {
        ret.filePath = path;
        localBuffer =
          stbi_load(path, &ret.width, &ret.height, &ret.bpp, STBI_rgb_alpha);
    } else
    {
        LOG_ERROR("Failed to create texture. Must pass either blob or path.");
        return ret;
    }

    if (localBuffer == NULL || localBuffer == 0x00)
    {
        LOG_ERROR("Failed to load texture data!");
        return ret;
    }

    if (GSK_DEVICE_API_OPENGL)
    {
        glGenTextures(1, &ret.id);
        glBindTexture(GL_TEXTURE_2D, ret.id);

        glTexImage2D(GL_TEXTURE_2D,
                     0,
                     p_options->internal_format,
                     ret.width,
                     ret.height,
                     0,
                     GL_RGBA,
                     GL_UNSIGNED_BYTE,
                     localBuffer);

        // Wrapping
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, TEXTURE_WRAPPING);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, TEXTURE_WRAPPING);

        // Mipmaps
        if (p_options->gen_mips > 0)
        {
            glGenerateTextureMipmap(ret.id);
            glTexParameteri(
              GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        } else
        {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        }

        // Anistropic Filtering
        if (p_options->af_range > 0)
        {
            glTexParameterf(
              GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY, p_options->af_range);
        }
    } // GSK_DEVICE_API_OPENGL

    else if (GSK_DEVICE_API_VULKAN && vkDevice)
    {
        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;

        VkDeviceSize imageSize = ret.width * ret.height * 4;

        vulkan_buffer_create(vkDevice->physicalDevice,
                             vkDevice->device,
                             imageSize,
                             VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                             VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                               VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                             &stagingBuffer,
                             &stagingBufferMemory);

        void *data;
        VK_CHECK(vkMapMemory(
          vkDevice->device, stagingBufferMemory, 0, imageSize, 0, &data));
        memcpy(data, localBuffer, (u32)imageSize);
        vkUnmapMemory(vkDevice->device, stagingBufferMemory);

        vulkan_image_create(vkDevice->physicalDevice,
                            vkDevice->device,
                            &ret.vulkan.textureImage,
                            &ret.vulkan.textureImageMemory,
                            ret.width,
                            ret.height,
                            VK_FORMAT_R8G8B8A8_SRGB,
                            VK_IMAGE_TILING_OPTIMAL,
                            VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                              VK_IMAGE_USAGE_SAMPLED_BIT,
                            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        vulkan_image_layout_transition(vkDevice->device,
                                       vkDevice->commandPool,
                                       vkDevice->graphicsQueue,
                                       ret.vulkan.textureImage,
                                       VK_FORMAT_R8G8B8A8_SRGB,
                                       VK_IMAGE_LAYOUT_UNDEFINED,
                                       VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

        vulkan_image_copy_from_buffer(vkDevice->device,
                                      vkDevice->commandPool,
                                      vkDevice->graphicsQueue,
                                      stagingBuffer,
                                      ret.vulkan.textureImage,
                                      (u32)ret.width,
                                      (u32)ret.height);

        // Final transition for shader access
        vulkan_image_layout_transition(
          vkDevice->device,
          vkDevice->commandPool,
          vkDevice->graphicsQueue,
          ret.vulkan.textureImage,
          VK_FORMAT_R8G8B8A8_SRGB,
          VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
          VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

        // Clean-up staging buffer
        vkDestroyBuffer(vkDevice->device, stagingBuffer, NULL);
        vkFreeMemory(vkDevice->device, stagingBufferMemory, NULL);

        // Create Texture ImageView
        ret.vulkan.textureImageView =
          vulkan_image_view_create(vkDevice->device,
                                   ret.vulkan.textureImage,
                                   VK_FORMAT_R8G8B8A8_SRGB,
                                   VK_IMAGE_ASPECT_COLOR_BIT);

        // Create Texture Sampler (for shader access)
        ret.vulkan.textureSampler = vulkan_image_texture_sampler(
          vkDevice->device, vkDevice->physicalDeviceProperties);

    } // GSK_DEVICE_API_VULKAN

    if (localBuffer) { stbi_image_free(localBuffer); }

    return ret;
}

gsk_Texture *
texture_create(const char *path,
               VulkanDeviceContext *vkDevice,
               TextureOptions options)
{
    gsk_Texture *tex = malloc(sizeof(gsk_Texture));
    if (tex == NULL)
    {
        LOG_CRITICAL("Failled to allocate memory for texture.");
    }

    *tex = _gsk_texture_create_internal(NULL, path, vkDevice, &options);

    return tex;
}

gsk_Texture *
texture_create_cubemap(u32 faceCount, ...)
{
    u32 textureId;
    glGenTextures(1, &textureId);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureId);

    gsk_Texture *tex = malloc(sizeof(gsk_Texture));
    tex->id          = textureId;

    va_list ap;
    va_start(ap, faceCount);
    va_end(ap);

    stbi_set_flip_vertically_on_load(FALSE);
    for (int i = 0; i < faceCount; i++)
    {
        unsigned char *data;
        const char *path = va_arg(ap, const char *);
        if (path != NULL)
        {
            data =
              stbi_load(path, &tex->width, &tex->height, &tex->bpp, /*RGBA*/ 0);

            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                         0,
                         GL_RGB,
                         tex->width,
                         tex->height,
                         0,
                         GL_RGB,
                         GL_UNSIGNED_BYTE,
                         data);
            stbi_image_free(data);
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return tex;
}

gsk_Texture *
texture_create_hdr(const char *path)
{
    LOG_DEBUG("Loading HDR Image at path: %s", path);

    gsk_Texture *tex = malloc(sizeof(gsk_Texture));

    stbi_set_flip_vertically_on_load(TRUE);
    float *data = stbi_loadf(path, &tex->width, &tex->height, &tex->bpp, 0);

    assert(data != NULL);

    u32 textureId;
    glGenTextures(1, &textureId);
    glBindTexture(GL_TEXTURE_2D, textureId);

    for (int i = 0; i < 1; i++)
    {
        glTexImage2D(GL_TEXTURE_2D,
                     0,
                     GL_RGB16F,
                     tex->width,
                     tex->height,
                     0,
                     GL_RGB,
                     GL_FLOAT,
                     data);
    }

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    tex->id       = textureId;
    tex->filePath = path;

    free(data);
    return tex;
}

void
texture_bind(gsk_Texture *self, u32 slot)
{
    self->activeSlot = slot;
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D, self->id);
}

void
texture_unbind()
{
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void
texture_cleanup(gsk_Texture *self, VulkanDeviceContext *vkDevice)
{
    if (GSK_DEVICE_API_VULKAN && vkDevice)
    {
        vkDestroyImage(vkDevice->device, self->vulkan.textureImage, NULL);
        vkFreeMemory(vkDevice->device, self->vulkan.textureImageMemory, NULL);
    }
}
