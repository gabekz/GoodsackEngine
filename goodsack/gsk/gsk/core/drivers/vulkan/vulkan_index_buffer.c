#include "vulkan_index_buffer.h"

#include <stdlib.h>
#include <string.h>

#include <core/drivers/vulkan/vulkan_buffer.h>
#include <util/gfx.h>

VulkanIndexBuffer *
vulkan_index_buffer_create(VkPhysicalDevice physicalDevice,
                           VkDevice device,
                           VkCommandPool commandPool,
                           VkQueue graphicsQueue,
                           ui16 *indices,
                           ui16 indicesCount)
{
    VkDeviceSize bufferSize = indicesCount * sizeof(ui16);

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

    void *data;
    vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, indices, bufferSize);
    vkUnmapMemory(device, stagingBufferMemory);

    VulkanIndexBuffer *ret = malloc(sizeof(VulkanIndexBuffer));

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
                       (ui32)bufferSize);

    vkDestroyBuffer(device, stagingBuffer, NULL);
    vkFreeMemory(device, stagingBufferMemory, NULL);

    ret->indicesCount = indicesCount;
    return ret;
}
