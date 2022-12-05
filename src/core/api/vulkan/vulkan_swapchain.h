#ifndef H_VULKAN_SWAPCHAIN
#define H_VULKAN_SWAPCHAIN

#include <util/gfx.h>
#include <util/sysdefs.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

typedef struct _vulkanSwapChainDetails VulkanSwapChainDetails;

struct _vulkanSwapChainDetails {
    VkSwapchainKHR swapchain;
    VkSurfaceFormatKHR *formats;
    VkPresentModeKHR *presentModes;
    VkSurfaceCapabilitiesKHR capabilities;
};

VulkanSwapChainDetails* vulkan_swapchain_create(
        VkDevice device, VkPhysicalDevice physicalDevice,
        VkSurfaceKHR surface, GLFWwindow *window);

// Helpers

// Create Swapchain Details structure
VulkanSwapChainDetails* vulkan_swapchain_query_details(
        VkPhysicalDevice device, VkSurfaceKHR surface);

VkSurfaceFormatKHR vulkan_swapchain_choose_format(
        VkSurfaceFormatKHR *formats, int count);

VkPresentModeKHR vulkan_swapchain_choose_present_mode(
        VkPresentModeKHR *modes, int count);

VkExtent2D vulkan_swapchain_choose_extent(
        VkSurfaceCapabilitiesKHR capabilities, GLFWwindow *window);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // H_VULKAN_SWAPCHAIN
