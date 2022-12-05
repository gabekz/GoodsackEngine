#ifndef H_VULKAN_SUPPORT
#define H_VULKAN_SUPPORT

#include <util/sysdefs.h>

#define VK_REQ_VALIDATION_SIZE 1
#define VK_REQ_VALIDATION_LIST { \
    "VK_LAYER_KHRONOS_validation"}

#define VK_REQ_DEVICE_EXT_SIZE  1
#define VK_REQ_DEVICE_EXT {     \
    "VK_KHR_swapchain"}

#endif // H_VULKAN_SUPPORT
