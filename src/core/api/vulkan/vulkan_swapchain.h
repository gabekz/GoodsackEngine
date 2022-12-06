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
    ui32 swapchainImageCount;
    VkImage *swapchainImages;
    VkFormat swapchainImageFormat;
    VkExtent2D swapchainExtent;

    VkImageView *swapchainImageViews;

    VkSurfaceFormatKHR *formats;
    VkPresentModeKHR *presentModes;
    VkSurfaceCapabilitiesKHR capabilities;

    VkFramebuffer *swapchainFramebuffers;
};

// Create Swapchain
VulkanSwapChainDetails* vulkan_swapchain_create(
        VkDevice device, VkPhysicalDevice physicalDevice,
        VkSurfaceKHR surface, GLFWwindow *window);

// Create Swapchain Details structure
VulkanSwapChainDetails* vulkan_swapchain_query_details(
        VkPhysicalDevice device, VkSurfaceKHR surface);

// Create Swapchain Image Views
void vulkan_swapchain_create_image_views(VulkanSwapChainDetails *details);

// Helpers

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
