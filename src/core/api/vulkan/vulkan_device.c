#include "vulkan_device.h"

#define VK_USE_PLATFORM_XCB_KHR
#define GLFW_EXPOSE_NATIVE_XCB

#define TEST_RENDER_PRIMITIVE   0   // 0 - Model Loader | 1 - Primitive
#define TEST_RENDER_MODE        0   // 0 - VkCmdDraw    | 1 - VkCmdDrawIndexed

#include <util/sysdefs.h>

#include <stdlib.h>
#include <string.h>

#include <util/gfx.h>
#include <util/debug.h>
#include <util/logger.h>

#include <core/texture/texture.h>

#include <core/api/vulkan/vulkan_command.h>
#include <core/api/vulkan/vulkan_descriptor.h>
#include <core/api/vulkan/vulkan_support.h>
#include <core/api/vulkan/vulkan_swapchain.h>
#include <core/api/vulkan/vulkan_uniform_buffer.h>
#include <core/api/vulkan/vulkan_vertex_buffer.h>

#include <import/loader_obj.h>
#include <model/primitives.h>

/* static */

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

static int _isDeviceSuitable(VkPhysicalDevice physicalDevice) {
    // TODO: Device-ranking support

    // Swapchain support
    // const char *validationLayers[VK_REQ_VALIDATION_SIZE] = VK_REQ_VALIDATION_LIST;
    const char *deviceExtensions[VK_REQ_DEVICE_EXT_SIZE] = VK_REQ_DEVICE_EXT;

    ui32 indices = vulkan_device_find_queue_families(physicalDevice);
    int extensionsSupported = _checkDeviceExtensionSupport(
            deviceExtensions, 1, physicalDevice);

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
    
    VK_CHECK(createDebugUtilsMessengerEXT(instance, &createInfo, NULL, &debugMessenger));
}

/* implement */

ui32 vulkan_device_find_queue_families(VkPhysicalDevice physicalDevice)
{
    ui32 graphicsFamily;
    ui32 queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice,
            &queueFamilyCount, NULL);

    VkQueueFamilyProperties queueFamilies[queueFamilyCount];
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice,
            &queueFamilyCount, queueFamilies);

    for(int i = 0; i < queueFamilyCount; i++) {
        if(queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            graphicsFamily = i;
        }
    }

    return graphicsFamily;
}

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
        LOG_INFO("Successfully created Vulkan Instance!");
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

    VkPhysicalDeviceProperties deviceProperties;

    // TODO: This does not list all devices before picking suitable
    for(int i = 0; i < deviceCount; i++) {
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

    // Store picked physical device properties here
    ret->physicalDeviceProperties = deviceProperties;

// Create Logical Device
    ui32 graphicsFamily = vulkan_device_find_queue_families(physicalDevice);
    float queuePriority = 1.0f;

    VkDeviceQueueCreateInfo queueCreateInfo = {};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = graphicsFamily;
    queueCreateInfo.queueCount = 1;
    queueCreateInfo.pQueuePriorities = &queuePriority;

    VkPhysicalDeviceFeatures deviceFeatures = {};
    deviceFeatures.samplerAnisotropy = VK_TRUE;

    VkDeviceCreateInfo logicCreateInfo = {};
    logicCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    logicCreateInfo.pQueueCreateInfos = &queueCreateInfo;
    logicCreateInfo.queueCreateInfoCount = 1;
    logicCreateInfo.pEnabledFeatures = &deviceFeatures;

    // Device Extensions
    const char *extensions[VK_REQ_DEVICE_EXT_SIZE] = VK_REQ_DEVICE_EXT;
    logicCreateInfo.enabledExtensionCount = VK_REQ_DEVICE_EXT_SIZE;
    logicCreateInfo.ppEnabledExtensionNames = extensions;

    VK_CHECK(vkCreateDevice(
      physicalDevice, &logicCreateInfo, NULL, &ret->device));

    ret->physicalDevice = physicalDevice;
    ret->currentFrame = 0;

    vkGetDeviceQueue(ret->device, graphicsFamily, 0, &ret->graphicsQueue);

    return ret;
}

static void _recordCommandBuffer(VulkanDeviceContext *context, ui32 imageIndex, VkCommandBuffer *commandBuffer) {
    //LOG_DEBUG("RECORDING command buffer");

    VkCommandBufferBeginInfo beginInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = 0, // Optional
        .pInheritanceInfo = NULL, // Optional
    };
    
    if (vkBeginCommandBuffer(*commandBuffer, &beginInfo) != VK_SUCCESS) {
        LOG_ERROR("Failed to begin recording command buffer!");
    }

    VkClearValue clearColor = {{{0.0f, 0.1f, 0.2f, 1.0f}}};
    VkClearValue depthStencil = {1.0f, 0.0f};

    VkClearValue clearValues[] = {
        clearColor,
        depthStencil
    };

    VkRenderPassBeginInfo renderPassInfo = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .renderPass = context->pipelineDetails->renderPass,
        .framebuffer = context->swapChainDetails->swapchainFramebuffers[imageIndex],
        .renderArea.offset = {0, 0},
        .renderArea.extent = context->swapChainDetails->swapchainExtent,

        .clearValueCount = 2,
        .pClearValues = clearValues
    };

    vkCmdBeginRenderPass(*commandBuffer,
        &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(*commandBuffer,
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
    vkCmdSetViewport(*commandBuffer, 0, 1, &viewport);

    VkRect2D scissor = {
        .offset = {0, 0},
        .extent = context->swapChainDetails->swapchainExtent
    };
    vkCmdSetScissor(*commandBuffer, 0, 1, &scissor);

    //LOG_DEBUG("Binding Vertex Buffer");
    VkBuffer vertexBuffers[] = {context->vertexBuffer->buffer};
    VkDeviceSize offsets[] = {0};

    VkBuffer indexBuffer = context->indexBuffer.buffer;

    vkCmdBindVertexBuffers(*commandBuffer, 0, 1, vertexBuffers, offsets);

    vkCmdBindDescriptorSets(*commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
            context->pipelineDetails->pipelineLayout, 0, 1,
            &context->descriptorSets[context->currentFrame], 0, NULL);

#if TEST_RENDER_MODE == 0

    vkCmdDraw(*commandBuffer, context->vertexBuffer->size, 1, 0, 0);

#elif TEST_RENDER_MODE == 1

    vkCmdBindIndexBuffer(*commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT16);
    vkCmdDrawIndexed(*commandBuffer, context->indexBuffer.indicesCount,
            1, 0, 0, 0);
#endif

    vkCmdEndRenderPass(*commandBuffer);

    if (vkEndCommandBuffer(*commandBuffer) != VK_SUCCESS) {
        LOG_ERROR("Failed to record command buffer!");
    }
}

void vulkan_context_create_command_pool(VulkanDeviceContext *context) {
// Create a Command Pool
    LOG_DEBUG("Create Command Pool");
    context->commandPool =
        vulkan_command_pool_create(context->physicalDevice, context->device);
// Create Command Buffers
    LOG_DEBUG("Create command buffers");
    context->commandBuffers = 
        vulkan_command_buffer_create(context->device, context->commandPool);

// Create a VERTEX BUFFER
    LOG_DEBUG("Create vertex buffer");

#if TEST_RENDER_PRIMITIVE == 0

    ModelData *modelDataTest =
        load_obj("../res/models/cerberus-triang.obj", 4.0f);

    float *vertices = modelDataTest->buffers.out;
    int size = modelDataTest->buffers.outI * sizeof(float);

#elif TEST_RENDER_PRIMITIVE == 1

    float *vertices = PRIM_ARR_V_PYRAMID;
    int size = PRIM_SIZ_V_PYRAMID * sizeof(float);

#endif

    VulkanVertexBuffer *vb = 
        vulkan_vertex_buffer_create(
                context->physicalDevice,
                context->device, 
                context->graphicsQueue,
                context->commandPool,
                vertices,
                size);

    context->vertexBuffer = vb;

#if TEST_RENDER_MODE == 1

    ui16 *indices = PRIM_ARR_I_PYRAMID;
    ui32 indicesCount = PRIM_SIZ_I_PYRAMID;

    context->indexBuffer = *vulkan_index_buffer_create(
            context->physicalDevice,
            context->device,
            context->commandPool,
            context->graphicsQueue,
            indices, indicesCount
    );

#endif

// Create UNIFORM BUFFERS
    LOG_DEBUG("Create uniform buffers");
    vulkan_uniform_buffer_create(context->physicalDevice, context->device,
           &context->uniformBuffers, &context->uniformBuffersMemory,
           &context->uniformBuffersMapped);

// Create Descriptor Pool
    LOG_DEBUG("Create descriptor pool");
    vulkan_descriptor_pool_create(context->device, &context->descriptorPool, 
        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);

// Create a texture
    LOG_DEBUG("Create a test texture");
    Texture *texture = 
        texture_create("../res/textures/pbr/cerberus/Cerberus_A.tga",
                0, 0, 0, context);

// Create Descriptor Sets
    LOG_DEBUG("Create descriptor sets");
    vulkan_descriptor_sets_create(context->device, context->descriptorPool,
        &context->descriptorSets, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        context->uniformBuffers, sizeof(UniformBufferObject),
        context->pipelineDetails->descriptorSetLayout,
        texture->vulkan.textureImageView, texture->vulkan.textureSampler);
}

void vulkan_context_create_sync(VulkanDeviceContext *context) {

    context->imageAvailableSemaphores = malloc(
            sizeof(VkSemaphore) * MAX_FRAMES_IN_FLIGHT);
    context->renderFinishedSemaphores = malloc(
            sizeof(VkSemaphore) * MAX_FRAMES_IN_FLIGHT);
    context->inFlightFences = malloc(
            sizeof(VkFence) * MAX_FRAMES_IN_FLIGHT);

    VkSemaphoreCreateInfo semaphoreInfo = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO
    };

    VkFenceCreateInfo fenceInfo = {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .flags = VK_FENCE_CREATE_SIGNALED_BIT
    };

    for(int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        if (vkCreateSemaphore(context->device, &semaphoreInfo, NULL,
            &context->imageAvailableSemaphores[i]) != VK_SUCCESS ||
            vkCreateSemaphore(context->device, &semaphoreInfo, NULL,
            &context->renderFinishedSemaphores[i]) != VK_SUCCESS ||
            vkCreateFence(context->device, &fenceInfo, NULL,
            &context->inFlightFences[i]) != VK_SUCCESS)
        {
                LOG_ERROR(
                    "failed to create synchronization objcets for a frame!");
        }
    }
}

void vulkan_drawFrame(VulkanDeviceContext *context, GLFWwindow *window) {
    VK_CHECK(vkWaitForFences(context->device, 1,
                &context->inFlightFences[context->currentFrame],
                VK_TRUE, UINT64_MAX));

    ui32 imageIndex;
    VkResult result = vkAcquireNextImageKHR(context->device,
            context->swapChainDetails->swapchain, UINT64_MAX,
            context->imageAvailableSemaphores[context->currentFrame],
            VK_NULL_HANDLE, &imageIndex);

    if(result == VK_ERROR_OUT_OF_DATE_KHR) {
        LOG_DEBUG("VK_ERROR_OUT_OF_DATE_KHR - recreating swapchain");
        context->swapChainDetails = vulkan_swapchain_recreate(
                context->physicalDevice,
                context->device,
                context->swapChainDetails,
                context->surface,
                context->pipelineDetails->renderPass,
                &context->depthResources,
                window // TODO: Maybe don't use this??
        );
        return; // must pull-out for requeue
    }
    else if(result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        LOG_ERROR("Failed to acquire next image!");
    }

    // Must be done AFTER we potentially recreate the swapchain.
    // Avoids Fence deadlock.
    VK_CHECK(vkResetFences(context->device, 1,
            &context->inFlightFences[context->currentFrame]));

    vkResetCommandBuffer(context->commandBuffers[context->currentFrame], 0);
    _recordCommandBuffer(context, imageIndex, &context->commandBuffers[context->currentFrame]);

    VkSemaphore waitSemaphores[] = {context->imageAvailableSemaphores[context->currentFrame]};
    VkSemaphore signalSemaphores[] = {context->renderFinishedSemaphores[context->currentFrame]};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

    // Update UBO data
    //LOG_DEBUG("update uniform buffers");
    vulkan_uniform_buffer_update(context->currentFrame,
            context->uniformBuffersMapped, context->swapChainDetails->swapchainExtent);

    VkSubmitInfo submitInfo = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = waitSemaphores,
        .pWaitDstStageMask = waitStages,

        .commandBufferCount = 1,
        .pCommandBuffers = &context->commandBuffers[context->currentFrame],

        .signalSemaphoreCount = 1,
        .pSignalSemaphores = signalSemaphores
    };

    VK_CHECK(vkQueueSubmit(context->graphicsQueue, 1,
                &submitInfo, context->inFlightFences[context->currentFrame]));

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
    context->currentFrame = (context->currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void vulkan_device_cleanup(VulkanDeviceContext* context) {

    vulkan_swapchain_cleanup(context->device, context->swapChainDetails);

    vkDestroyBuffer(context->device, context->vertexBuffer->buffer, NULL);
    vkFreeMemory(context->device, context->vertexBuffer->bufferMemory, NULL);

    for(int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroySemaphore(context->device, context->imageAvailableSemaphores[i], NULL);
        vkDestroySemaphore(context->device, context->renderFinishedSemaphores[i], NULL);
        vkDestroyFence(context->device, context->inFlightFences[i], NULL);

        //free(context->imageAvailableSemaphores);
        //free(context->renderFinishedSemaphores);
        //free(context->inFlightFences);
    }
    vkDestroyCommandPool(context->device, context->commandPool, NULL);

    vkDestroyShaderModule(context->device, context->pipelineDetails->vertShaderModule, NULL);
    vkDestroyShaderModule(context->device, context->pipelineDetails->fragShaderModule, NULL);

    vkDestroyPipeline(context->device, context->pipelineDetails->graphicsPipeline, NULL);
    vkDestroyPipelineLayout(context->device, context->pipelineDetails->pipelineLayout, NULL);
    vkDestroyRenderPass(context->device, context->pipelineDetails->renderPass, NULL);

    vkDestroyDevice(context->device, NULL);
    vkDestroySurfaceKHR(context->vulkanInstance, context->surface, NULL);
    //DestroyDebugUtilsMessengerEXT(context->instance, context->debugMessenger, NULL); // TODO: if enabled validation
    vkDestroyInstance(context->vulkanInstance, NULL);

    free(context);
    free(context->swapChainDetails);
    free(context->pipelineDetails);
}
