/*
 * Copyright (c) 2022-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#ifndef __VULKAN_DEVICE_H__
#define __VULKAN_DEVICE_H__

#include "util/gfx.h"
#include "util/sysdefs.h"

#include "core/drivers/vulkan/vulkan_depth.h"
#include "core/drivers/vulkan/vulkan_index_buffer.h"
#include "core/drivers/vulkan/vulkan_pipeline.h"
#include "core/drivers/vulkan/vulkan_swapchain.h"
#include "core/drivers/vulkan/vulkan_vertex_buffer.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

typedef struct _vulkanDeviceContext VulkanDeviceContext;

struct _vulkanDeviceContext
{
    VkPhysicalDevice physicalDevice;
    VkPhysicalDeviceProperties physicalDeviceProperties;

    VkDevice device;
    VkSurfaceKHR surface;
    VkInstance vulkanInstance;

    VkDebugUtilsMessengerEXT debugMessenger;

    VkQueue graphicsQueue;
    // VkQueue presentQueue;
    u32 graphicsFamily;

    // TEMPORARY FOR RENDERING TESTS
    u32 presentImageIndex;

    VulkanSwapChainDetails *swapChainDetails;
    VulkanPipelineDetails *pipelineDetails;

    // Temporary vertex buffer for testing
    VulkanVertexBuffer *vertexBuffer;
    VulkanIndexBuffer indexBuffer;

    // UBO
    VkBuffer *uniformBuffers;
    VkDeviceMemory *uniformBuffersMemory;
    void **uniformBuffersMapped;

    VkCommandPool commandPool;
    VkCommandBuffer *commandBuffers;

    VkDescriptorPool descriptorPool;
    VkDescriptorSet *descriptorSets;

    u32 currentFrame;
    VkSemaphore *imageAvailableSemaphores;
    VkSemaphore *renderFinishedSemaphores;
    VkFence *inFlightFences;

    // Depth Buffer information
    VulkanDepthResources *depthResources;
};

VulkanDeviceContext *
vulkan_device_create();
void
vulkan_device_cleanup(VulkanDeviceContext *context);

u32
vulkan_device_find_queue_families(VkPhysicalDevice physicalDevice);

void
vulkan_context_create_sync(VulkanDeviceContext *context);

#if 0
static inline void init_vulkan() {
    createInstance();
    setupDebugMessenger();
    createSurface();
    pickPhysicalDevice();
    createLogicalDevice();
    createSwapChain();
}
#endif

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __VULKAN_DEVICE_H__
