#include "vulkan_image.h"

#include <core/api/vulkan/vulkan_buffer.h>

#include <util/gfx.h>
#include <util/sysdefs.h>

void vulkan_image_create(VkPhysicalDevice physicalDevice, VkDevice device,
        VkImage *image, VkDeviceMemory *imageMemory,
        ui32 width, ui32 height, VkFormat format,
        VkImageTiling tiling, VkImageUsageFlags usage,
        VkMemoryPropertyFlags properties) {

    VkImageCreateInfo imageInfo = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .imageType = VK_IMAGE_TYPE_2D,
        .extent.width = width,
        .extent.height = height,
        .extent.depth = 1,
        .mipLevels = 1,
        .arrayLayers = 1,
        .format = format,
        .tiling = tiling,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .usage = usage,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE
    };

    VK_CHECK(vkCreateImage(device, &imageInfo, NULL, image));

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(device, *image, &memRequirements);

    VkMemoryAllocateInfo allocInfo = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = memRequirements.size,
        .memoryTypeIndex = vulkan_memory_type(
                physicalDevice, memRequirements.memoryTypeBits, properties)
    };

    // TODO: Pointer allocation fix (e.g, ** -> * deref)
    VK_CHECK(vkAllocateMemory(device, &allocInfo, NULL, imageMemory));

    vkBindImageMemory(device, *image, *imageMemory, 0);

}
