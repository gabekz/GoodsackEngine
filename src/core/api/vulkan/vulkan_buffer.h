#ifndef H_VULKAN_BUFFER
#define H_VULKAN_BUFFER

#include <util/gfx.h>

void vulkan_buffer_create(VkPhysicalDevice physicalDevice,
                          VkDevice device,
                          VkDeviceSize size,
                          VkBufferUsageFlags usage,
                          VkMemoryPropertyFlags properties,
                          VkBuffer *buffer,
                          VkDeviceMemory *bufferMemory);

void vulkan_buffer_copy(VkDevice device, VkQueue graphicsQueue,
                        VkCommandPool commandPool,
                        VkBuffer srcBuffer,
                        VkBuffer dstBuffer,
                        VkDeviceSize size);

#endif // H_VULKAN_BUFFER
