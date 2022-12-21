#ifndef H_VULKAN_RENDER
#define H_VULKAN_RENDER

#include <core/api/vulkan/vulkan_device.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

void vulkan_render_setup(VulkanDeviceContext *context);

void vulkan_render_record(VulkanDeviceContext *context, ui32 imageIndex,
        VkCommandBuffer *commandBuffer);

void vulkan_render_draw(VulkanDeviceContext *context, GLFWwindow *window);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // H_VULKAN_RENDER
