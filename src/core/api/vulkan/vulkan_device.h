#ifndef H_VULKAN_DEVICE
#define H_VULKAN_DEVICE

#include <util/gfx.h>
#include <util/sysdefs.h>

#include <core/api/vulkan/vulkan_swapchain.h>
#include <core/api/vulkan/vulkan_pipeline.h>
#include <core/api/vulkan/vulkan_vertex_buffer.h>

#define MAX_FRAMES_IN_FLIGHT 2

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

typedef struct _vulkanDeviceContext VulkanDeviceContext;

struct _vulkanDeviceContext {
    VkDevice device;
    VkPhysicalDevice physicalDevice;
    VkSurfaceKHR surface;
    VkInstance vulkanInstance;

    VkDebugUtilsMessengerEXT debugMessenger;

    VkQueue graphicsQueue;
    //VkQueue presentQueue;
    ui32 graphicsFamily;

    VulkanSwapChainDetails *swapChainDetails;
    VulkanPipelineDetails *pipelineDetails;

    // Temporary vertex buffer for testing
    VulkanVertexBuffer *vertexBuffer;

    VkCommandPool commandPool;
    VkCommandBuffer commandBuffer;

    VkSemaphore imageAvailableSemaphore;
    VkSemaphore renderFinishedSemaphore;
    VkFence inFlightFence;
};

VulkanDeviceContext* vulkan_device_create();
void vulkan_device_cleanup(VulkanDeviceContext* context);

void vulkan_context_create_framebuffers(VulkanDeviceContext *context);
void vulkan_context_create_command_pool(VulkanDeviceContext *context);
void vulkan_context_create_sync(VulkanDeviceContext *context);

void vulkan_drawFrame(VulkanDeviceContext *context);

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

#endif // H_VULKAN_DEVICE
