/*
 * Copyright (c) 2022-2023, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#ifndef __VULKAN_BUFFER_H__
#define __VULKAN_BUFFER_H__

#include "util/gfx.h"
#include "util/sysdefs.h"

// TODO: Possibly move to vulkan_memory
ui32
vulkan_memory_type(VkPhysicalDevice physicalDevice,
                   ui32 typeFilter,
                   VkMemoryPropertyFlags properties);

void
vulkan_buffer_create(VkPhysicalDevice physicalDevice,
                     VkDevice device,
                     VkDeviceSize size,
                     VkBufferUsageFlags usage,
                     VkMemoryPropertyFlags properties,
                     VkBuffer *buffer,
                     VkDeviceMemory *bufferMemory);

void
vulkan_buffer_copy(VkDevice device,
                   VkQueue graphicsQueue,
                   VkCommandPool commandPool,
                   VkBuffer srcBuffer,
                   VkBuffer dstBuffer,
                   VkDeviceSize size);

#endif // __VULKAN_BUFFER_H__
