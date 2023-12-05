/*
 * Copyright (c) 2022-2023, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#include "vulkan_descriptor_builder.h"

void
vulkan_descriptor_builder_begin(VulkanDescriptorLayoutCache *layoutCache)
{
}

VkDescriptorSet
vulkan_descriptor_builder_end()
{
    return NULL;
}

void
vulkan_descriptor_builder_bind_buffer(u32 binding,
                                      VkDescriptorBufferInfo *bufferInfo,
                                      VkDescriptorType descriptorType,
                                      VkShaderStageFlags shaderFlags)
{
}

void
vulkan_descriptor_builder_bind_image(u32 binding,
                                     VkDescriptorImageInfo *imageInfo,
                                     VkDescriptorType descriptorType,
                                     VkShaderStageFlags shaderFlags)
{
}
