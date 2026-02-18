/*
 * Copyright (c) 2022-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#include "vulkan_vertex_array.h"

#include "util/gfx.h"
#include "util/logger.h"

#include "core/drivers/vulkan/vulkan.h"
#include "core/drivers/vulkan/vulkan_buffer.h"
#include "core/drivers/vulkan/vulkan_vertex_buffer.h"

#define MAX_BINDINGS 4

gsk_VulkanVertexArray
gsk_vulkan_vertex_array_create()
{
    gsk_VulkanVertexArray ret    = {0};
    ret.buffers                  = malloc(sizeof(VkBuffer) * 10);
    ret.attributes_accum_counter = 0;
    ret.attributes_count         = 0;
    ret.bindings_count           = 0;
    return ret;
}

void
gsk_vulkan_vertex_array_push(gsk_VulkanVertexArray *p_self,
                             VulkanVertexBuffer *p_vertex_buffer,
                             f32 stride)
{
    if (p_self == NULL) { LOG_CRITICAL("vertex array is NULL"); }

    if (p_self->vertex_buffers_count <= 0)
    {
        p_self->p_vk_buff = &p_vertex_buffer->buffer;
    }

    p_self->buffers[p_self->vertex_buffers_count] = p_vertex_buffer->buffer;
    p_self->p_vertex_buffers[p_self->vertex_buffers_count] = p_vertex_buffer;
    p_self->vertex_buffers_count += 1;

    // TODO: probably have to malloc here. weird issue
    // also push binding
    p_self->vertex_binding_descriptions[p_self->bindings_count] =
      (VkVertexInputBindingDescription2EXT) {
        .sType     = VK_STRUCTURE_TYPE_VERTEX_INPUT_BINDING_DESCRIPTION_2_EXT,
        .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
        .binding   = p_self->bindings_count,
        .stride    = stride,
        .divisor   = 1,
      };
    p_self->bindings_count += 1;
    p_self->attributes_accum_counter = 0;
}

void
gsk_vulkan_vertex_array_add_attrib(gsk_VulkanVertexArray *p_self,
                                   u32 binding,
                                   u32 location,
                                   f32 offset)
{
    p_self->vertex_attribute_descriptions[p_self->attributes_count] =
      (VkVertexInputAttributeDescription2EXT) {
        .sType    = VK_STRUCTURE_TYPE_VERTEX_INPUT_ATTRIBUTE_DESCRIPTION_2_EXT,
        .format   = VK_FORMAT_R32G32B32_SFLOAT,
        .binding  = p_self->bindings_count - 1,
        .location = p_self->attributes_count,
        .offset   = offset,
      };
    p_self->attributes_count += 1;
    p_self->attributes_accum_counter += 1;
}

void
gsk_vulkan_vertex_array_bind(gsk_VulkanVertexArray *p_self,
                             VkCommandBuffer *p_command_buffer)
{
    if (p_self == NULL) { LOG_CRITICAL("vertex array is NULL"); }
    if (p_self->vertex_buffers_count <= 0)
    {
        LOG_CRITICAL("no vertex buffers");
        return;
    }

    VkDeviceSize offsets[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    vkCmdBindVertexBuffers(*p_command_buffer,
                           0,
                           p_self->vertex_buffers_count,
                           p_self->buffers,
                           offsets);

    if (p_self->p_index_buffer)
    {
        vkCmdBindIndexBuffer(*p_command_buffer,
                             p_self->p_index_buffer->buffer,
                             0,
                             VK_INDEX_TYPE_UINT32);
    }
}