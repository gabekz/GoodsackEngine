#include "vulkan_descriptor.h"
#include <core/api/vulkan/vulkan_support.h>

#include <stdlib.h>

#include <util/gfx.h>

void vulkan_descriptor_create_layout(VkDevice device,
        VkDescriptorSetLayout *descriptorSetLayout)
{
    ui32 bindingCount = 2;
    VkDescriptorSetLayoutBinding *bindings = malloc(
            sizeof(VkDescriptorSetLayoutBinding) * bindingCount);

    // UBO Layout
    bindings[0] = (VkDescriptorSetLayoutBinding){
        .binding = 0,
        .descriptorCount = 1, // number of values in the array (e.g, 1 UBO)
        .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,

        .stageFlags = VK_SHADER_STAGE_VERTEX_BIT, // UBO reference location
        .pImmutableSamplers = NULL, // optional
    };

    // Texture Sampler Layout
    bindings[1] = (VkDescriptorSetLayoutBinding){
        .binding = 1,
        .descriptorCount = 1, // number of values in the array (e.g, 1 UBO)
        .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,

        .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT, // Reference location
        .pImmutableSamplers = NULL, // optional
    };

    VkDescriptorSetLayoutCreateInfo layoutInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .bindingCount = bindingCount,
        .pBindings = bindings 
    };

    VK_CHECK(vkCreateDescriptorSetLayout(
            device, &layoutInfo, NULL, descriptorSetLayout));
}

void vulkan_descriptor_pool_create(VkDevice device,
        VkDescriptorPool *descriptorPool, VkDescriptorType type)
{
    ui32 poolSizeCount = 2;
    VkDescriptorPoolSize *poolSizes = malloc(
            sizeof(VkDescriptorPoolSize) * poolSizeCount);

    poolSizes[0] = (VkDescriptorPoolSize){
        .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .descriptorCount = (ui32)MAX_FRAMES_IN_FLIGHT,
    };
    poolSizes[1] = (VkDescriptorPoolSize){
        .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .descriptorCount = (ui32)MAX_FRAMES_IN_FLIGHT,
    };

    VkDescriptorPoolCreateInfo poolInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .poolSizeCount = poolSizeCount,
        .pPoolSizes = poolSizes,
        .maxSets = (ui32)MAX_FRAMES_IN_FLIGHT,
    };

    VK_CHECK(vkCreateDescriptorPool(
            device, &poolInfo, NULL, descriptorPool));
}

void vulkan_descriptor_sets_create(VkDevice device,
        VkDescriptorPool descriptorPool, VkDescriptorSet **descriptorSets,
        VkDescriptorType type, VkBuffer *uniformBuffers, ui32 structSize,
        VkDescriptorSetLayout layout,
        VkImageView textureImageView, VkSampler textureSampler)
{
    VkDescriptorSetLayout *layouts = malloc(
            sizeof(VkDescriptorSetLayout) * MAX_FRAMES_IN_FLIGHT);
    // TODO: FIX THIS SHIT
    layouts[0] = layout;
    layouts[1] = layout;

    VkDescriptorSetAllocateInfo allocInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .descriptorPool = descriptorPool,
        .descriptorSetCount = (ui32)MAX_FRAMES_IN_FLIGHT,
        .pSetLayouts = layouts,
    };

    VkDescriptorSet *sets = malloc(sizeof(VkDescriptorSet) * MAX_FRAMES_IN_FLIGHT);

    VK_CHECK(vkAllocateDescriptorSets(device, &allocInfo, sets));
    //descriptorSets = &sets;


    for(int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        // UBO info (binding 0)
        VkDescriptorBufferInfo bufferInfo = {
            .buffer = uniformBuffers[i],
            .offset = 0,
            .range = structSize
        };
        // Texture info (binding 1)
        VkDescriptorImageInfo imageInfo = {
            .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            .imageView = textureImageView,
            .sampler = textureSampler
        };

        ui32 descriptorWritesCount = 2;
        VkWriteDescriptorSet *descriptorWrites = malloc(
                sizeof(VkWriteDescriptorSet) * descriptorWritesCount);

        descriptorWrites[0] = (VkWriteDescriptorSet){
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet = sets[i],
            .dstBinding = 0,
            .dstArrayElement = 0,

            .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .descriptorCount = 1,

            .pBufferInfo = &bufferInfo,
            .pImageInfo = NULL, // optional
            .pTexelBufferView = NULL, // optional
        };

        descriptorWrites[1] = (VkWriteDescriptorSet){
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet = sets[i],
            .dstBinding = 1,
            .dstArrayElement = 0,

            .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .descriptorCount = 1,

            .pImageInfo = &imageInfo,
        };

        // Write descriptors
        vkUpdateDescriptorSets(device, descriptorWritesCount,
                descriptorWrites, 0, NULL);
    }

    *descriptorSets = sets;
}
