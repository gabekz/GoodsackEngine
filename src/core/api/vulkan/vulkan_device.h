#ifndef H_VULKAN_DEVICE
#define H_VULKAN_DEVICE

#include <util/gfx.h>
#include <util/sysdefs.h>

#include <core/api/vulkan/vulkan_swapchain.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

typedef struct _vulkanDeviceContext VulkanDeviceContext;

struct _vulkanDeviceContext {
    VkDevice device;
    VkPhysicalDevice physicalDevice;
    VkSurfaceKHR surface;
    VkInstance vulkanInstance;

    VkQueue graphicsQueue;
    //VkQueue presentQueue;
    ui32 graphicsFamily;

    VulkanSwapChainDetails *swapChainDetails;
};

VulkanDeviceContext* vulkan_device_create();
void vulkan_device_cleanup(VulkanDeviceContext* context);

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
