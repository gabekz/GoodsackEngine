#ifndef H_VULKAN_UNIFORM_BUFFER
#define H_VULKAN_UNIFORM_BUFFER

#include <util/gfx.h>
#include <util/maths.h>
#include <util/sysdefs.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

typedef struct UniformBufferObject {
    mat4 model, view, proj;
} UniformBufferObject;

void vulkan_uniform_buffer_create_descriptor(
        VkDevice device,
        VkDescriptorSetLayout *descriptorSetLayout);

void vulkan_uniform_buffer_create(
        VkPhysicalDevice physicalDevice, VkDevice device,
        VkBuffer **uniformBuffers, VkDeviceMemory **uniformBuffersMemory,
        void ***pMappedList);

void vulkan_uniform_buffer_update(ui32 currentImage, 
        void **uniformBuffersMapped, VkExtent2D swapchainExtent);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // H_VULKAN_UNIFORM_BUFFER
