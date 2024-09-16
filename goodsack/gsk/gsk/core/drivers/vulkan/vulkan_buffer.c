/*
 * Copyright (c) 2022-2023, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#include "vulkan_buffer.h"

#include <string.h>

#include "util/gfx.h"
#include "util/logger.h"

#include "core/drivers/vulkan/vulkan_command.h"
#include "core/drivers/vulkan/vulkan_support.h"

u32
vulkan_memory_type(VkPhysicalDevice physicalDevice,
                   u32 typeFilter,
                   VkMemoryPropertyFlags properties)
{

    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

    for (u32 i = 0; i < memProperties.memoryTypeCount; i++)
    {
        if ((typeFilter & (1 << i)) &&
            (memProperties.memoryTypes[i].propertyFlags & properties) ==
              properties)
        {
            return i;
        }
    }
    LOG_ERROR("Failed to find suitable memory type!");
    return 0;
}

void
vulkan_buffer_create(VkPhysicalDevice physicalDevice,
                     VkDevice device,
                     VkDeviceSize size,
                     VkBufferUsageFlags usage,
                     VkMemoryPropertyFlags properties,
                     VkBuffer *buffer,
                     VkDeviceMemory *bufferMemory)
{
    VkBufferCreateInfo bufferInfo = {.sType =
                                       VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
                                     .size        = size,
                                     .usage       = usage,
                                     .sharingMode = VK_SHARING_MODE_EXCLUSIVE};

    VK_CHECK(vkCreateBuffer(device, &bufferInfo, NULL, buffer));

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(device, *buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo = {
      .sType          = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
      .allocationSize = memRequirements.size,
      .memoryTypeIndex =
        vulkan_memory_type(physicalDevice,
                           memRequirements.memoryTypeBits,
                           VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                             VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)};

    VK_CHECK(vkAllocateMemory(device, &allocInfo, NULL, bufferMemory));

    VK_CHECK(vkBindBufferMemory(device, *buffer, *bufferMemory, 0));
}

void
vulkan_buffer_copy(VkDevice device,
                   VkQueue graphicsQueue,
                   VkCommandPool commandPool,
                   VkBuffer srcBuffer,
                   VkBuffer dstBuffer,
                   VkDeviceSize size)
{
    VkCommandBuffer commandBuffer =
      vulkan_command_stc_begin(device, commandPool);

    VkBufferCopy copyRegion = {.srcOffset = 0, // optional
                               .dstOffset = 0, // optional
                               .size      = size};

    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

    vulkan_command_stc_end(device, graphicsQueue, commandBuffer, commandPool);
}
