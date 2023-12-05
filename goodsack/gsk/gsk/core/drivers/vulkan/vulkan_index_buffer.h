/*
 * Copyright (c) 2022-2023, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#ifndef __VULKAN_INDEX_BUFFER_H__
#define __VULKAN_INDEX_BUFFER_H__

#include "util/gfx.h"
#include "util/sysdefs.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

typedef struct VulkanIndexBuffer
{
    VkBuffer buffer;
    VkDeviceMemory bufferMemory;
    u16 indicesCount;
} VulkanIndexBuffer;

VulkanIndexBuffer *
vulkan_index_buffer_create(VkPhysicalDevice physicalDevice,
                           VkDevice device,
                           VkCommandPool commandPool,
                           VkQueue graphicsQueue,
                           u16 *indices,
                           u16 indicesCount);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __VULKAN_INDEX_BUFFER_H__
