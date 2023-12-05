/*
 * Copyright (c) 2022-2023, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#include "vulkan_depth.h"

#include <stdlib.h>

#include "util/gfx.h"
#include "core/drivers/vulkan/vulkan_image.h"

static VkFormat
_FindSupportedFormat(VkPhysicalDevice physicalDevice,
                     VkFormat *candidates,
                     int candidateCount,
                     VkImageTiling tiling,
                     VkFormatFeatureFlags features)
{
    for (int i = 0; i < candidateCount; i++) {
        VkFormat format = candidates[i];
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);

        if (tiling == VK_IMAGE_TILING_LINEAR &&
            (props.linearTilingFeatures & features) == features) {
            return format;
        } else if (tiling == VK_IMAGE_TILING_OPTIMAL &&
                   (props.optimalTilingFeatures & features) == features) {
            return format;
        }
    }

    LOG_CRITICAL("Failed to find supported format!");
    VkFormat failed = (VkFormat)NULL;
    return failed;
}

static ui32
_HasStencilComponent(VkFormat format)
{
    return format == VK_FORMAT_D32_SFLOAT_S8_UINT ||
           format == VK_FORMAT_D24_UNORM_S8_UINT;
}

VkFormat
vulkan_depth_find_format(VkPhysicalDevice physicalDevice)
{
    VkFormat candidates[] = {VK_FORMAT_D32_SFLOAT,
                             VK_FORMAT_D32_SFLOAT_S8_UINT,
                             VK_FORMAT_D24_UNORM_S8_UINT};

    return _FindSupportedFormat(physicalDevice,
                                candidates,
                                3,
                                VK_IMAGE_TILING_OPTIMAL,
                                VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}

VulkanDepthResources *
vulkan_depth_create_resources(VkPhysicalDevice physicalDevice,
                              VkDevice device,
                              VkExtent2D swapChainExtent)
{

    VulkanDepthResources *ret = malloc(sizeof(VulkanDepthResources));

    VkFormat depthFormat = vulkan_depth_find_format(physicalDevice);

    vulkan_image_create(physicalDevice,
                        device,
                        &ret->depthImage,
                        &ret->depthImageMemory,
                        swapChainExtent.width,
                        swapChainExtent.height,
                        depthFormat,
                        VK_IMAGE_TILING_OPTIMAL,
                        VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    ret->depthImageView = vulkan_image_view_create(
      device, ret->depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);

    // Explicitly transitionioning the depth image
    // vulkan_image_layout_transition();

    return ret;
}
