/*
 * Copyright (c) 2022-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#ifndef __VULKAN_UNIFORM_BUFFER_H__
#define __VULKAN_UNIFORM_BUFFER_H__

#include "util/gfx.h"
#include "util/maths.h"
#include "util/sysdefs.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

typedef struct UniformBufferObject
{
    mat4 model, view, proj;
} UniformBufferObject;

void
vulkan_uniform_buffer_create(VkPhysicalDevice physicalDevice,
                             VkDevice device,
                             VkBuffer **uniformBuffers,
                             VkDeviceMemory **uniformBuffersMemory,
                             void ***pMappedList);

void
vulkan_uniform_buffer_update(u32 currentImage,
                             void **uniformBuffersMapped,
                             VkExtent2D swapchainExtent);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __VULKAN_UNIFORM_BUFFER_H__
