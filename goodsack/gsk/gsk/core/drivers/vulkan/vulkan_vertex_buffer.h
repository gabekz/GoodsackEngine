/*
 * Copyright (c) 2022-2023, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#ifndef __VULKAN_VERTEX_BUFFER_H__
#define __VULKAN_VERTEX_BUFFER_H__

#include "util/gfx.h"
#include "util/sysdefs.h"

#if __cplusplus
extern "C" {
#endif // __cplusplus

typedef struct VulkanVertexBuffer VulkanVertexBuffer;

struct VulkanVertexBuffer
{

    VkBuffer buffer;
    VkDeviceMemory bufferMemory;

    void *data;
    u32 size;
};

VulkanVertexBuffer *
vulkan_vertex_buffer_create(VkPhysicalDevice physicalDevice,
                            VkDevice device,
                            VkQueue graphicsQueue,
                            VkCommandPool commandPool,
                            void *data,
                            VkDeviceSize size);

void
vulkan_vertex_buffer_cleanup(VkDevice device, VkBuffer buffer);

VkVertexInputBindingDescription
vulkan_vertex_buffer_get_binding_description();
VkVertexInputAttributeDescription *
vulkan_vertex_buffer_get_attribute_descriptions();

#if __cplusplus
}
#endif // __cplusplus

#endif // __VULKAN_VERTEX_BUFFER_H__
