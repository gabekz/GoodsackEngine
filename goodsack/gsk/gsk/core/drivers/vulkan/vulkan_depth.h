/*
 * Copyright (c) 2022-2023, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#ifndef __VULKAN_DEPTH_H__
#define __VULKAN_DEPTH_H__

#include "util/gfx.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

typedef struct VulkanDepthResources
{
    VkImage depthImage;
    VkDeviceMemory depthImageMemory;
    VkImageView depthImageView;
} VulkanDepthResources;

VkFormat
vulkan_depth_find_format(VkPhysicalDevice physicalDevice);

VulkanDepthResources *
vulkan_depth_create_resources(VkPhysicalDevice physicalDevice,
                              VkDevice device,
                              VkExtent2D swapChainExtent);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __VULKAN_DEPTH_H__