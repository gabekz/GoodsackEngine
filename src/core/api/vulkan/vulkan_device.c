#include "vulkan_device.h"

#define VK_USE_PLATFORM_XCB_KHR
#define GLFW_INCLUDE_VULKAN
#define GLFW_EXPOSE_NATIVE_XCB

#include <xcb/xcb.h>
#define VK_KHR_xcb_surface
#include <xcb/xcb.h>

#include <stdlib.h>
#include <string.h>

#include <util/gfx.h>
#include <util/debug.h>
#include <util/logging.h>

#include <util/sysdefs.h>

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_xcb.h>
#include <vulkan/vk_sdk_platform.h>


/* static */

static int _checkValidationLayerSupport(const char *validationLayers[], ui32 count) {
    ui32 layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, NULL);
    VkLayerProperties availableLayers[layerCount];

    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers);

    for(int i = 0; i < count; i++) {
        int layerFound = 0;

        for(int j = 0; j < count; j++) {
            if(strcmp(validationLayers[i], availableLayers[j].layerName) == 0) {
                layerFound = 1;
                break;
            }
        }

        if(!layerFound) {
            return 0;
        }
    }

    return 1;
}

static int _isDeviceSuitable(VkPhysicalDevice device) {
    // TODO: Device-ranking
    return 1;
}

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

    ui32 glfwExtensionCount = 0;
    const char **glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    //vkEnumerateInstanceExtensionProperties(NULL, &extensionCount, NULL);
    //printf("\n%d extensions supported", extensionCount);
    
    createInfo.enabledExtensionCount = glfwExtensionCount;
    createInfo.ppEnabledExtensionNames = glfwExtensions;
    createInfo.enabledLayerCount = 0;

// New Vulkan Instance
    VulkanDeviceContext *ret = malloc(sizeof(VulkanDeviceContext));
    ret->vulkanInstance = malloc(sizeof(VkInstance));
    ret->device = malloc(sizeof(VkDevice));
    VkResult result = vkCreateInstance(&createInfo, NULL, ret->vulkanInstance);
    if (vkCreateInstance(&createInfo, NULL, ret->vulkanInstance) != VK_SUCCESS) {
        printf("Failed to initialize Vulkan Instance!");
        return NULL;
    }
    else {
        printf("Successfully created Vulkan Instance!");
    }

// Validation Layer Handling
    const char *validationLayers[3] = {
        "VK_LAYERS_KHRONOS_validation",
        "VK_EXT_DEBUG_UTILS_EXTENSION_NAME",
        "VK_KHR_XCB_SURFACE_EXTENSION_NAME"
    };
    const unsigned char kEnableValidationLayers = 1;

    if(kEnableValidationLayers) {
        if(_checkValidationLayerSupport(validationLayers, 3)) {
            LOG_ERROR("Validation layer requested, but not available!");
        }
        createInfo.enabledLayerCount = 3;
        createInfo.ppEnabledLayerNames = validationLayers;
    }

// Validate Physical Device
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    ui32 deviceCount = 0;
    vkEnumeratePhysicalDevices(*ret->vulkanInstance, &deviceCount, NULL);

    if(deviceCount == 0) {
        printf("failed to find GPUs with Vulkan support!");
    }

    VkPhysicalDevice devices[15];
    vkEnumeratePhysicalDevices(*ret->vulkanInstance, &deviceCount, devices);

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
    logicCreateInfo.enabledExtensionCount = 0;

    if(vkCreateDevice(
      physicalDevice, &logicCreateInfo, NULL, ret->device) != VK_SUCCESS) {
        LOG_ERROR("Failed to create Logical Device!");
    }

    vkGetDeviceQueue(*ret->device, graphicsFamily, 0, &ret->graphicsQueue);

    return ret;
}

void vulkan_device_surface(VulkanDeviceContext* context) {

#if defined(SYS_ENV_WIN)
    VkWin32SurfaceCreateInfoKHR createInfo {};

#elif defined(SYS_ENV_UNIX)
    //VkXcbSurfaceCreateInfoKHR surfaceCreateInfo = {};
    VkXcbSurfaceCreateInfoKHR surfaceCreateInfo = {};
    surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR;
    surfaceCreateInfo.pNext = NULL;
    surfaceCreateInfo.flags = 0;
    //surfaceCreateInfo.window = window;
    //surfaceCreateInfo.connection = connection;
    //surfaceCreateInfo.window = window;

    /*
    VkSurfaceKHR surface;
    if(vkCreateXcbSurfaceKHR(instance, &surfaceCreateInfo, NULL, &surface)
      != VK_SUCCESS) {
        LOG_ERROR("Failed to produce XCB Surface!");

    }
assert(result == VK_SUCCESS);
*/

#endif

}

void cleanup(VulkanDeviceContext* context) {
    vkDestroyInstance(*context->vulkanInstance, NULL);
    vkDestroyDevice(*context->device, NULL);

    free(context->vulkanInstance);
    free(context->device);
    free(context);
}
