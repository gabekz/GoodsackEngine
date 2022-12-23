#include "vulkan_uniform_buffer.h"

#include <string.h>

#include <core/api/vulkan/vulkan_buffer.h>
#include <core/api/vulkan/vulkan_support.h>

#include <util/gfx.h>
#include <util/maths.h>

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

void vulkan_uniform_buffer_update(ui32 currentImage, void **uniformBuffersMapped, VkExtent2D swapchainExtent) {
    mat4 init = GLM_MAT4_IDENTITY_INIT;
    UniformBufferObject ubo = {};
    glm_mat4_copy(init, ubo.model);
    glm_mat4_copy(init, ubo.proj);
    glm_mat4_copy(init, ubo.view);

    glm_lookat((vec3){2.0f, 2.0f, 2.0f}, (vec3){0, 0, 0}, (vec3){0, 0, 1}, ubo.view);

    glm_perspective(glm_rad(45.0f), (float)swapchainExtent.width / (float)swapchainExtent.height,
            0.1f, 10.0f, ubo.proj);

    ubo.proj[1][1] *= -1;

    glm_rotate(ubo.model, glm_rad(90.0f), (vec3){1, 0, 0});

    memcpy(uniformBuffersMapped[currentImage], &ubo, sizeof(UniformBufferObject));
}

//void vulkan_uniform_buffer_update2(VulkanDeviceContext *context,
//       void **uniformBuffersMapped) {
