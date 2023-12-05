/*
 * Copyright (c) 2022-2023, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#ifndef __VULKAN_FRAMEBUFFER_H__
#define __VULKAN_FRAMEBUFFER_H__

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

#endif // __VULKAN_FRAMEBUFFER_H__
