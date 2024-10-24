/*
 * Copyright (c) 2022-2024, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#ifndef __DEVICE_CONTEXT_H__
#define __DEVICE_CONTEXT_H__

#include "util/gfx.h"
#include "util/sysdefs.h"

#include "core/drivers/vulkan/vulkan_device.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

typedef struct gsk_ApplicationProperties
{
    const char *title;
    const char *description;

    struct
    {
        int major : 1;
        int minor : 1;
    } version;
} gsk_ApplicationProperties;

GLFWwindow *
gsk_window_create(int win_width,
                  int win_height,
                  const char *win_image_path,
                  const char *win_app_title,
                  VulkanDeviceContext **vkd);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __DEVICE_CONTEXT_H__