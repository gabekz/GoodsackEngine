/*
 * Copyright (c) 2022-2023, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#ifndef __VULKAN_DEBUG_H__
#define __VULKAN_DEBUG_H__

#include "util/gfx.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

VKAPI_ATTR VkBool32 VKAPI_CALL
vulkan_debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                      VkDebugUtilsMessageTypeFlagsEXT messageType,
                      const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
                      void *pUserData);

VkResult
createDebugUtilsMessengerEXT(
  VkInstance instance,
  const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
  const VkAllocationCallbacks *pAllocator,
  VkDebugUtilsMessengerEXT *pDebugMessenger);

void
vulkan_debug_messenger_create(VkInstance instance,
                              VkDebugUtilsMessengerEXT debugMessenger);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __VULKAN_DEBUG_H__
