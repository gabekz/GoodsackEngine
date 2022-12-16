#ifndef H_VULKAN_SUPPORT
#define H_VULKAN_SUPPORT

#include <util/logger.h>
#include <util/sysdefs.h>

#define MAX_FRAMES_IN_FLIGHT 2

#define VK_REQ_VALIDATION_SIZE 1
#define VK_REQ_VALIDATION_LIST { \
    "VK_LAYER_KHRONOS_validation"}

#define VK_REQ_DEVICE_EXT_SIZE  1
#define VK_REQ_DEVICE_EXT {     \
    "VK_KHR_swapchain"}

#define VK_CHECK(x)                                                 \
	do                                                              \
	{                                                               \
		VkResult err = x;                                           \
		if (err)                                                    \
		{                                                           \
            LOG_CRITICAL("Vulkan error: %s", x);                    \
		}                                                           \
    } while (0)

#endif // H_VULKAN_SUPPORT
