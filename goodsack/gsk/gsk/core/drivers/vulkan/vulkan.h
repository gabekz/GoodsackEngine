/*
 * Copyright (c) 2022-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

/* Vulkan implementation - headers */

#ifndef __VULKAN_H__
#define __VULKAN_H__

// Helpers + Defs
#include "core/drivers/vulkan/vulkan_support.h"

#include "core/drivers/vulkan/vulkan_command.h"
#include "core/drivers/vulkan/vulkan_descriptor.h"
#include "core/drivers/vulkan/vulkan_device.h"

#include "core/drivers/vulkan/vulkan_pipeline.h"
#include "core/drivers/vulkan/vulkan_render.h"
#include "core/drivers/vulkan/vulkan_swapchain.h"

// Buffers
#include "core/drivers/vulkan/vulkan_buffer.h"
#include "core/drivers/vulkan/vulkan_uniform_buffer.h"
#include "core/drivers/vulkan/vulkan_vertex_buffer.h"

#include "core/drivers/vulkan/vulkan_image.h"

#include "core/drivers/vulkan/vulkan_framebuffer.h"

#endif // __VULKAN_H__