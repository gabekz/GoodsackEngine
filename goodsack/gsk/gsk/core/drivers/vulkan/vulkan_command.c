/*
 * Copyright (c) 2022-2023, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#include "vulkan_command.h"

#include <stdlib.h>

#include "core/drivers/vulkan/vulkan_device.h"
#include "core/drivers/vulkan/vulkan_support.h"

VkCommandPool
vulkan_command_pool_create(VkPhysicalDevice physicalDevice, VkDevice device)
{
    ui32 indices = vulkan_device_find_queue_families(physicalDevice);

    VkCommandPool commandPool;

    // Command Pool
    VkCommandPoolCreateInfo poolInfo = {
      .sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
      .flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
      .queueFamilyIndex = indices};

    VK_CHECK(vkCreateCommandPool(device, &poolInfo, NULL, &commandPool));

    return commandPool;
}

VkCommandBuffer *
vulkan_command_buffer_create(VkDevice device, VkCommandPool commandPool)
{
    VkCommandBuffer *commandBuffers =
      malloc(sizeof(VkCommandBuffer) * MAX_FRAMES_IN_FLIGHT);

    VkCommandBufferAllocateInfo allocInfo = {
      .sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
      .commandPool        = commandPool,
      .level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
      .commandBufferCount = MAX_FRAMES_IN_FLIGHT};

    VK_CHECK(vkAllocateCommandBuffers(device, &allocInfo, commandBuffers));

    return commandBuffers;
}

VkCommandBuffer
vulkan_command_stc_begin(VkDevice device, VkCommandPool commandPool)
{
    VkCommandBufferAllocateInfo allocInfo = {
      .sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
      .commandPool        = commandPool,
      .level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
      .commandBufferCount = 1};

    VkCommandBuffer commandBuffer;
    VK_CHECK(vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer));

    VkCommandBufferBeginInfo beginInfo = {
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
      .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT};

    VK_CHECK(vkBeginCommandBuffer(commandBuffer, &beginInfo));

    return commandBuffer;
}

void
vulkan_command_stc_end(VkDevice device,
                       VkQueue graphicsQueue,
                       VkCommandBuffer commandBuffer,
                       VkCommandPool commandPool)
{
    VK_CHECK(vkEndCommandBuffer(commandBuffer));

    VkSubmitInfo submitInfo = {.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
                               .commandBufferCount = 1,
                               .pCommandBuffers    = &commandBuffer};

    VK_CHECK(vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE));
    VK_CHECK(vkQueueWaitIdle(graphicsQueue));

    vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
}
