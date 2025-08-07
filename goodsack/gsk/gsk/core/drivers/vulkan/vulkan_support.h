/*
 * Copyright (c) 2022-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#ifndef __VULKAN_SUPPORT_H__
#define __VULKAN_SUPPORT_H__

#include "util/logger.h"
#include "util/sysdefs.h"

#define MAX_FRAMES_IN_FLIGHT 2

#define VK_REQ_VALIDATION_COUNT 1
#define VK_REQ_VALIDATION_LIST        \
    {                                 \
        "VK_LAYER_KHRONOS_validation" \
    }

#define VK_REQ_DEVICE_EXT_COUNT 1
#define VK_REQ_DEVICE_EXT  \
    {                      \
        "VK_KHR_swapchain" \
    }

#if defined(SYS_ENV_UNIX)
#define VK_PLATFORM_SURFACE_EXT "VK_KHR_xcb_surface"
#elif defined(SYS_ENV_WIN)
#define VK_PLATFORM_SURFACE_EXT "VK_KHR_win32_surface"
#endif

#define VK_EXTENSION_TEST_COUNT 3
#define VK_EXTENSION_TEST                          \
    {                                              \
        "VK_KHR_surface", VK_PLATFORM_SURFACE_EXT, \
          VK_EXT_DEBUG_UTILS_EXTENSION_NAME        \
    }

#define VK_CHECK(x)                                       \
    do                                                    \
    {                                                     \
        VkResult err = x;                                 \
        if (err) { LOG_CRITICAL("Vulkan error: %s", x); } \
    } while (0)

#endif // __VULKAN_SUPPORT_H__
