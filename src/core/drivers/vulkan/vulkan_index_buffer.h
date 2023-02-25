#ifndef H_VULKAN_INDEX_BUFFER
#define H_VULKAN_INDEX_BUFFER

#include <util/gfx.h>
#include <util/sysdefs.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

typedef struct VulkanIndexBuffer
{
    VkBuffer buffer;
    VkDeviceMemory bufferMemory;
    ui16 indicesCount;
} VulkanIndexBuffer;

VulkanIndexBuffer *
vulkan_index_buffer_create(VkPhysicalDevice physicalDevice,
                           VkDevice device,
                           VkCommandPool commandPool,
                           VkQueue graphicsQueue,
                           ui16 *indices,
                           ui16 indicesCount);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // H_VULKAN_INDEX_BUFFER
