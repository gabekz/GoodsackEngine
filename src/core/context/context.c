#include "context.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <util/gfx.h>
#include <util/debug.h>
#include <util/logging.h>

#include <util/sysdefs.h>

#include <vulkan/vulkan.h>

#ifdef USING_VULKAN
#define GLFW_INCLUDE_VULKAN
#endif

static void _resize_callback
(GLFWwindow* window, int widthRe, int heightRe) {
    printf("window resize: %d and %d\n", widthRe, heightRe);
    glViewport(0, 0, widthRe, heightRe);
}

static void _key_callback 
(GLFWwindow* window, int key, int scancode, int action, int mods) {
   if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
      glfwSetWindowShouldClose(window, GLFW_TRUE);
   }
}

static int checkValidationLayerSupport(const char *validationLayers[], ui32 count) {
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

static int isDeviceSuitable(VkPhysicalDevice device) {
    // TODO: Device-ranking
    return 1;
}


GLFWwindow* createWindow(int winWidth, int winHeight) {

   glfwSetErrorCallback(_error_callback);

   if(!glfwInit()) { // Initialization failed
        printf("Failed to initialize glfw");
   }

// OpenGL
#ifndef USING_VULKAN
   // Minimum OpenGL version required
   glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
   glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
   glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
   // debug ALL OpenGL Errors
   glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, SYS_DEBUG);

   GLFWwindow* window =
      glfwCreateWindow(winWidth, winHeight, "Title", NULL, NULL);

   if(!window) printf("Failed to create window");

   // Set the context and load GL [Note: different for Vk]
   glfwMakeContextCurrent(window);
   gladLoadGL(glfwGetProcAddress);

   glfwGetFramebufferSize(window, &winWidth, &winHeight);
   glfwSetFramebufferSizeCallback(window, _resize_callback);
   glfwSetKeyCallback(window, _key_callback);

    // Initialize GL debug callback
    glDebugInit();
   // Get current OpenGL version
   printf("%s\n", glGetString(GL_VERSION));
   // Refresh rate 
   glfwSwapInterval(1);

// Vulkan
#else
   glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
   glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
   GLFWwindow* window =
      glfwCreateWindow(winWidth, winHeight, "Title", NULL, NULL);

   if(!window) printf("Failed to create window");

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
    ContextProperties *props = malloc(sizeof(ContextProperties));
    props->vulkanInstance = malloc(sizeof(VkInstance));
    VkResult result = vkCreateInstance(&createInfo, NULL, props->vulkanInstance);
    if (vkCreateInstance(&createInfo, NULL, props->vulkanInstance) != VK_SUCCESS) {
        printf("Failed to initialize Vulkan Instance!");
        return NULL;
    }
    else {
        printf("Successfully created Vulkan Instance!");
    }

// Validation Layer Handling
    const char *validationLayers[2] = {
        "VK_LAYERS_KHRONOS_validation",
        "VK_EXT_DEBUG_UTILS_EXTENSION_NAME"
    };
    const unsigned char kEnableValidationLayers = 1;

    if(kEnableValidationLayers) {
        if(checkValidationLayerSupport(validationLayers, 2)) {
            LOG_ERROR("Validation layer requested, but not available!");
        }
        createInfo.enabledLayerCount = 2;
        createInfo.ppEnabledLayerNames = validationLayers;
    }

// Validate Physical Device
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    ui32 deviceCount = 0;
    vkEnumeratePhysicalDevices(*props->vulkanInstance, &deviceCount, NULL);

    if(deviceCount == 0) {
        printf("failed to find GPUs with Vulkan support!");
    }

    VkPhysicalDevice devices[15];
    vkEnumeratePhysicalDevices(*props->vulkanInstance, &deviceCount, devices);

    for(int i = 0; i < deviceCount; i++) {
        VkPhysicalDeviceProperties deviceProperties;
        vkGetPhysicalDeviceProperties(devices[i], &deviceProperties);
        LOG_INFO("Device Found: %s", deviceProperties.deviceName);

        if(isDeviceSuitable(devices[i])) {
        LOG_INFO("Picked Suitable Device: %s", deviceProperties.deviceName);
            physicalDevice = devices[i];
            break;
        }
    }
    if(physicalDevice == VK_NULL_HANDLE) {
        LOG_ERROR("Failed to fidn a Suitable GPU!");
    }

#endif /* USING_VULKAN */

   return window;

}

void cleanup(ContextProperties *contextProperties) {
    vkDestroyInstance(*contextProperties->vulkanInstance, NULL);
    free(contextProperties->vulkanInstance);
    free(contextProperties);
}
