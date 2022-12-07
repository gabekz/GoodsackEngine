#include "vulkan_vertex_buffer.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <util/logger.h>

VkVertexInputBindingDescription vulkan_vertex_buffer_get_binding_description() {
    VkVertexInputBindingDescription bindingDescription = {
        .binding = 0,
        .stride = sizeof(float),
        .inputRate = VK_VERTEX_INPUT_RATE_VERTEX
    };
    return bindingDescription;
}

VkVertexInputAttributeDescription* vulkan_vertex_buffer_get_attribute_descriptions() {
    VkVertexInputAttributeDescription *attributeDescriptions =
        malloc(sizeof(VkVertexInputAttributeDescription) * 2);

    attributeDescriptions[0] = (VkVertexInputAttributeDescription){
        .binding = 0,
        .location = 0,
        .format = VK_FORMAT_R32G32_SFLOAT,
        .offset = sizeof(float) // position
    };
    attributeDescriptions[1] = (VkVertexInputAttributeDescription){
        .binding = 0,
        .location = 1,
        .format = VK_FORMAT_R32G32_SFLOAT,
        .offset = sizeof(float) // color
    };

    return attributeDescriptions;
}

static ui32 _findMemoryType(VkPhysicalDevice physicalDevice, 
        ui32 typeFilter, VkMemoryPropertyFlags properties) {

    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

    for(ui32 i = 0; i < memProperties.memoryTypeCount; i++) {
        if((typeFilter & (1 << i)) && 
                (memProperties.memoryTypes[i].propertyFlags & properties) ==
                properties) {
            return i;
        }
    }
    LOG_ERROR("Failed to find suitable memory type!");
    return 0;
}

VulkanVertexBuffer* vulkan_vertex_buffer_create(VkPhysicalDevice physicalDevice, VkDevice device, void *data, ui32 size) {

    VulkanVertexBuffer *vertexBuffer = malloc(sizeof(VulkanVertexBuffer));

    VkBufferCreateInfo bufferInfo = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = size,
        .usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE
    };

    if(vkCreateBuffer(device, &bufferInfo, NULL, &vertexBuffer->buffer) != VK_SUCCESS) {
        LOG_ERROR("Failed to create vertex buffer!");
    }

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(device, vertexBuffer->buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = memRequirements.size,
        .memoryTypeIndex = _findMemoryType(physicalDevice,
                memRequirements.memoryTypeBits, 
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | 
                VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)
    };

    if(vkAllocateMemory(device, &allocInfo, NULL, &vertexBuffer->bufferMemory) != VK_SUCCESS) {
        LOG_ERROR("Failed to allocate vertex buffer memory!");
    }

    vkBindBufferMemory(device, vertexBuffer->buffer, vertexBuffer->bufferMemory, 0);
    vkMapMemory(device, vertexBuffer->bufferMemory, 0, bufferInfo.size, 0, &data);

    // send to structure
    vertexBuffer->data = malloc(bufferInfo.size);
    memcpy(data, vertexBuffer->data, bufferInfo.size);

    vkUnmapMemory(device, vertexBuffer->bufferMemory);

    // NOTE:
    // Option 1): VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
    // Option 2):
    // Call vkFlushMappedMemoryRanges after writing to the mapped memory,
    // and call vkInvalidateMappedMemoryRanges before reading from the
    // mapped memory

    //VulkanVertexBuffer *vertexBuffer = malloc(sizeof(VulkanVertexBuffer));



    return vertexBuffer;
}

void vulkan_vertex_buffer_cleanup(VkDevice device, VkBuffer buffer) {
    // TODO:
    vkDestroyBuffer(device, buffer, NULL);
    //vkFreeMemory(device, vertexBufferMemory, NULL);

}
