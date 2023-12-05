/*
 * Copyright (c) 2022-2023, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#ifndef __DEVICE_CONTEXT_H__
#define __DEVICE_CONTEXT_H__

#include <util/gfx.h>
#include <util/sysdefs.h>

#include <core/drivers/vulkan/vulkan_device.h>

typedef struct _applicationProperties ApplicationProperties;

struct _applicationProperties
{
    const char *title;
    const char *description;

    struct
    {
        int major : 1;
        int minor : 1;
    } version;
};

GLFWwindow *
createWindow(int winWidth, int winHeight, VulkanDeviceContext **vkd);
// void cleanup(ContextProperties *contextProperties);

#endif // __DEVICE_CONTEXT_H__