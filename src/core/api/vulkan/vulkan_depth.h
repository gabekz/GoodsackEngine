#ifndef H_VULKAN_DEPTH
#define H_VULKAN_DEPTH

#include <util/gfx.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

typedef struct VulkanDepthResources
{
    VkImage depthImage;
    VkDeviceMemory depthImageMemory;
    VkImageView depthImageView;
} VulkanDepthResources;

VkFormat
vulkan_depth_find_format(VkPhysicalDevice physicalDevice);

VulkanDepthResources *
vulkan_depth_create_resources(VkPhysicalDevice physicalDevice,
                              VkDevice device,
                              VkExtent2D swapChainExtent);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // H_VULKAN_DEPTH
