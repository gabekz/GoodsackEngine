#include "context.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <util/gfx.h>
#include <util/debug.h>
#include <util/logger.h>

#include <util/sysdefs.h>

#include <core/api/device.h>
#include <core/api/vulkan/vulkan.h>

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

GLFWwindow* createWindow(int winWidth, int winHeight, VulkanDeviceContext **vkd) {

    glfwSetErrorCallback(_error_callback);

    if(!glfwInit()) { // Initialization failed
        printf("Failed to initialize glfw");
    }

// OpenGL
    if(DEVICE_API_OPENGL) {
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        // debug ALL OpenGL Errors
        glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, SYS_DEBUG);

        GLFWwindow* window =
        glfwCreateWindow(winWidth, winHeight, "Title", NULL, NULL);

        if(!window) LOG_ERROR("Failed to create window");

        // Set the context and load GL [Note: different for Vk]
        glfwMakeContextCurrent(window);
        gladLoadGL(glfwGetProcAddress);

        glfwGetFramebufferSize(window, &winWidth, &winHeight);
        glfwSetFramebufferSizeCallback(window, _resize_callback);
        glfwSetKeyCallback(window, _key_callback);

         // Initialize GL debug callback
         glDebugInit();
        // Get current OpenGL version
        LOG_INFO("%s\n", glGetString(GL_VERSION));
        // Refresh rate 
        glfwSwapInterval(1);

        return window;
    }

// Vulkan
    else if(DEVICE_API_VULKAN) {
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

        GLFWwindow* window =
          glfwCreateWindow(winWidth, winHeight, "Title", NULL, NULL);

        glfwSetKeyCallback(window, _key_callback);

        VulkanDeviceContext *vulkanDevice = vulkan_device_create();
        *vkd = vulkanDevice;

        if (glfwCreateWindowSurface(vulkanDevice->vulkanInstance, window, NULL, &vulkanDevice->surface) != VK_SUCCESS) {
                LOG_ERROR("failed to create window surface!");
        }

        vulkanDevice->swapChainDetails = vulkan_swapchain_create(
            vulkanDevice->device, vulkanDevice->physicalDevice,
            vulkanDevice->surface, window);

        vulkanDevice->pipelineDetails = vulkan_pipeline_create(
            vulkanDevice->physicalDevice,
            vulkanDevice->device,
            vulkanDevice->swapChainDetails->swapchainImageFormat,
            vulkanDevice->swapChainDetails->swapchainExtent);

        vulkanDevice->depthResources = vulkan_depth_create_resources(
            vulkanDevice->physicalDevice,
            vulkanDevice->device,
            vulkanDevice->swapChainDetails->swapchainExtent);

        vulkanDevice->swapChainDetails->swapchainFramebuffers = 
            vulkan_framebuffer_create(
                vulkanDevice->device,
                vulkanDevice->swapChainDetails->swapchainImageCount,
                vulkanDevice->swapChainDetails->swapchainImageViews,
                vulkanDevice->depthResources->depthImageView,
                vulkanDevice->swapChainDetails->swapchainExtent,
                vulkanDevice->pipelineDetails->renderPass
            );

        //vulkan_context_create_framebuffers(vulkanDevice);
        vulkan_context_create_command_pool(vulkanDevice);
        vulkan_context_create_sync(vulkanDevice);

        return window;
    }
    LOG_ERROR("Failed to create window. Graphics API not specified!");
    return NULL;
}
