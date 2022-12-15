#include "vulkan_buffer.h"
#include <core/api/vulkan/vulkan_support.h>

#include <string.h>

#include <util/gfx.h>
#include <util/logger.h>

static ui32 _findMemoryType(VkPhysicalDevice physicalDevice, 
        ui32 typeFilter, VkMemoryPropertyFlags properties) {

    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

    for(ui32 i = 0; i < memProperties.memoryTypeCount; i++) {
        if((typeFilter & (1 << i)) && 
                (memProperties.memoryTypes[i].propertyFlags & properties) ==
                properties) {
            return i;
        }
    }
    LOG_ERROR("Failed to find suitable memory type!");
    return 0;
}


void vulkan_buffer_create(VkPhysicalDevice physicalDevice,
                          VkDevice device, VkDeviceSize size,
                          VkBufferUsageFlags usage,
                          VkMemoryPropertyFlags properties,
                          VkBuffer *buffer,
                          VkDeviceMemory *bufferMemory) {
    VkBufferCreateInfo bufferInfo = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = size,
        .usage = usage,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE
    };

    VK_CHECK(vkCreateBuffer(device, &bufferInfo, NULL, buffer));

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(device, *buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = memRequirements.size,
        .memoryTypeIndex = _findMemoryType(physicalDevice,
                memRequirements.memoryTypeBits, 
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | 
                VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)
    };

    VK_CHECK(vkAllocateMemory(device, &allocInfo, NULL,
                bufferMemory));

    VK_CHECK(vkBindBufferMemory(device, *buffer,
                *bufferMemory, 0));
}

void vulkan_buffer_copy(VkDevice device, VkQueue graphicsQueue,
                        VkCommandPool commandPool,
                        VkBuffer srcBuffer,
                        VkBuffer dstBuffer,
                        VkDeviceSize size)
{
    VkCommandBufferAllocateInfo allocInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandPool = commandPool,
        .commandBufferCount = 1
    };

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
    };
    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    VkBufferCopy copyRegion = {
        .srcOffset = 0, // optional
        .dstOffset = 0, // optional
        .size = size
    };
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .commandBufferCount = 1,
        .pCommandBuffers = &commandBuffer
    };

    vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(graphicsQueue);

    vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
}
