#include "vulkan_uniform_buffer.h"

#include <string.h>

#include <core/api/vulkan/vulkan_buffer.h>
#include <core/api/vulkan/vulkan_support.h>

#include <util/gfx.h>
#include <util/maths.h>

void vulkan_uniform_buffer_create_descriptor(VkDevice device, VkDescriptorSetLayout *descriptorSetLayout) {
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

void vulkan_uniform_buffer_create(
        VkPhysicalDevice physicalDevice, VkDevice device,
        VkBuffer **uniformBuffers, VkDeviceMemory **uniformBuffersMemory,
        void ***pMappedList)
{
    VkDeviceSize size = sizeof(UniformBufferObject);

    VkBuffer *buffers = malloc(sizeof(VkBuffer) * MAX_FRAMES_IN_FLIGHT);
    VkDeviceMemory *buffersMemory = malloc(sizeof(VkDeviceMemory) * MAX_FRAMES_IN_FLIGHT);

    //*uniformBuffers = malloc(sizeof(VkBuffer) * MAX_FRAMES_IN_FLIGHT);
    //*uniformBuffersMemory = malloc(sizeof(VkDeviceMemory) * MAX_FRAMES_IN_FLIGHT);

    void **uniformBuffersMapped = malloc(sizeof(void *) * MAX_FRAMES_IN_FLIGHT);
    *pMappedList = uniformBuffersMapped;

    for(int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vulkan_buffer_create(physicalDevice, device,
                size,
                VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                  VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                (&buffers[i]), (&buffersMemory[i]));

        VK_CHECK(vkMapMemory(device, buffersMemory[i], 0, size, 0, 
                &uniformBuffersMapped[i]));
    }
    *uniformBuffers = buffers;
    *uniformBuffersMemory = buffersMemory;

    *pMappedList = uniformBuffersMapped;
}

void vulkan_uniform_buffer_update(ui32 currentImage, void **uniformBuffersMapped) {
    mat4 init = GLM_MAT4_IDENTITY_INIT;
    UniformBufferObject ubo = {};
    glm_mat4_copy(init, ubo.model);
    glm_mat4_copy(init, ubo.proj);
    glm_mat4_copy(init, ubo.view);

    memcpy(uniformBuffersMapped[currentImage], &ubo, sizeof(UniformBufferObject));
}
