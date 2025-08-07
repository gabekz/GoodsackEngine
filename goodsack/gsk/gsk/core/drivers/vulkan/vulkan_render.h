/*
 * Copyright (c) 2022-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#ifndef __VULKAN_RENDER_H__
#define __VULKAN_RENDER_H__

#include "core/drivers/vulkan/vulkan_device.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

void
vulkan_render_setup(VulkanDeviceContext *context);

// void vulkan_render_record(VulkanDeviceContext *context, u32 imageIndex,
//         VkCommandBuffer *commandBuffer);

// void vulkan_render_draw(VulkanDeviceContext *context, GLFWwindow *window);

void
vulkan_render_draw_begin(VulkanDeviceContext *context, GLFWwindow *window);
void
vulkan_render_draw_end(VulkanDeviceContext *context, GLFWwindow *window);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __VULKAN_RENDER_H__