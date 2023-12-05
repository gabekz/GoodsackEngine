/*
 * Copyright (c) 2022-2023, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#include "vulkan_swapchain.h"

#include <stdint.h>
#include <stdlib.h>

#include "util/logger.h"
#include "util/maths.h"

#include "core/drivers/vulkan/vulkan_depth.h"
#include "core/drivers/vulkan/vulkan_framebuffer.h"
#include "core/drivers/vulkan/vulkan_support.h"

VulkanSwapChainDetails *
vulkan_swapchain_query_details(VkPhysicalDevice device, VkSurfaceKHR surface)
{

    VulkanSwapChainDetails *details = malloc(sizeof(VulkanSwapChainDetails));

    VK_CHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
      device, surface, &details->capabilities));

    // Store Formats
    int formatCount = 0;
    VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(
      device, surface, &formatCount, NULL));

    if (formatCount != 0) {
        details->formats = malloc(sizeof(VkSurfaceFormatKHR) * formatCount);

        vkGetPhysicalDeviceSurfaceFormatsKHR(
          device, surface, &formatCount, details->formats);
    }

    // Store Present-Modes
    u32 presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(
      device, surface, &presentModeCount, NULL);

    if (presentModeCount != 0) {
        details->presentModes =
          malloc(sizeof(VkPresentModeKHR) * presentModeCount);

        vkGetPhysicalDeviceSurfacePresentModesKHR(
          device, surface, &presentModeCount, details->presentModes);
    }

    return details;
}
VkSurfaceFormatKHR
vulkan_swapchain_choose_format(VkSurfaceFormatKHR *formats, int count)
{
    // Find preferred format
    for (int i = 0; i < count; i++) {
        if (formats[i].format == VK_FORMAT_B8G8R8A8_SRGB &&
            formats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return formats[i];
        }
    }
    return formats[0];
}

VkPresentModeKHR
vulkan_swapchain_choose_present_mode(VkPresentModeKHR *modes, int count)
{
    for (int i = 0; i < count; i++) {
        if (modes[i] == VK_PRESENT_MODE_MAILBOX_KHR) { return modes[i]; }
    }
    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D
vulkan_swapchain_choose_extent(VkSurfaceCapabilitiesKHR capabilities,
                               GLFWwindow *window)
{
    if (capabilities.currentExtent.width != UINT32_MAX) {
        return capabilities.currentExtent;
    } else {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        VkExtent2D actualExtent = {(u32)width, (u32)height};

        actualExtent.width =
          MIN(MAX(actualExtent.width, capabilities.minImageExtent.width),
              capabilities.maxImageExtent.width);
        actualExtent.height =
          MIN(MAX(actualExtent.height, capabilities.minImageExtent.height),
              capabilities.maxImageExtent.height);
        // CLAMP(actualExtent.width, capabilities.minImageExtent.width,
        // capabilities.minImageExtent.width); CLAMP(actualExtent.height,
        // capabilities.minImageExtent.height,
        // capabilities.minImageExtent.height);

        return actualExtent;
    }
}

VulkanSwapChainDetails *
vulkan_swapchain_create(VkDevice device,
                        VkPhysicalDevice physicalDevice,
                        VkSurfaceKHR surface,
                        GLFWwindow *window)
{

    VulkanSwapChainDetails *details =
      vulkan_swapchain_query_details(physicalDevice, surface);

    VkSurfaceFormatKHR format =
      vulkan_swapchain_choose_format(details->formats, 1);
    VkPresentModeKHR presentMode =
      vulkan_swapchain_choose_present_mode(details->presentModes, 1);
    VkExtent2D extent =
      vulkan_swapchain_choose_extent(details->capabilities, window);

    u32 imageCount = details->capabilities.minImageCount + 1;

    if (details->capabilities.maxImageCount > 0 &&
        imageCount < details->capabilities.maxImageCount) {
        imageCount = details->capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo = {
      .sType            = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
      .surface          = surface,
      .minImageCount    = imageCount,
      .imageFormat      = format.format,
      .imageColorSpace  = format.colorSpace,
      .imageExtent      = extent,
      .imageArrayLayers = 1,
      .imageUsage       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,

      .imageSharingMode      = VK_SHARING_MODE_EXCLUSIVE,
      .queueFamilyIndexCount = 0,
      .pQueueFamilyIndices   = NULL,

      .preTransform   = details->capabilities.currentTransform,
      .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,

      .presentMode = presentMode,
      .clipped     = VK_TRUE,

      .oldSwapchain = VK_NULL_HANDLE};

    if (vkCreateSwapchainKHR(device, &createInfo, NULL, &details->swapchain) !=
        VK_SUCCESS) {
        LOG_ERROR("failed to create swap chain!");
    }

    vkGetSwapchainImagesKHR(device, details->swapchain, &imageCount, NULL);
    details->swapchainImages = malloc(sizeof(VkImage) * imageCount);
    vkGetSwapchainImagesKHR(
      device, details->swapchain, &imageCount, details->swapchainImages);

    details->swapchainImageFormat = format.format;
    details->swapchainExtent      = extent;

    // Craete Image Views
    details->swapchainImageViews = malloc(sizeof(VkImageView) * imageCount);
    details->swapchainImageCount = imageCount;

    for (int i = 0; i < imageCount; i++) {
        VkImageViewCreateInfo createInfo = {
          .sType    = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
          .image    = details->swapchainImages[i],
          .viewType = VK_IMAGE_VIEW_TYPE_2D,
          .format   = details->swapchainImageFormat,

          .components.r = VK_COMPONENT_SWIZZLE_IDENTITY,
          .components.g = VK_COMPONENT_SWIZZLE_IDENTITY,
          .components.b = VK_COMPONENT_SWIZZLE_IDENTITY,
          .components.a = VK_COMPONENT_SWIZZLE_IDENTITY,

          .subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT,
          .subresourceRange.baseMipLevel   = 0,
          .subresourceRange.levelCount     = 1,
          .subresourceRange.baseArrayLayer = 0,
          .subresourceRange.layerCount     = 1};

        if (vkCreateImageView(
              device, &createInfo, NULL, &details->swapchainImageViews[i]) !=
            VK_SUCCESS) {
            LOG_ERROR("Failed to create image views!");
        }
    }

    return details;
}

VulkanSwapChainDetails *
vulkan_swapchain_recreate(VkPhysicalDevice physicalDevice,
                          VkDevice device,
                          VulkanSwapChainDetails *swapChainDetails,
                          VkSurfaceKHR surface,
                          VkRenderPass renderPass,
                          VulkanDepthResources **ptrDepthResources,
                          GLFWwindow *window)
{
    vkDeviceWaitIdle(device);

    // cleanup OLD swapchain
    vulkan_swapchain_cleanup(device, swapChainDetails);
    // TODO: Free?

    // Create a NEW swapchain
    swapChainDetails =
      vulkan_swapchain_create(device, physicalDevice, surface, window);

    // Recreate depth resources
    *ptrDepthResources = vulkan_depth_create_resources(
      physicalDevice, device, swapChainDetails->swapchainExtent);

    VulkanDepthResources p     = **ptrDepthResources;
    VkImageView depthImageView = p.depthImageView;

    u32 framebufferCount = swapChainDetails->swapchainImageCount;
    swapChainDetails->swapchainFramebuffers =
      vulkan_framebuffer_create(device,
                                framebufferCount,
                                swapChainDetails->swapchainImageViews,
                                depthImageView,
                                swapChainDetails->swapchainExtent,
                                renderPass);

    return swapChainDetails;
}

void
vulkan_swapchain_cleanup(VkDevice device,
                         VulkanSwapChainDetails *swapchainDetails)
{
    // TODO: may be incorrect sizes. i.e. framebuffer is not same count
    for (int i = 0; i < swapchainDetails->swapchainImageCount; i++) {
        vkDestroyFramebuffer(
          device, swapchainDetails->swapchainFramebuffers[i], NULL);
        vkDestroyImageView(
          device, swapchainDetails->swapchainImageViews[i], NULL);
    }

    vkDestroySwapchainKHR(device, swapchainDetails->swapchain, NULL);
}
