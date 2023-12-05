/*
 * Copyright (c) 2022-2023, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#include "device_context.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// TODO: Move this
#include <GoodsackEngineConfig.h>

#include "util/gfx.h"
#include "util/logger.h"
#include "util/sysdefs.h"

#include "core/device/device.h"
#include "core/drivers/opengl/opengl.h"
#include "core/drivers/vulkan/vulkan.h"

#include "core/graphics/renderer/pipeline/pass_screen.h"

static void
_error_callback(int error, const char *description)
{
    LOG_ERROR("%s", description);
}

static void
_resize_callback(GLFWwindow *window, int widthRe, int heightRe)
{
    printf("window resize: %d and %d\n", widthRe, heightRe);
    glViewport(0, 0, widthRe, heightRe);
    postbuffer_resize((ui32)widthRe, (ui32)heightRe);
}

static void
_key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    // Quit the application
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }

    // Toggle cursor state
    if (key == GLFW_KEY_F1 && action == GLFW_PRESS) {
        Input deviceInput = device_getInput();
        device_setCursorState(!deviceInput.cursor_state.is_locked,
                              !deviceInput.cursor_state.is_visible);
    }

    // 1.) Send the key to the input device stack
    // 2.) (From UPDATE) - GetKey() checks the stack for actions and inputs
    // 3.) EOF(Endofframe)
}

static void
_mouse_callback(GLFWwindow *window, int button, int action, int mods)
{
    // Nothing here for now!
}

static void
_cursor_callback(GLFWwindow *window, double xpos, double ypos)
{
    // send input coords to device container
    Input input              = device_getInput();
    input.cursor_position[0] = xpos;
    input.cursor_position[1] = ypos;

    device_setInput(input);
}

GLFWwindow *
createWindow(int winWidth, int winHeight, VulkanDeviceContext **vkd)
{

    glfwSetErrorCallback(_error_callback);

    if (!glfwInit()) { // Initialization failed
        printf("Failed to initialize glfw");
    }

    // OpenGL
    if (DEVICE_API_OPENGL) {
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        // debug ALL OpenGL Errors
        glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, SYS_DEBUG);

        char title[256];
        sprintf(title,
                "Goodsack Engine | %d.%d.%d.%d\n",
                GOODSACK_VERSION_MAJOR,
                GOODSACK_VERSION_MINOR,
                GOODSACK_VERSION_PATCH,
                GOODSACK_VERSION_TWEAK);

        GLFWwindow *window =
          glfwCreateWindow(winWidth, winHeight, title, NULL, NULL);

        if (!window) LOG_ERROR("Failed to create window");

        // Set the context and load GL [Note: different for Vk]
        glfwMakeContextCurrent(window);
        gladLoadGL(glfwGetProcAddress);

        glfwGetFramebufferSize(window, &winWidth, &winHeight);
        glfwSetFramebufferSizeCallback(window, _resize_callback);
        glfwSetKeyCallback(window, _key_callback);
        glfwSetCursorPosCallback(window, _cursor_callback);
        glfwSetMouseButtonCallback(window, _mouse_callback);

        // Initialize GL debug callback
        glDebugInit();
        // Get current OpenGL version
        LOG_INFO("%s\n", glGetString(GL_VERSION));

        unsigned char pixels[16 * 16 * 4];
        memset(pixels, 0xff, sizeof(pixels));

        GLFWimage *image = malloc(sizeof(GLFWimage));
        image->width     = 16;
        image->height    = 16;
        image->pixels    = pixels;

        GLFWcursor *cursor = glfwCreateCursor(image, 0, 0);
        if (cursor != NULL) { glfwSetCursor(window, cursor); }

        return window;
    }

    // Vulkan
    else if (DEVICE_API_VULKAN) {
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

        GLFWwindow *window =
          glfwCreateWindow(winWidth, winHeight, "Title", NULL, NULL);

        glfwSetKeyCallback(window, _key_callback);

        VulkanDeviceContext *vulkanDevice = vulkan_device_create();
        *vkd                              = vulkanDevice;

        if (glfwCreateWindowSurface(vulkanDevice->vulkanInstance,
                                    window,
                                    NULL,
                                    &vulkanDevice->surface) != VK_SUCCESS) {
            LOG_ERROR("failed to create window surface!");
        }

        vulkanDevice->swapChainDetails =
          vulkan_swapchain_create(vulkanDevice->device,
                                  vulkanDevice->physicalDevice,
                                  vulkanDevice->surface,
                                  window);

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
            vulkanDevice->pipelineDetails->renderPass);

        vulkan_render_setup(vulkanDevice);
        vulkan_context_create_sync(vulkanDevice);

        return window;
    }
    LOG_ERROR("Failed to create window. Graphics API not specified!");
    return NULL;
}
