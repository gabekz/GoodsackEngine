#include "vulkan_device.h"

#define VK_USE_PLATFORM_XCB_KHR
#define GLFW_EXPOSE_NATIVE_XCB

#include <util/sysdefs.h>

#include <stdlib.h>
#include <string.h>

#include <util/gfx.h>
#include <util/debug.h>
#include <util/logger.h>

#include <core/api/vulkan/vulkan_support.h>
#include <core/api/vulkan/vulkan_swapchain.h>
#include <core/api/vulkan/vulkan_vertex_buffer.h>

#include <model/primitives.h>

/* static */

static ui32 _findQueueFamilies(VkPhysicalDevice device) {
    ui32 graphicsFamily;
    ui32 queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, NULL);

    VkQueueFamilyProperties queueFamilies[queueFamilyCount];
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies);

    for(int i = 0; i < queueFamilyCount; i++) {
        if(queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            graphicsFamily = i;
        }
    }

    return graphicsFamily;
}

static int _checkValidationLayerSupport(const char *validationLayers[], ui32 count) {
    ui32 layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, NULL);

    VkLayerProperties availableLayers[layerCount];

    if(vkEnumerateInstanceLayerProperties(&layerCount, availableLayers) != VK_SUCCESS) {
        LOG_ERROR("Failed to enumerate instanced layer properties!");
    }

#if 0 // Print all available layers
    LOG_INFO("Available Validation Layers (%d): ", layerCount);
    for(int i = 0; i < layerCount; i++) {
        LOG_INFO("\t%s", availableLayers[i].layerName);
    }
#endif

    for(int i = 0; i < count; i++) {
        int layerFound = 0;
#if 0
            LOG_INFO("Checking validation layer %s", validationLayers[i]);
#endif

        for(int j = 0; j < layerCount; j++) {
            if(strcmp(validationLayers[i], availableLayers[j].layerName) == 0) {
                layerFound = 1;
                LOG_INFO("Validation Layer found: %s", validationLayers[i]);
                break;
            }
        }

        if(!layerFound) {
            LOG_WARN("Validation layer UNAVAILABLE: %s", validationLayers[i]);
            return 0;
        }
    }

    return 1;
}

static int _checkDeviceExtensionSupport(const char *extensions[], ui32 count, VkPhysicalDevice device) {
    ui32 extensionsCount;
    vkEnumerateDeviceExtensionProperties(device, NULL, &extensionsCount, NULL);

    VkExtensionProperties availableExtensions[extensionsCount];
    vkEnumerateDeviceExtensionProperties(
        device, NULL, &extensionsCount, availableExtensions);

#if 0 // Print all available device extensions
    LOG_INFO("Available Device Extensions (%d): ", extensionsCount);
    for(int i = 0; i < extensionsCount ; i++) {
        LOG_INFO("\t%s", availableExtensions[i].extensionName);
    }
#endif

    for(int i = 0; i < count; i++) {
        int extensionFound = 0;
        for(int j = 0; j < extensionsCount; j++) {
            if(strcmp(extensions[i], availableExtensions[j].extensionName) == 0) {
                extensionFound = 1;
                LOG_INFO("Device Extension found: %s", extensions[i]);
                break;
            }
        }

        if(!extensionFound)
            LOG_WARN("Device Extension UNAVAILABLE: %s", extensions[i]);
            return 0;
    }

    return 1;
}

static int _isDeviceSuitable(VkPhysicalDevice device) {
    // TODO: Device-ranking support

    // Swapchain support
    // const char *validationLayers[VK_REQ_VALIDATION_SIZE] = VK_REQ_VALIDATION_LIST;
    const char *deviceExtensions[VK_REQ_DEVICE_EXT_SIZE] = VK_REQ_DEVICE_EXT;

    ui32 indices = _findQueueFamilies(device);
    int extensionsSupported = _checkDeviceExtensionSupport(deviceExtensions, 1, device);

    int swapChainAdequate = 0;
    if(extensionsSupported) {
        //swapChainAdequate = _querySwapChainSupport(device);

    }

    return 1;
}


static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData) {

    switch(messageSeverity) {
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
        LOG_WARN("[Validation Layer] %s", pCallbackData->pMessage);
        break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
        default:
        LOG_CRITICAL("[Validation Layer] %s", pCallbackData->pMessage);
    }


    return VK_FALSE;
}

static VkResult createDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
    PFN_vkCreateDebugUtilsMessengerEXT p =
        (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
        instance, "vkCreateDebugUtilsMessengerEXT");
    if(p == NULL) {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
    return p(instance, pCreateInfo, pAllocator, pDebugMessenger);
}

static void createMessenger(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger) {
    VkDebugUtilsMessengerCreateInfoEXT createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = debugCallback;
    createInfo.pUserData = NULL; // Optional
    
    if(createDebugUtilsMessengerEXT(instance, &createInfo, NULL, &debugMessenger) != VK_SUCCESS) {
        LOG_ERROR("Failed to create debug utils messenger!");
    }
}

/* implement */

VulkanDeviceContext *vulkan_device_create() {
// Vulkan Application Info
    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Application Name";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "Below Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

// Vulkan Instance Information
    VkInstanceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

// Validation Layer + Extension Handling for Instance
    const unsigned char kEnableValidationLayers = 1;

    ui32 glfwExtensionCount = 0;
    const char **glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    //vkEnumerateInstanceExtensionProperties(NULL, &extensionCount, NULL);
    //printf("\n%d extensions supported", extensionCount);
    
    const char *extensionTest[3] = {
        "VK_KHR_surface",
        "VK_KHR_xcb_surface",
        VK_EXT_DEBUG_UTILS_EXTENSION_NAME
    };

    const char *validationLayers[VK_REQ_VALIDATION_SIZE] = 
        VK_REQ_VALIDATION_LIST;

    if(kEnableValidationLayers) {
        if(!_checkValidationLayerSupport(validationLayers, 1)) {
            LOG_ERROR("Validation layer requested, but not available!");
        }
        createInfo.ppEnabledLayerNames = validationLayers;
        createInfo.enabledLayerCount = VK_REQ_VALIDATION_SIZE;

        createInfo.enabledExtensionCount = 3;
        createInfo.ppEnabledExtensionNames = extensionTest;
    }
    else {
        createInfo.enabledLayerCount = 0;
        createInfo.enabledExtensionCount = glfwExtensionCount;
        createInfo.ppEnabledExtensionNames = glfwExtensions;
    }

    // Create Instance
    VulkanDeviceContext *ret = malloc(sizeof(VulkanDeviceContext));
    VkResult result = vkCreateInstance(&createInfo, NULL, &ret->vulkanInstance);
    if (vkCreateInstance(&createInfo, NULL, &ret->vulkanInstance) != VK_SUCCESS) {
        printf("Failed to initialize Vulkan Instance!");
        return NULL;
    }
    else {
        printf("Successfully created Vulkan Instance!");
    }

// Create Debug Messenger
    if(kEnableValidationLayers) {
        createMessenger(ret->vulkanInstance, ret->debugMessenger);
    }

// Physical Device
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    ui32 deviceCount = 0;
    vkEnumeratePhysicalDevices(ret->vulkanInstance, &deviceCount, NULL);

    if(deviceCount == 0) {
        printf("failed to find GPUs with Vulkan support!");
    }

    VkPhysicalDevice devices[15];
    vkEnumeratePhysicalDevices(ret->vulkanInstance, &deviceCount, devices);

    // TODO: This does not list all devices before picking suitable
    for(int i = 0; i < deviceCount; i++) {
        VkPhysicalDeviceProperties deviceProperties;
        vkGetPhysicalDeviceProperties(devices[i], &deviceProperties);
        LOG_INFO("Device Found: %s", deviceProperties.deviceName);

        if(_isDeviceSuitable(devices[i])) {
        LOG_INFO("Picked Suitable Device: %s", deviceProperties.deviceName);
            physicalDevice = devices[i];
            break;
        }
    }
    if(physicalDevice == VK_NULL_HANDLE) {
        LOG_ERROR("Failed to find a Suitable GPU!");
    }

// Create Logical Device
    ui32 graphicsFamily = _findQueueFamilies(physicalDevice);
    float queuePriority = 1.0f;

    VkDeviceQueueCreateInfo queueCreateInfo = {};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = graphicsFamily;
    queueCreateInfo.queueCount = 1;
    queueCreateInfo.pQueuePriorities = &queuePriority;

    VkPhysicalDeviceFeatures deviceFeatures = {};

    VkDeviceCreateInfo logicCreateInfo = {};
    logicCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    logicCreateInfo.pQueueCreateInfos = &queueCreateInfo;
    logicCreateInfo.queueCreateInfoCount = 1;
    logicCreateInfo.pEnabledFeatures = &deviceFeatures;

    // Device Extensions
    const char *extensions[VK_REQ_DEVICE_EXT_SIZE] = VK_REQ_DEVICE_EXT;
    logicCreateInfo.enabledExtensionCount = VK_REQ_DEVICE_EXT_SIZE;
    logicCreateInfo.ppEnabledExtensionNames = extensions;

    if(vkCreateDevice(
      physicalDevice, &logicCreateInfo, NULL, &ret->device) != VK_SUCCESS) {
        LOG_ERROR("Failed to create Logical Device!");
    }
    ret->physicalDevice = physicalDevice;

    vkGetDeviceQueue(ret->device, graphicsFamily, 0, &ret->graphicsQueue);

    return ret;
}

static void _recordCommandBuffer(VulkanDeviceContext *context, ui32 imageIndex) {

    VkCommandBufferBeginInfo beginInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = 0, // Optional
        .pInheritanceInfo = NULL, // Optional
    };
    
    if (vkBeginCommandBuffer(context->commandBuffer, &beginInfo) != VK_SUCCESS) {
        LOG_ERROR("Failed to begin recording command buffer!");
    }

    VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};

    VkRenderPassBeginInfo renderPassInfo = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .renderPass = context->pipelineDetails->renderPass,
        .framebuffer = context->swapChainDetails->swapchainFramebuffers[imageIndex],
        .renderArea.offset = {0, 0},
        .renderArea.extent = context->swapChainDetails->swapchainExtent,

        .clearValueCount = 1,
        .pClearValues = &clearColor
    };

    vkCmdBeginRenderPass(context->commandBuffer,
        &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(context->commandBuffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        context->pipelineDetails->graphicsPipeline);

    // Set viewports and scissors (again?)
    VkViewport viewport = {
        .x = 0.0f,
        .y = 0.0f,
        .width = (float)context->swapChainDetails->swapchainExtent.width,
        .height = (float)context->swapChainDetails->swapchainExtent.height,
        .minDepth = 0.0f,
        .maxDepth = 1.0f
    };
    vkCmdSetViewport(context->commandBuffer, 0, 1, &viewport);

    VkRect2D scissor = {
        .offset = {0, 0},
        .extent = context->swapChainDetails->swapchainExtent
    };
    vkCmdSetScissor(context->commandBuffer, 0, 1, &scissor);

#if 0
    vkCmdDraw(context->commandBuffer, 3, 1, 0, 0);
#else

    LOG_DEBUG("Binding Vertex Buffer");
    VkBuffer vertexBuffers[] = {context->vertexBuffer->buffer};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(context->commandBuffer, 0, 1, vertexBuffers, offsets);

    LOG_DEBUG("Drawing Vertex Buffer");
    vkCmdDraw(context->commandBuffer, context->vertexBuffer->size, 1, 0, 0);
#endif

    vkCmdEndRenderPass(context->commandBuffer);

    if (vkEndCommandBuffer(context->commandBuffer) != VK_SUCCESS) {
        LOG_ERROR("Failed to record command buffer!");
    }
}

void vulkan_context_create_framebuffers(VulkanDeviceContext *context) {

    VkFramebuffer *swapchainFramebuffers = malloc(sizeof(VkFramebuffer) *
            context->swapChainDetails->swapchainImageCount);

    for(int i = 0; i < context->swapChainDetails->swapchainImageCount; i++) {
        VkImageView attachments[] = {
        context->swapChainDetails->swapchainImageViews[i]};

        VkFramebufferCreateInfo framebufferInfo = {
            .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            .renderPass = context->pipelineDetails->renderPass,
            .attachmentCount = 1,
            .pAttachments = attachments,
            .width = context->swapChainDetails->swapchainExtent.width,
            .height = context->swapChainDetails->swapchainExtent.height,
            .layers = 1
        };

        if (vkCreateFramebuffer(context->device, &framebufferInfo, NULL, &swapchainFramebuffers[i]) != VK_SUCCESS) {
            LOG_ERROR("failed to create framebuffer!");
        }
    }

    context->swapChainDetails->swapchainFramebuffers = swapchainFramebuffers;

}

void vulkan_context_create_command_pool(VulkanDeviceContext *context) {

    ui32 indices = _findQueueFamilies(context->physicalDevice);

// Command Pool
    VkCommandPoolCreateInfo poolInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = indices
    };

    if (vkCreateCommandPool(context->device, &poolInfo, NULL, &context->commandPool) != VK_SUCCESS) {
        LOG_ERROR("Failed to create command pool!");
    }

// Create a VERTEX BUFFER

    float *vertices = PRIM_ARR_TEST;
    int size = PRIM_SIZ_TEST * sizeof(float);

    LOG_DEBUG("Create vertex buffer");
    VulkanVertexBuffer *vb = 
        vulkan_vertex_buffer_create(
                context->physicalDevice,
                context->device, 
                vertices,
                size);

    context->vertexBuffer = vb;

// Command Buffer
    VkCommandBufferAllocateInfo allocInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = context->commandPool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1
    };

    if (vkAllocateCommandBuffers(context->device, &allocInfo, &context->commandBuffer) != VK_SUCCESS) {
        LOG_ERROR("Failed to allocate command buffers!");
    }
}

void vulkan_context_create_sync(VulkanDeviceContext *context) {

    VkSemaphoreCreateInfo semaphoreInfo = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO
    };

    VkFenceCreateInfo fenceInfo = {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .flags = VK_FENCE_CREATE_SIGNALED_BIT
    };

if (vkCreateSemaphore(context->device, &semaphoreInfo, NULL,
    &context->imageAvailableSemaphore) != VK_SUCCESS ||
    vkCreateSemaphore(context->device, &semaphoreInfo, NULL,
    &context->renderFinishedSemaphore) != VK_SUCCESS ||
    vkCreateFence(context->device, &fenceInfo, NULL,
    &context->inFlightFence) != VK_SUCCESS) {
        LOG_ERROR("failed to create semaphores!");
    }
}

void vulkan_drawFrame(VulkanDeviceContext *context) {
    if(vkWaitForFences(context->device, 1,
                &context->inFlightFence, VK_TRUE, UINT64_MAX) != VK_SUCCESS) {
        LOG_ERROR("vkWaitForFences Failed!");
    }

    vkResetFences(context->device, 1, &context->inFlightFence);

    ui32 imageIndex;
    VkResult result = vkAcquireNextImageKHR(context->device,
            context->swapChainDetails->swapchain, UINT64_MAX,
            context->imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);

    if(result == VK_ERROR_OUT_OF_DATE_KHR) {
        LOG_WARN("Cannot acquire next image. Try recreating the swapchain?");
    }
    else if(result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        LOG_ERROR("Failed to acquire next image!");
    }

    vkResetCommandBuffer(context->commandBuffer, 0);
    _recordCommandBuffer(context, imageIndex);

    VkSemaphore waitSemaphores[] = {context->imageAvailableSemaphore};
    VkSemaphore signalSemaphores[] = {context->renderFinishedSemaphore};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

    VkSubmitInfo submitInfo = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = waitSemaphores,
        .pWaitDstStageMask = waitStages,

        .commandBufferCount = 1,
        .pCommandBuffers = &context->commandBuffer,

        .signalSemaphoreCount = 1,
        .pSignalSemaphores = signalSemaphores
    };

    if (vkQueueSubmit(context->graphicsQueue, 1, &submitInfo, context->inFlightFence) != VK_SUCCESS) {
        LOG_ERROR("Failed to submit draw command buffer!");
    }

    VkSwapchainKHR swapChains[] = {context->swapChainDetails->swapchain};

    VkPresentInfoKHR presentInfo = {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,

        .waitSemaphoreCount = 1,
        .pWaitSemaphores = signalSemaphores,

        .swapchainCount = 1,
        .pSwapchains = swapChains,
        .pImageIndices = &imageIndex,

        .pResults = NULL // Optional
    };

    vkQueuePresentKHR(context->graphicsQueue, &presentInfo);
}

void vulkan_device_cleanup(VulkanDeviceContext* context) {

    // Cleanup image views
    for(int i = 0; i < context->swapChainDetails->swapchainImageCount; i++) {
        vkDestroyImageView(context->device, context->swapChainDetails->swapchainImageViews[i], NULL);
        vkDestroyFramebuffer(context->device, context->swapChainDetails->swapchainFramebuffers[i], NULL);
    }

    vkDestroySemaphore(context->device, context->imageAvailableSemaphore, NULL);
    vkDestroySemaphore(context->device, context->renderFinishedSemaphore, NULL);
    vkDestroyFence(context->device, context->inFlightFence, NULL);

    vkDestroyCommandPool(context->device, context->commandPool, NULL);

    vkDestroyPipeline(context->device, context->pipelineDetails->graphicsPipeline, NULL);
    vkDestroyPipelineLayout(context->device, context->pipelineDetails->pipelineLayout, NULL);
    vkDestroyRenderPass(context->device, context->pipelineDetails->renderPass, NULL);

    vkDestroyDevice(context->device, NULL);
    vkDestroySurfaceKHR(context->vulkanInstance, context->surface, NULL);
    //DestroyDebugUtilsMessengerEXT(context->instance, context->debugMessenger, NULL); // TODO: if enabled validation
    vkDestroyInstance(context->vulkanInstance, NULL);

    //free(context->vulkanInstance);
    //free(context->device);
    free(context);
}
