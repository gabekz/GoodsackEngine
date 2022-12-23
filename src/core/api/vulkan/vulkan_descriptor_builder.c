#include "vulkan_descriptor_builder.h"

void vulkan_descriptor_builder_begin(VulkanDescriptorLayoutCache *layoutCache) {}

VkDescriptorSet vulkan_descriptor_builder_end() {}

void vulkan_descriptor_builder_bind_buffer(ui32 binding,
        VkDescriptorBufferInfo *bufferInfo,
        VkDescriptorType descriptorType,
        VkShaderStageFlags shaderFlags)
{

}

void vulkan_descriptor_builder_bind_image(ui32 binding,
        VkDescriptorImageInfo* imageInfo,
        VkDescriptorType descriptorType,
        VkShaderStageFlags shaderFlags)
{

}
