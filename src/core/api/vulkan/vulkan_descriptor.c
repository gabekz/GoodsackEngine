#include "vulkan_descriptor.h"
#include <core/api/vulkan/vulkan_support.h>

#include <stdlib.h>

#include <util/gfx.h>

void vulkan_descriptor_create_layout(VkDevice device,
        VkDescriptorSetLayout *descriptorSetLayout)
{
    VkDescriptorSetLayoutBinding uboLayoutBinding = {
        .binding = 0,
        .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .descriptorCount = 1, // number of values in the array (e.g, 1 UBO)

        .stageFlags = VK_SHADER_STAGE_VERTEX_BIT, // UBO reference location
        .pImmutableSamplers = NULL, // optional
    };

    VkDescriptorSetLayoutCreateInfo layoutInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .bindingCount = 1,
        .pBindings = &uboLayoutBinding
    };

    VK_CHECK(vkCreateDescriptorSetLayout(
            device, &layoutInfo, NULL, descriptorSetLayout));
}

void vulkan_descriptor_pool_create(VkDevice device,
        VkDescriptorPool *descriptorPool, VkDescriptorType type)
{
    VkDescriptorPoolSize poolSize = {
        .type = type, // e.g, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER
        .descriptorCount = (ui32)MAX_FRAMES_IN_FLIGHT,
    };

    VkDescriptorPoolCreateInfo poolInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .poolSizeCount = 1,
        .pPoolSizes = &poolSize,
        .maxSets = (ui32)MAX_FRAMES_IN_FLIGHT,
    };

    VK_CHECK(vkCreateDescriptorPool(
            device, &poolInfo, NULL, descriptorPool));
}

void vulkan_descriptor_sets_create(VkDevice device,
        VkDescriptorPool descriptorPool, VkDescriptorSet **descriptorSets,
        VkDescriptorType type, VkBuffer *uniformBuffers, ui32 structSize, VkDescriptorSetLayout layout)
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
        VkDescriptorBufferInfo bufferInfo = {
            .buffer = uniformBuffers[i],
            .offset = 0,
            .range = structSize
        };

        VkWriteDescriptorSet descriptorWrite = {
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

        vkUpdateDescriptorSets(device, 1, &descriptorWrite, 0, NULL);
    }

    *descriptorSets = sets;
}
