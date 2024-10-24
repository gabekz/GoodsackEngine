/*
 * Copyright (c) 2022-2023, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#include "vulkan_vertex_buffer.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util/logger.h"

#include "core/drivers/vulkan/vulkan_buffer.h"
#include "core/drivers/vulkan/vulkan_support.h"

/*
#include <util/maths.h>

typedef struct Vertex {
    vec2 pos;
    vec3 color;
    vec2 texCoord;
} Vertex;
*/

VkVertexInputBindingDescription
vulkan_vertex_buffer_get_binding_description()
{
    VkVertexInputBindingDescription bindingDescription = {
      .binding = 0,
      .stride =
        ((sizeof(float) * 3) + (sizeof(float) * 2) + (sizeof(float) * 3)),
      .inputRate = VK_VERTEX_INPUT_RATE_VERTEX};
    return bindingDescription;
}

VkVertexInputAttributeDescription *
vulkan_vertex_buffer_get_attribute_descriptions()
{
    VkVertexInputAttributeDescription *attributeDescriptions =
      malloc(sizeof(VkVertexInputAttributeDescription) * 3);

    // Position
    attributeDescriptions[0] =
      (VkVertexInputAttributeDescription) {.binding  = 0,
                                           .location = 0,
                                           .format = VK_FORMAT_R32G32B32_SFLOAT,
                                           .offset = 0};
    // UV
    attributeDescriptions[1] =
      (VkVertexInputAttributeDescription) {.binding  = 0,
                                           .location = 1,
                                           .format = VK_FORMAT_R32G32B32_SFLOAT,
                                           .offset = 3 * sizeof(float)};
    // Normal
    attributeDescriptions[2] =
      (VkVertexInputAttributeDescription) {.binding  = 0,
                                           .location = 2,
                                           .format = VK_FORMAT_R32G32B32_SFLOAT,
                                           .offset = 5 * sizeof(float)};

    return attributeDescriptions;
}

VulkanVertexBuffer *
vulkan_vertex_buffer_create(VkPhysicalDevice physicalDevice,
                            VkDevice device,
                            VkQueue graphicsQueue,
                            VkCommandPool commandPool,
                            void *data,
                            VkDeviceSize size)
{

    VulkanVertexBuffer *vertexBuffer = malloc(sizeof(VulkanVertexBuffer));

    vertexBuffer->data = data;
    vertexBuffer->size = size;
    void *pData;

    // Staging Buffer
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    vulkan_buffer_create(physicalDevice,
                         device,
                         size,
                         VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                           VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                         &stagingBuffer,
                         &stagingBufferMemory);

    VK_CHECK(vkMapMemory(device, stagingBufferMemory, 0, size, 0, &pData));
    memcpy(pData, vertexBuffer->data, size);
    vkUnmapMemory(device, stagingBufferMemory);

    vulkan_buffer_create(physicalDevice,
                         device,
                         size,
                         VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                           VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                         VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                         &vertexBuffer->buffer,
                         &vertexBuffer->bufferMemory);

    vulkan_buffer_copy(device,
                       graphicsQueue,
                       commandPool,
                       stagingBuffer,
                       vertexBuffer->buffer,
                       size);

    vkDestroyBuffer(device, stagingBuffer, NULL);
    vkFreeMemory(device, stagingBufferMemory, NULL);

    // VulkanVertexBuffer *vertexBuffer = malloc(sizeof(VulkanVertexBuffer));

    return vertexBuffer;
}

void
vulkan_vertex_buffer_cleanup(VkDevice device, VkBuffer buffer)
{
    // TODO:
    vkDestroyBuffer(device, buffer, NULL);
    // vkFreeMemory(device, vertexBufferMemory, NULL);
}
