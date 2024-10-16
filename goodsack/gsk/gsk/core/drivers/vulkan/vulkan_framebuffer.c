/*
 * Copyright (c) 2022-2023, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#include "vulkan_framebuffer.h"

#include <stdlib.h>

#include "util/gfx.h"
#include "util/sysdefs.h"

#include "core/drivers/vulkan/vulkan_support.h"

VkFramebuffer *
vulkan_framebuffer_create(VkDevice device,
                          u32 framebufferCount,
                          VkImageView *swapchainImageViews,
                          VkImageView depthImageView,
                          VkExtent2D swapchainExtent,
                          VkRenderPass renderPass)
{
    VkFramebuffer *framebuffers =
      malloc(sizeof(VkFramebuffer) *
             framebufferCount /* previously swapChainImageCount */);

    for (int i = 0; i < framebufferCount; i++)
    {
        VkImageView attachments[] = {swapchainImageViews[i], depthImageView};

        VkFramebufferCreateInfo framebufferInfo = {
          .sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
          .renderPass      = renderPass,
          .attachmentCount = 2,
          .pAttachments    = attachments,
          .width           = swapchainExtent.width,
          .height          = swapchainExtent.height,
          .layers          = 1};

        VK_CHECK(vkCreateFramebuffer(
          device, &framebufferInfo, NULL, &framebuffers[i]));
    }

    return framebuffers;
}
