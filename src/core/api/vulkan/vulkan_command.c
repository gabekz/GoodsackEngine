#include "vulkan_command.h"

#include <core/api/vulkan/vulkan_support.h>

VkCommandBuffer vulkan_command_stc_begin(VkDevice device,
        VkCommandPool commandPool) {

    VkCommandBufferAllocateInfo allocInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = commandPool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1
    };

    VkCommandBuffer commandBuffer;
    VK_CHECK(vkAllocateCommandBuffers(
            device, &allocInfo, &commandBuffer));

    VkCommandBufferBeginInfo beginInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
    };
    
    VK_CHECK(vkBeginCommandBuffer(commandBuffer, &beginInfo)); 

    return commandBuffer;
}

void vulkan_command_stc_end(VkDevice device, VkQueue graphicsQueue,
        VkCommandBuffer commandBuffer, VkCommandPool commandPool)
{
    VK_CHECK(vkEndCommandBuffer(commandBuffer));

    VkSubmitInfo submitInfo = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .commandBufferCount = 1,
        .pCommandBuffers = &commandBuffer
    };

    VK_CHECK(vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE));
    VK_CHECK(vkQueueWaitIdle(graphicsQueue));

    vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
}
