/*
 * Copyright (c) 2022-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#ifndef __GSK_VULKAN_VERTEX_ARRAY_H__
#define __GSK_VULKAN_VERTEX_ARRAY_H__

#include "util/gfx.h"
#include "util/sysdefs.h"

#include "core/drivers/vulkan/vulkan_command.h"
#include "core/drivers/vulkan/vulkan_index_buffer.h"
#include "core/drivers/vulkan/vulkan_vertex_buffer.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

typedef struct gsk_VulkanVertexArray
{
    VulkanIndexBuffer *p_index_buffer;
    VulkanVertexBuffer *p_vertex_buffers[8];
    VkBuffer *buffers;
    u32 vertex_buffers_count;

    VkBuffer *p_vk_buff;

    u32 bindings_count;
    u32 attributes_count;
    u32 attributes_accum_counter;

    VkVertexInputBindingDescription2EXT vertex_binding_descriptions[8];
    VkVertexInputAttributeDescription2EXT vertex_attribute_descriptions[8];

} gsk_VulkanVertexArray;

gsk_VulkanVertexArray
gsk_vulkan_vertex_array_create();

void
gsk_vulkan_vertex_array_push(gsk_VulkanVertexArray *p_self,
                             VulkanVertexBuffer *p_vertex_buffer,
                             f32 stride);

void
gsk_vulkan_vertex_array_add_attrib(gsk_VulkanVertexArray *p_self,
                                   u32 binding,
                                   u32 location,
                                   f32 offset);

void
gsk_vulkan_vertex_array_bind(gsk_VulkanVertexArray *p_self,
                             VkCommandBuffer *p_command_buffer);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __GSK_VULKAN_VERTEX_ARRAY_H__