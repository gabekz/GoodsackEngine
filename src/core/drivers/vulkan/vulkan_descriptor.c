#include "vulkan_descriptor.h"
#include <core/drivers/vulkan/vulkan_support.h>

#include <stdlib.h>

VkDescriptorPool
vulkan_descriptor_pool_create(VkDevice device)
{
    ui32 poolSizesCount              = 11;
    VkDescriptorPoolSize poolSizes[] = {
      // Descriptor Type                             Descriptor Count
      {VK_DESCRIPTOR_TYPE_SAMPLER, 10},
      {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 10},
      {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 10},
      {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 10},
      {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 10},
      {VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 10},
      {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 10},
      {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 10},
      {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 10},
      {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 10},
      {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 10},
    };

    VkDescriptorPoolCreateInfo poolInfo = {
      .sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
      .poolSizeCount = poolSizesCount,
      .pPoolSizes    = poolSizes,
      .maxSets       = (ui32)MAX_FRAMES_IN_FLIGHT * 10,
    };

    VkDescriptorPool descriptorPool;
    VK_CHECK(vkCreateDescriptorPool(device, &poolInfo, NULL, &descriptorPool));

    return descriptorPool;
}

VkDescriptorSetLayout
vulkan_descriptor_create_layout(VkDevice device)
{
    ui32 bindingCount = 2;
    VkDescriptorSetLayoutBinding bindings[2];

    // UBO Layout
    bindings[0] = (VkDescriptorSetLayoutBinding) {
      .binding         = 0,
      .descriptorCount = 1, // number of values in the array (e.g, 1 UBO)
      .descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,

      .stageFlags = VK_SHADER_STAGE_VERTEX_BIT, // UBO reference location
      .pImmutableSamplers = NULL,               // optional
    };

    // Texture Sampler Layout
    bindings[1] = (VkDescriptorSetLayoutBinding) {
      .binding         = 1,
      .descriptorCount = 1, // number of values in the array (e.g, 1 UBO)
      .descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,

      .stageFlags         = VK_SHADER_STAGE_FRAGMENT_BIT, // Reference location
      .pImmutableSamplers = NULL,                         // optional
    };

    VkDescriptorSetLayoutCreateInfo layoutInfo = {
      .sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
      .bindingCount = bindingCount,
      .pBindings    = bindings};

    VkDescriptorSetLayout descriptorSetLayout;

    VK_CHECK(vkCreateDescriptorSetLayout(
      device, &layoutInfo, NULL, &descriptorSetLayout));

    return descriptorSetLayout;
}

VkDescriptorSet *
vulkan_descriptor_sets_create(VkDevice device,
                              VkDescriptorPool descriptorPool,
                              VkDescriptorSetLayout layout,
                              VkBuffer *uniformBuffers,
                              ui32 structSize,
                              VkImageView textureImageView,
                              VkSampler textureSampler)
{
    // TODO: Make a proper multiplier for the layout
    VkDescriptorSetLayout layouts[MAX_FRAMES_IN_FLIGHT];
    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        layouts[i] = layout;
    }

    VkDescriptorSetAllocateInfo allocInfo = {
      .sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
      .descriptorPool     = descriptorPool,
      .descriptorSetCount = (ui32)MAX_FRAMES_IN_FLIGHT,
      .pSetLayouts        = layouts,
    };

    VkDescriptorSet *sets =
      malloc(sizeof(VkDescriptorSet) * MAX_FRAMES_IN_FLIGHT);

    VK_CHECK(vkAllocateDescriptorSets(device, &allocInfo, sets));

    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        // UBO info (binding 0)
        VkDescriptorBufferInfo bufferInfo = {
          .buffer = uniformBuffers[i], .offset = 0, .range = structSize};
        // Texture info (binding 1)
        VkDescriptorImageInfo imageInfo = {
          .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
          .imageView   = textureImageView,
          .sampler     = textureSampler};

        ui32 descriptorWritesCount = 2;
        VkWriteDescriptorSet descriptorWrites[2];

        descriptorWrites[0] = (VkWriteDescriptorSet) {
          .sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
          .dstSet          = sets[i],
          .dstBinding      = 0,
          .dstArrayElement = 0,

          .descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
          .descriptorCount = 1,

          .pBufferInfo      = &bufferInfo,
          .pImageInfo       = NULL, // optional
          .pTexelBufferView = NULL, // optional
        };

        descriptorWrites[1] = (VkWriteDescriptorSet) {
          .sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
          .dstSet          = sets[i],
          .dstBinding      = 1,
          .dstArrayElement = 0,

          .descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
          .descriptorCount = 1,

          .pImageInfo = &imageInfo,
        };

        // Write descriptors
        vkUpdateDescriptorSets(
          device, descriptorWritesCount, descriptorWrites, 0, NULL);
    }

    return sets;
}
