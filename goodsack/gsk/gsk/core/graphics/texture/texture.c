/*
 * Copyright (c) 2022-2023, Gabriel Kutuzov
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

gsk_Texture *
texture_create(const char *path,
               VulkanDeviceContext *vkDevice,
               TextureOptions options)
{
    gsk_Texture *tex  = malloc(sizeof(gsk_Texture));
    tex->filePath = path;

    // TODO: create parameter
    stbi_set_flip_vertically_on_load(options.flip_vertically);
    unsigned char *localBuffer;
    if (path != NULL) {
        LOG_INFO("Loading Image at path: %s", path);
        // LOG_DEBUG("Format: %d, GenMips: %d, AFRange: %f",
        //         format, genMipMaps, afRange);

        localBuffer = stbi_load(path,
                                &tex->width,
                                &tex->height,
                                &tex->bpp,
                                /*Type*/ STBI_rgb_alpha);
    } else {
        localBuffer = NULL;
    }

    if (localBuffer == NULL || localBuffer == 0x00) {
        LOG_CRITICAL("Failed to load texture data!");
    }

    if (DEVICE_API_OPENGL) {
        glGenTextures(1, &tex->id);
        glBindTexture(GL_TEXTURE_2D, tex->id);

        glTexImage2D(GL_TEXTURE_2D,
                     0,
                     options.internal_format,
                     tex->width,
                     tex->height,
                     0,
                     GL_RGBA,
                     GL_UNSIGNED_BYTE,
                     localBuffer);

        // Wrapping
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, TEXTURE_WRAPPING);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, TEXTURE_WRAPPING);

        // Mipmaps
        if (options.gen_mips >= 0) {
            glGenerateTextureMipmap(tex->id);
            glTexParameteri(
              GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        } else {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        }

        // Anistropic Filtering
        if (options.af_range > 0) {
            glTexParameterf(
              GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY, options.af_range);
        }
    } // DEVICE_API_OPENGL

    else if (DEVICE_API_VULKAN && vkDevice) {
        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;

        VkDeviceSize imageSize = tex->width * tex->height * 4;

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
        memcpy(data, localBuffer, (ui32)imageSize);
        vkUnmapMemory(vkDevice->device, stagingBufferMemory);

        vulkan_image_create(vkDevice->physicalDevice,
                            vkDevice->device,
                            &tex->vulkan.textureImage,
                            &tex->vulkan.textureImageMemory,
                            tex->width,
                            tex->height,
                            VK_FORMAT_R8G8B8A8_SRGB,
                            VK_IMAGE_TILING_OPTIMAL,
                            VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                              VK_IMAGE_USAGE_SAMPLED_BIT,
                            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        vulkan_image_layout_transition(vkDevice->device,
                                       vkDevice->commandPool,
                                       vkDevice->graphicsQueue,
                                       tex->vulkan.textureImage,
                                       VK_FORMAT_R8G8B8A8_SRGB,
                                       VK_IMAGE_LAYOUT_UNDEFINED,
                                       VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

        vulkan_image_copy_from_buffer(vkDevice->device,
                                      vkDevice->commandPool,
                                      vkDevice->graphicsQueue,
                                      stagingBuffer,
                                      tex->vulkan.textureImage,
                                      (ui32)tex->width,
                                      (ui32)tex->height);

        // Final transition for shader access
        vulkan_image_layout_transition(
          vkDevice->device,
          vkDevice->commandPool,
          vkDevice->graphicsQueue,
          tex->vulkan.textureImage,
          VK_FORMAT_R8G8B8A8_SRGB,
          VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
          VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

        // Clean-up staging buffer
        vkDestroyBuffer(vkDevice->device, stagingBuffer, NULL);
        vkFreeMemory(vkDevice->device, stagingBufferMemory, NULL);

        // Create Texture ImageView
        tex->vulkan.textureImageView =
          vulkan_image_view_create(vkDevice->device,
                                   tex->vulkan.textureImage,
                                   VK_FORMAT_R8G8B8A8_SRGB,
                                   VK_IMAGE_ASPECT_COLOR_BIT);

        // Create Texture Sampler (for shader access)
        tex->vulkan.textureSampler = vulkan_image_texture_sampler(
          vkDevice->device, vkDevice->physicalDeviceProperties);

    } // DEVICE_API_VULKAN

    if (localBuffer) { stbi_image_free(localBuffer); }

    return tex;
}

gsk_Texture *
texture_create_cubemap(ui32 faceCount, ...)
{
    ui32 textureId;
    glGenTextures(1, &textureId);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureId);

    gsk_Texture *tex = malloc(sizeof(gsk_Texture));
    tex->id      = textureId;

    va_list ap;
    va_start(ap, faceCount);
    va_end(ap);

    stbi_set_flip_vertically_on_load(FALSE);
    for (int i = 0; i < faceCount; i++) {
        unsigned char *data;
        const char *path = va_arg(ap, const char *);
        if (path != NULL) {
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
    LOG_INFO("Loading HDR Image at path: %s", path);

    gsk_Texture *tex = malloc(sizeof(gsk_Texture));

    stbi_set_flip_vertically_on_load(TRUE);
    float *data = stbi_loadf(path, &tex->width, &tex->height, &tex->bpp, 0);

    assert(data != NULL);

    ui32 textureId;
    glGenTextures(1, &textureId);
    glBindTexture(GL_TEXTURE_2D, textureId);

    for (int i = 0; i < 1; i++) {
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
texture_bind(gsk_Texture *self, ui32 slot)
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
    if (DEVICE_API_VULKAN && vkDevice) {
        vkDestroyImage(vkDevice->device, self->vulkan.textureImage, NULL);
        vkFreeMemory(vkDevice->device, self->vulkan.textureImageMemory, NULL);
    }
}
