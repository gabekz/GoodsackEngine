#include "vulkan_device.h"

#pragma warning disable C2057
#pragma warning disable C2133

#define VK_USE_PLATFORM_XCB_KHR
#define GLFW_EXPOSE_NATIVE_XCB

#include <util/sysdefs.h>

#include <stdlib.h>
#include <string.h>

#include <util/gfx.h>
#include <util/logger.h>

#include <core/texture/texture.h>

#include <core/api/vulkan/vulkan_command.h>
#include <core/api/vulkan/vulkan_debug.h>
#include <core/api/vulkan/vulkan_descriptor.h>
#include <core/api/vulkan/vulkan_support.h>
#include <core/api/vulkan/vulkan_swapchain.h>
#include <core/api/vulkan/vulkan_vertex_buffer.h>

#include <asset/import/loader_obj.h>
#include <model/primitives.h>

/* static */

static int
_checkValidationLayerSupport(const char *validationLayers[], ui32 count)
{
    ui32 layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, NULL);

    VkLayerProperties *availableLayers = malloc(sizeof(VkLayerProperties) * layerCount);

    if (vkEnumerateInstanceLayerProperties(&layerCount, availableLayers) !=
        VK_SUCCESS) {
        LOG_ERROR("Failed to enumerate instanced layer properties!");
    }

#if 0 // Print all available layers
    LOG_INFO("Available Validation Layers (%d): ", layerCount);
    for(int i = 0; i < layerCount; i++) {
        LOG_INFO("\t%s", availableLayers[i].layerName);
    }
#endif

    for (int i = 0; i < count; i++) {
        int layerFound = 0;
#if 0
            LOG_INFO("Checking validation layer %s", validationLayers[i]);
#endif

        for (int j = 0; j < layerCount; j++) {
            if (strcmp(validationLayers[i], availableLayers[j].layerName) ==
                0) {
                layerFound = 1;
                LOG_INFO("Validation Layer found: %s", validationLayers[i]);
                break;
            }
        }

        if (!layerFound) {
            LOG_WARN("Validation layer UNAVAILABLE: %s", validationLayers[i]);
            return 0;
        }
    }

    return 1;
}

static int
_checkDeviceExtensionSupport(const char *extensions[],
                             ui32 count,
                             VkPhysicalDevice device)
{
    ui32 extensionsCount;
    vkEnumerateDeviceExtensionProperties(device, NULL, &extensionsCount, NULL);

    VkExtensionProperties availableExtensions[1];
    vkEnumerateDeviceExtensionProperties(
      device, NULL, &extensionsCount, availableExtensions);

#if 0 // Print all available device extensions
    LOG_INFO("Available Device Extensions (%d): ", extensionsCount);
    for(int i = 0; i < extensionsCount ; i++) {
        LOG_INFO("\t%s", availableExtensions[i].extensionName);
    }
#endif

    for (int i = 0; i < count; i++) {
        int extensionFound = 0;
        for (int j = 0; j < extensionsCount; j++) {
            if (strcmp(extensions[i], availableExtensions[j].extensionName) ==
                0) {
                extensionFound = 1;
                LOG_INFO("Device Extension found: %s", extensions[i]);
                break;
            }
        }

        if (!extensionFound)
            LOG_WARN("Device Extension UNAVAILABLE: %s", extensions[i]);
        return 0;
    }

    return 1;
}

static int
_isDeviceSuitable(VkPhysicalDevice physicalDevice)
{
    // TODO: Device-ranking support

    // Swapchain support
    // const char *validationLayers[VK_REQ_VALIDATION_SIZE] =
    // VK_REQ_VALIDATION_LIST;
    const char *deviceExtensions[VK_REQ_DEVICE_EXT_SIZE] = VK_REQ_DEVICE_EXT;

    ui32 indices = vulkan_device_find_queue_families(physicalDevice);
    int extensionsSupported =
      _checkDeviceExtensionSupport(deviceExtensions, 1, physicalDevice);

    int swapChainAdequate = 0;
    if (extensionsSupported) {
        // swapChainAdequate = _querySwapChainSupport(device);
    }

    return 1;
}

/* implement */

ui32
vulkan_device_find_queue_families(VkPhysicalDevice physicalDevice)
{
    ui32 graphicsFamily;
    ui32 queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(
      physicalDevice, &queueFamilyCount, NULL);

    VkQueueFamilyProperties queueFamilies[1];
    vkGetPhysicalDeviceQueueFamilyProperties(
      physicalDevice, &queueFamilyCount, queueFamilies);

    for (int i = 0; i < queueFamilyCount; i++) {
        if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            graphicsFamily = i;
        }
    }

    return graphicsFamily;
}

VulkanDeviceContext *
vulkan_device_create()
{
    // Vulkan Application Info
    VkApplicationInfo appInfo = {
        .sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName   = "Application Name",
        .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
        .pEngineName        = "Below Engine",
        .engineVersion      = VK_MAKE_VERSION(1, 0, 0),
        .apiVersion         = VK_API_VERSION_1_0,
    };

    // Vulkan Instance Information
    VkInstanceCreateInfo createInfo = {
        .sType                = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pApplicationInfo     = &appInfo,
    };

    // Validation Layer + Extension Handling for Instance
    const unsigned char kEnableValidationLayers = 1;

    ui32 glfwExtensionCount = 0;
    const char **glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    // vkEnumerateInstanceExtensionProperties(NULL, &extensionCount, NULL);
    // printf("\n%d extensions supported", extensionCount);

    const char *extensionTest[2] = {"VK_KHR_surface",
                                    VK_EXT_DEBUG_UTILS_EXTENSION_NAME};

    const char *validationLayers[VK_REQ_VALIDATION_SIZE] =
      VK_REQ_VALIDATION_LIST;

    if (kEnableValidationLayers) {
        if (!_checkValidationLayerSupport(validationLayers, 1)) {
            LOG_ERROR("Validation layer requested, but not available!");
        }
        createInfo.ppEnabledLayerNames = validationLayers;
        createInfo.enabledLayerCount   = VK_REQ_VALIDATION_SIZE;

        createInfo.enabledExtensionCount   = 2;
        createInfo.ppEnabledExtensionNames = extensionTest;
    } else {
        createInfo.enabledLayerCount       = 0;
        createInfo.enabledExtensionCount   = glfwExtensionCount;
        createInfo.ppEnabledExtensionNames = glfwExtensions;
    }

    // Create Instance
    VulkanDeviceContext *ret = malloc(sizeof(VulkanDeviceContext));
    VkResult result = vkCreateInstance(&createInfo, NULL, &ret->vulkanInstance);
    if (vkCreateInstance(&createInfo, NULL, &ret->vulkanInstance) !=
        VK_SUCCESS) {
        printf("Failed to initialize Vulkan Instance!");
        return NULL;
    } else {
        LOG_INFO("Successfully created Vulkan Instance!");
    }

    // Create Debug Messenger
    if (kEnableValidationLayers) {
        vulkan_debug_messenger_create(ret->vulkanInstance, ret->debugMessenger);
    }

    // Physical Device
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    ui32 deviceCount                = 0;
    vkEnumeratePhysicalDevices(ret->vulkanInstance, &deviceCount, NULL);

    if (deviceCount == 0) {
        printf("failed to find GPUs with Vulkan support!");
    }

    VkPhysicalDevice devices[15];
    vkEnumeratePhysicalDevices(ret->vulkanInstance, &deviceCount, devices);

    VkPhysicalDeviceProperties deviceProperties;

    // TODO: This does not list all devices before picking suitable
    for (int i = 0; i < deviceCount; i++) {
        vkGetPhysicalDeviceProperties(devices[i], &deviceProperties);
        LOG_INFO("Device Found: %s", deviceProperties.deviceName);

        if (_isDeviceSuitable(devices[i])) {
            LOG_INFO("Picked Suitable Device: %s", deviceProperties.deviceName);
            physicalDevice = devices[i];
            break;
        }
    }
    if (physicalDevice == VK_NULL_HANDLE) {
        LOG_ERROR("Failed to find a Suitable GPU!");
    }

    // Store picked physical device properties here
    ret->physicalDeviceProperties = deviceProperties;

    // Create Logical Device
    ui32 graphicsFamily = vulkan_device_find_queue_families(physicalDevice);
    float queuePriority = 1.0f;

    VkDeviceQueueCreateInfo queueCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        .queueFamilyIndex = graphicsFamily,
        .queueCount       = 1,
        .pQueuePriorities = &queuePriority,
    };

    VkPhysicalDeviceFeatures deviceFeatures = {
        .samplerAnisotropy        = VK_TRUE,
    };

    VkDeviceCreateInfo logicCreateInfo   = {
        .sType                = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pQueueCreateInfos    = &queueCreateInfo,
        .queueCreateInfoCount = 1,
        .pEnabledFeatures     = &deviceFeatures,
    };

    // Device Extensions
    const char *extensions[VK_REQ_DEVICE_EXT_SIZE] = VK_REQ_DEVICE_EXT;
    logicCreateInfo.enabledExtensionCount          = VK_REQ_DEVICE_EXT_SIZE;
    logicCreateInfo.ppEnabledExtensionNames        = extensions;

    VK_CHECK(
      vkCreateDevice(physicalDevice, &logicCreateInfo, NULL, &ret->device));

    ret->physicalDevice = physicalDevice;
    ret->currentFrame   = 0;

    vkGetDeviceQueue(ret->device, graphicsFamily, 0, &ret->graphicsQueue);

    return ret;
}

void
vulkan_context_create_sync(VulkanDeviceContext *context)
{

    context->imageAvailableSemaphores =
      malloc(sizeof(VkSemaphore) * MAX_FRAMES_IN_FLIGHT);
    context->renderFinishedSemaphores =
      malloc(sizeof(VkSemaphore) * MAX_FRAMES_IN_FLIGHT);
    context->inFlightFences = malloc(sizeof(VkFence) * MAX_FRAMES_IN_FLIGHT);

    VkSemaphoreCreateInfo semaphoreInfo = {
      .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};

    VkFenceCreateInfo fenceInfo = {.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
                                   .flags = VK_FENCE_CREATE_SIGNALED_BIT};

    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        if (vkCreateSemaphore(context->device,
                              &semaphoreInfo,
                              NULL,
                              &context->imageAvailableSemaphores[i]) !=
              VK_SUCCESS ||
            vkCreateSemaphore(context->device,
                              &semaphoreInfo,
                              NULL,
                              &context->renderFinishedSemaphores[i]) !=
              VK_SUCCESS ||
            vkCreateFence(
              context->device, &fenceInfo, NULL, &context->inFlightFences[i]) !=
              VK_SUCCESS) {
            LOG_ERROR("failed to create synchronization objcets for a frame!");
        }
    }
}

void
vulkan_device_cleanup(VulkanDeviceContext *context)
{

    vulkan_swapchain_cleanup(context->device, context->swapChainDetails);

    // vkDestroyBuffer(context->device, context->vertexBuffer->buffer, NULL);
    // vkFreeMemory(context->device, context->vertexBuffer->bufferMemory, NULL);

    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroySemaphore(
          context->device, context->imageAvailableSemaphores[i], NULL);
        vkDestroySemaphore(
          context->device, context->renderFinishedSemaphores[i], NULL);
        vkDestroyFence(context->device, context->inFlightFences[i], NULL);

        // free(context->imageAvailableSemaphores);
        // free(context->renderFinishedSemaphores);
        // free(context->inFlightFences);
    }
    vkDestroyCommandPool(context->device, context->commandPool, NULL);

    vkDestroyShaderModule(
      context->device, context->pipelineDetails->vertShaderModule, NULL);
    vkDestroyShaderModule(
      context->device, context->pipelineDetails->fragShaderModule, NULL);

    vkDestroyPipeline(
      context->device, context->pipelineDetails->graphicsPipeline, NULL);
    vkDestroyPipelineLayout(
      context->device, context->pipelineDetails->pipelineLayout, NULL);
    vkDestroyRenderPass(
      context->device, context->pipelineDetails->renderPass, NULL);

    vkDestroyDevice(context->device, NULL);
    vkDestroySurfaceKHR(context->vulkanInstance, context->surface, NULL);
    // DestroyDebugUtilsMessengerEXT(context->instance, context->debugMessenger,
    // NULL); // TODO: if enabled validation
    vkDestroyInstance(context->vulkanInstance, NULL);

    free(context);
    free(context->swapChainDetails);
    free(context->pipelineDetails);
}
