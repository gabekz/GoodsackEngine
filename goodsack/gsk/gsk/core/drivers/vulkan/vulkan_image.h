/*
 * Copyright (c) 2022-2023, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#ifndef __VULKAN_IMAGE_H__
#define __VULKAN_IMAGE_H__

#include "util/gfx.h"
#include "util/sysdefs.h"

#include "core/drivers/vulkan/vulkan_support.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

// TODO: Move to vulkan_texture.*
// Create and Allocate a Vulkan VkImage Texture.
void
vulkan_image_create(VkPhysicalDevice physicalDevice,
                    VkDevice device,
                    VkImage *image,
                    VkDeviceMemory *imageMemory,
                    u32 width,
                    u32 height,
                    VkFormat format,
                    VkImageTiling tiling,
                    VkImageUsageFlags usage,
                    VkMemoryPropertyFlags properties);

void
vulkan_image_layout_transition(VkDevice device,
                               VkCommandPool commandPool,
                               VkQueue graphicsQueue,
                               VkImage image,
                               VkFormat format,
                               VkImageLayout prevLayout,
                               VkImageLayout newLayout);

void
vulkan_image_copy_from_buffer(VkDevice device,
                              VkCommandPool commandPool,
                              VkQueue graphicsQueue,
                              VkBuffer buffer,
                              VkImage image,
                              u32 width,
                              u32 height);

// ImageViews

VkImageView
vulkan_image_view_create(VkDevice device,
                         VkImage textureImage,
                         VkFormat format,
                         VkImageAspectFlags aspectFlags);

VkSampler
vulkan_image_texture_sampler(VkDevice device,
                             VkPhysicalDeviceProperties deviceProperties);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // H_VULKAN_IMAGE
