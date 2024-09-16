/*
 * Copyright (c) 2022-2023, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#include "vulkan_image.h"

#include "util/gfx.h"
#include "util/sysdefs.h"

#include "core/drivers/vulkan/vulkan_buffer.h"
#include "core/drivers/vulkan/vulkan_command.h"

void
vulkan_image_create(VkPhysicalDevice physicalDevice,
                    VkDevice device,
                    VkImage *image,
                    VkDeviceMemory *imageMemory,
                    u32 width,
                    u32 height,
                    VkFormat format,
                    VkImageTiling tiling,
                    VkImageUsageFlags usage,
                    VkMemoryPropertyFlags properties)
{

    VkImageCreateInfo imageInfo = {.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
                                   .imageType     = VK_IMAGE_TYPE_2D,
                                   .extent.width  = width,
                                   .extent.height = height,
                                   .extent.depth  = 1,
                                   .mipLevels     = 1,
                                   .arrayLayers   = 1,
                                   .format        = format,
                                   .tiling        = tiling,
                                   .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                                   .usage         = usage,
                                   .samples       = VK_SAMPLE_COUNT_1_BIT,
                                   .sharingMode   = VK_SHARING_MODE_EXCLUSIVE};

    VK_CHECK(vkCreateImage(device, &imageInfo, NULL, image));

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(device, *image, &memRequirements);

    VkMemoryAllocateInfo allocInfo = {
      .sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
      .allocationSize  = memRequirements.size,
      .memoryTypeIndex = vulkan_memory_type(
        physicalDevice, memRequirements.memoryTypeBits, properties)};

    // TODO: Pointer allocation fix (e.g, ** -> * deref)
    VK_CHECK(vkAllocateMemory(device, &allocInfo, NULL, imageMemory));

    vkBindImageMemory(device, *image, *imageMemory, 0);
}

void
vulkan_image_layout_transition(VkDevice device,
                               VkCommandPool commandPool,
                               VkQueue graphicsQueue,
                               VkImage image,
                               VkFormat format,
                               VkImageLayout prevLayout,
                               VkImageLayout newLayout)
{
    VkCommandBuffer commandBuffer =
      vulkan_command_stc_begin(device, commandPool);

    VkImageMemoryBarrier barrier = {
      .sType     = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
      .oldLayout = prevLayout, // TODO: Rename
      .newLayout = newLayout,

      .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
      .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,

      .image                           = image,
      .subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT,
      .subresourceRange.baseMipLevel   = 0,
      .subresourceRange.levelCount     = 1,
      .subresourceRange.baseArrayLayer = 0,
      .subresourceRange.layerCount     = 1,
    };

    VkPipelineStageFlags srcStage;
    VkPipelineStageFlags dstStage;

    if (prevLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
        newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
    {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    } else if (prevLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
               newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
    {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    } else
    {
        LOG_ERROR("Unsupported texture layout transition!");
    }

    vkCmdPipelineBarrier(
      commandBuffer, srcStage, dstStage, 0, 0, NULL, 0, NULL, 1, &barrier);

    vulkan_command_stc_end(device, graphicsQueue, commandBuffer, commandPool);
}

void
vulkan_image_copy_from_buffer(VkDevice device,
                              VkCommandPool commandPool,
                              VkQueue graphicsQueue,
                              VkBuffer buffer,
                              VkImage image,
                              u32 width,
                              u32 height)
{
    VkCommandBuffer commandBuffer =
      vulkan_command_stc_begin(device, commandPool);

    VkBufferImageCopy region = {
      .bufferOffset      = 0,
      .bufferRowLength   = 0,
      .bufferImageHeight = 0,

      .imageSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT,
      .imageSubresource.mipLevel       = 0,
      .imageSubresource.baseArrayLayer = 0,
      .imageSubresource.layerCount     = 1,

      .imageOffset = {0, 0, 0},
      .imageExtent = {width, height, 1},
    };

    vkCmdCopyBufferToImage(commandBuffer,
                           buffer,
                           image,
                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                           1,
                           &region);

    vulkan_command_stc_end(device, graphicsQueue, commandBuffer, commandPool);
}

VkImageView
vulkan_image_view_create(VkDevice device,
                         VkImage textureImage,
                         VkFormat format,
                         VkImageAspectFlags aspectFlags)
{
    VkImageView imageView;
    VkImageViewCreateInfo viewInfo = {
      .sType    = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
      .image    = textureImage,
      .viewType = VK_IMAGE_VIEW_TYPE_2D,
      .format   = format,

      .subresourceRange.aspectMask =
        aspectFlags, // e.g, VK_IMAGE_ASPECT_COLOR_BIT,
      .subresourceRange.baseMipLevel   = 0,
      .subresourceRange.levelCount     = 1,
      .subresourceRange.baseArrayLayer = 0,
      .subresourceRange.layerCount     = 1,
    };

    VK_CHECK(vkCreateImageView(device, &viewInfo, NULL, &imageView));
    return imageView;
}

VkSampler
vulkan_image_texture_sampler(VkDevice device,
                             VkPhysicalDeviceProperties deviceProperties)
{
    VkSampler textureSampler;

    VkSamplerCreateInfo samplerInfo = {
      .sType     = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
      .magFilter = VK_FILTER_LINEAR,
      .minFilter = VK_FILTER_LINEAR,

      .addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
      .addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
      .addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,

      .anisotropyEnable = VK_TRUE,
      .maxAnisotropy    = deviceProperties.limits.maxSamplerAnisotropy,

      // color when sampling beyond the image with clamp/border mode
      .borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,

      // VK_FALSE = [0, 1] | VK_TRUE = [0, texWidth/texHeight]
      .unnormalizedCoordinates = VK_FALSE,

      .compareEnable = VK_FALSE,
      .compareOp     = VK_COMPARE_OP_ALWAYS,

      .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
      .mipLodBias = 0.0f,
      .minLod     = 0.0f,
      .maxLod     = 0.0f};

    VK_CHECK(vkCreateSampler(device, &samplerInfo, NULL, &textureSampler));
    return textureSampler;
}
