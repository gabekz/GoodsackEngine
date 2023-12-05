/*
 * Copyright (c) 2022-2023, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#ifndef __VULKAN_DESCRIPTOR_H__
#define __VULKAN_DESCRIPTOR_H__

#include <util/gfx.h>
#include <util/sysdefs.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

// Create descriptor pool
VkDescriptorPool
vulkan_descriptor_pool_create(VkDevice device);

// Creates Vulkan DescriptorSet layouts - fit for 1 UBO (MVP) and
// 1 TextureSampler (albedo)
VkDescriptorSetLayout
vulkan_descriptor_create_layout(VkDevice device);

// Creates a descriptor set fit for 1 UBO (MVP) and 1 TextureSampler (albedo)
VkDescriptorSet *
vulkan_descriptor_sets_create(VkDevice device,
                              VkDescriptorPool descriptorPool,
                              VkDescriptorSetLayout layout,
                              VkBuffer *uniformBuffers,
                              ui32 structSize,
                              VkImageView textureImageView,
                              VkSampler textureSampler);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __VULKAN_DESCRIPTOR_H__
