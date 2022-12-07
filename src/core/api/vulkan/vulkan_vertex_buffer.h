#ifndef H_VULKAN_VERTEX_BUFFER
#define H_VULKAN_VERTEX_BUFFER

#include <util/gfx.h>

#if __cplusplus
extern "C" {
#endif // __cplusplus

typedef struct VulkanVertexBuffer VulkanVertexBuffer;

struct VulkanVertexBuffer {

    VkBuffer buffer;
    VkDeviceMemory bufferMemory;

    void *data;
    ui32 size;
};

VulkanVertexBuffer* vulkan_vertex_buffer_create(VkPhysicalDevice physicalDevice, VkDevice device, void *data, ui32 size);
void vulkan_vertex_buffer_cleanup(VkDevice device, VkBuffer buffer);

VkVertexInputBindingDescription vulkan_vertex_buffer_get_binding_description();
VkVertexInputAttributeDescription* vulkan_vertex_buffer_get_attribute_descriptions();

#if __cplusplus
}
#endif // __cplusplus

#endif // H_VULKAN_VERTEX_BUFFER
