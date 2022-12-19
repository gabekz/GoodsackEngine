#ifndef H_VULKAN_DESCRIPTOR
#define H_VULKAN_DESCRIPTOR

#include <util/gfx.h>
#include <util/sysdefs.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

void vulkan_descriptor_create_layout();

void vulkan_descriptor_pool_create(VkDevice device,
        VkDescriptorPool *descriptorPool, VkDescriptorType type);

void vulkan_descriptor_sets_create(VkDevice device,
        VkDescriptorPool descriptorPool, VkDescriptorSet **descriptorSets,
        VkDescriptorType type, VkBuffer *uniformBuffers, ui32 structSize, VkDescriptorSetLayout layout);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // H_VULKAN_DESCRIPTOR
