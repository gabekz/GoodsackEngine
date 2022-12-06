#include "vulkan_device.h"

#define VK_USE_PLATFORM_XCB_KHR
#define GLFW_EXPOSE_NATIVE_XCB

#include <util/sysdefs.h>

#include <stdlib.h>
#include <string.h>

#include <util/gfx.h>
#include <util/debug.h>
#include <util/logging.h>

#include <core/api/vulkan/vulkan_support.h>
#include <core/api/vulkan/vulkan_swapchain.h>


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

    LOG_ERROR("Validation layer: %s", pCallbackData->pMessage);

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


void vulkan_device_cleanup(VulkanDeviceContext* context) {

    // Cleanup image views
    for(int i = 0; i < context->swapChainDetails->swapchainImageCount; i++) {
        vkDestroyImageView(context->device, context->swapChainDetails->swapchainImageViews[i], NULL);
    }

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
