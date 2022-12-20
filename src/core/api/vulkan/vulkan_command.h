#ifndef H_VULKAN_COMMAND
#define H_VULKAN_COMMAND

#include <util/gfx.h>
#include <util/sysdefs.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

// Allocate and return a command pool (based on queueFamilyIndices)
VkCommandPool vulkan_command_pool_create(VkPhysicalDevice physicalDevice,
        VkDevice device);

// Allocates a list of command buffers per available MAX_FRAMES_IN_FLIGHT
VkCommandBuffer *vulkan_command_buffer_create(VkDevice device,
        VkCommandPool commandPool);

// Begin SingleTimeCommands
VkCommandBuffer vulkan_command_stc_begin(VkDevice device,
        VkCommandPool commandPool);

// End SingleTimeCommands
void vulkan_command_stc_end(VkDevice device, VkQueue graphicsQueue,
        VkCommandBuffer commandBuffer, VkCommandPool commandPool);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // H_VULKAN_COMMAND
