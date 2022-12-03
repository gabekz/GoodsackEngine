#ifndef H_VULKAN_DEVICE
#define H_VULKAN_DEVICE

#include <util/gfx.h>
#include <util/sysdefs.h>

#include <vulkan/vulkan.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

typedef struct _vulkanDeviceContext VulkanDeviceContext;

struct _vulkanDeviceContext {
    VkInstance *vulkanInstance;
    VkDevice *device;
    VkSurfaceKHR surface;

    VkQueue graphicsQueue;
    ui32 graphicsFamily;
};

VulkanDeviceContext* vulkan_device_create();

void vulkan_device_surface(VulkanDeviceContext* context);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // H_VULKAN_DEVICE
