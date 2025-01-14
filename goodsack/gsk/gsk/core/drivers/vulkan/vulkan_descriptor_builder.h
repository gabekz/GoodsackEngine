/*
 * Copyright (c) 2022-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#ifndef __VULKAN_DESCRIPTOR_BUILDER_H__
#define __VULKAN_DESCRIPTOR_BUILDER_H__

#include "util/gfx.h"
#include "util/sysdefs.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

typedef struct VulkanDescriptorAllocator
{

    struct
    {
        VkDescriptorPool *usedPools;
        VkDescriptorPool *freePools;
    };

} VulkanDescriptorAllocator;

typedef struct VulkanDescriptorLayoutCache
{
    int a;
} VulkanDescriptorLayoutCache;

// Descriptor Allocator

VulkanDescriptorAllocator *
vulkan_descriptor_allocator_init(VkDevice device);
void
vulkan_descriptor_allocator_reset();

int
vulkan_descriptor_allocate(VkDescriptorSet *set, VkDescriptorSetLayout layout);

void
vulkan_descriptor_allocator_cleanup(VulkanDescriptorAllocator *self);

// Descriptor Builder

void
vulkan_descriptor_builder_begin(VulkanDescriptorLayoutCache *layoutCache);
VkDescriptorSet
vulkan_descriptor_builder_end();

void
vulkan_descriptor_builder_bind_buffer(u32 binding,
                                      VkDescriptorBufferInfo *bufferInfo,
                                      VkDescriptorType descriptorType,
                                      VkShaderStageFlags shaderFlags);

void
vulkan_descriptor_builder_bind_image(u32 binding,
                                     VkDescriptorImageInfo *imageInfo,
                                     VkDescriptorType descriptorType,
                                     VkShaderStageFlags shaderFlags);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __VULKAN_DESCRIPTOR_BUILDER_H__
