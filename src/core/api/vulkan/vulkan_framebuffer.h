#ifndef H_VULKAN_FRAMEBUFFER
#define H_VULKAN_FRAMEBUFFER

#include <util/gfx.h>
#include <util/sysdefs.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

VkFramebuffer *
vulkan_framebuffer_create(VkDevice device,
                          ui32 framebufferCount,
                          VkImageView *swapchainImageViews,
                          VkImageView depthImageView,
                          VkExtent2D swapchainExtent,
                          VkRenderPass renderPass);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // H_VULKAN_FRAMEBUFFER
