/*
 * Copyright (c) 2022-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#include "vulkan_index_buffer.h"

#include <stdlib.h>
#include <string.h>

#include "util/gfx.h"

#include "core/drivers/vulkan/vulkan_buffer.h"
#include "core/drivers/vulkan/vulkan_support.h"

VulkanIndexBuffer *
vulkan_index_buffer_create(VkPhysicalDevice physicalDevice,
                           VkDevice device,
                           VkQueue graphicsQueue,
                           VkCommandPool commandPool,
                           void *data,
                           u16 indicesCount)
{
    VulkanIndexBuffer *ret = malloc(sizeof(VulkanIndexBuffer));

    VkDeviceSize bufferSize = indicesCount * sizeof(u32);

    ret->data = data;
    ret->size = bufferSize;
    void *pData;

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;

    vulkan_buffer_create(physicalDevice,
                         device,
                         bufferSize,
                         VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                           VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                         &stagingBuffer,
                         &stagingBufferMemory);

    VK_CHECK(
      vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &pData));
    memcpy(pData, ret->data, bufferSize);
    vkUnmapMemory(device, stagingBufferMemory);

    vulkan_buffer_create(physicalDevice,
                         device,
                         bufferSize,
                         VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                           VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                         VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                         &ret->buffer,
                         &ret->bufferMemory);

    vulkan_buffer_copy(device,
                       graphicsQueue,
                       commandPool,
                       stagingBuffer,
                       ret->buffer,
                       bufferSize);

    vkDestroyBuffer(device, stagingBuffer, NULL);
    vkFreeMemory(device, stagingBufferMemory, NULL);

    ret->indicesCount = indicesCount;
    return ret;
}
