/*
 * Copyright (c) 2022-2023, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#ifndef __VULKAN_PIPELINE_H__
#define __VULKAN_PIPELINE_H__

#include "util/gfx.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

typedef struct _vulkanPipelineDetails VulkanPipelineDetails;

struct _vulkanPipelineDetails
{
    VkPipeline graphicsPipeline;
    VkRenderPass renderPass;

    VkDescriptorSetLayout descriptorSetLayout;
    VkPipelineLayout pipelineLayout;

    VkShaderModule vertShaderModule;
    VkShaderModule fragShaderModule;
};

VulkanPipelineDetails *
vulkan_pipeline_create(VkPhysicalDevice physicalDevice,
                       VkDevice device,
                       VkFormat swapchainImageFormat,
                       VkExtent2D swapchainExtent);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __VULKAN_PIPELINE_H__
