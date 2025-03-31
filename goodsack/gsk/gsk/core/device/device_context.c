/*
 * Copyright (c) 2022-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#include "device_context.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// TODO: Move this
#include <GoodsackEngineConfig.h>

#include "stb_image.h"

#include "core/graphics/texture/texture.h"
#include "util/gfx.h"
#include "util/logger.h"
#include "util/sysdefs.h"

#include "core/device/device.h"
#include "core/drivers/opengl/opengl.h"
#include "core/drivers/vulkan/vulkan.h"

#include "core/graphics/renderer/v1/renderer.h"
#include "runtime/gsk_runtime_wrapper.h"

static void
_error_callback(int error, const char *description)
{
    LOG_ERROR("%s", description);
}

static void
_resize_callback(GLFWwindow *window, int new_width, int new_height)
{
    if ((new_width > 0 && new_height > 0) == FALSE) { return; }

    gsk_Renderer *p_renderer = gsk_runtime_get_renderer();
    gsk_renderer_resize(p_renderer, new_width, new_height);
}

static void
_key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    // Quit the application
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }

#if SYS_DEBUG
    // Toggle cursor state
    if (key == GLFW_KEY_F1 && action == GLFW_PRESS)
    {
        gsk_Input deviceInput = gsk_device_getInput();
        device_setCursorState(!deviceInput.cursor_state.is_locked,
                              !deviceInput.cursor_state.is_visible);
    }
#endif

    // 1.) Send the key to the input device stack
    // 2.) (From UPDATE) - GetKey() checks the stack for actions and inputs
    // 3.) EOF(Endofframe)
}

static void
_mouse_callback(GLFWwindow *window, int button, int action, int mods)
{
#if SYS_DEBUG
    gsk_Input deviceInput = gsk_device_getInput();
    if (deviceInput.cursor_state.is_visible == FALSE) { return; }

    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    {
        gsk_runtime_set_debug_entity_id(gsk_runtime_get_hovered_entity_id());
    }
#endif // SYS_DEBUG
}

static void
_cursor_callback(GLFWwindow *window, double xpos, double ypos)
{
    // send input coords to device container
    gsk_Input input          = gsk_device_getInput();
    input.cursor_position[0] = xpos;
    input.cursor_position[1] = ypos;

    gsk_device_setInput(input);
}

GLFWwindow *
gsk_window_create(int win_width,
                  int win_height,
                  const char *win_image_path,
                  const char *win_app_title,
                  VulkanDeviceContext **vkd)
{

    u8 is_fullscreen = FALSE;

    glfwSetErrorCallback(_error_callback);

    if (!glfwInit()) { LOG_CRITICAL("Failed to initialize glfw"); }

    // OpenGL
    if (GSK_DEVICE_API_OPENGL)
    {
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        // debug ALL OpenGL Errors
        glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, SYS_DEBUG);

        // glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);

        char title[256];
        sprintf(title,
                "%s | Goodsack Engine %d.%d.%d.%d\n",
                win_app_title,
                GOODSACK_VERSION_MAJOR,
                GOODSACK_VERSION_MINOR,
                GOODSACK_VERSION_PATCH,
                GOODSACK_VERSION_TWEAK);
        // sprintf(title, win_app_title);

        const GLFWmonitor *monitor = glfwGetPrimaryMonitor();

// windowed-fullscreen hints
#if 1
        const GLFWvidmode *mode = glfwGetVideoMode(monitor);

        glfwWindowHint(GLFW_RED_BITS, mode->redBits);
        glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
        glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
        glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);

        // glfwWindowHint(GLFW_AUTO_ICONIFY, GLFW_FALSE);
        // glfwWindowHint(GLFW_FLOATING, GLFW_FALSE);

#endif

        GLFWwindow *window = glfwCreateWindow(
          win_width, win_height, title, (is_fullscreen) ? monitor : NULL, NULL);

        if (!window) { LOG_CRITICAL("Failed to create window"); }

        // glfwSetWindowPos(window, 500, 0);

        // load image
        GLFWimage *image_win = malloc(sizeof(GLFWimage));
        image_win->pixels    = stbi_load(
          win_image_path, &image_win->width, &image_win->height, 0, 4);
        if (image_win->pixels)
        {
            glfwSetWindowIcon(window, 1, image_win);
            stbi_image_free(image_win->pixels);
        }

        // Set the context and load GL [Note: different for Vk]
        glfwMakeContextCurrent(window);
        gladLoadGL(glfwGetProcAddress);

        glfwGetFramebufferSize(window, &win_width, &win_height);

        glfwSetFramebufferSizeCallback(window, _resize_callback);
        glfwSetKeyCallback(window, _key_callback);
        glfwSetCursorPosCallback(window, _cursor_callback);
        glfwSetMouseButtonCallback(window, _mouse_callback);

        // Initialize GL debug callback
        _gsk_gl_debug_init();

        unsigned char pixels[16 * 16 * 4];
        memset(pixels, 0xff, sizeof(pixels));

        GLFWimage *image = malloc(sizeof(GLFWimage));
        image->width     = 16;
        image->height    = 16;
        image->pixels    = pixels;

        GLFWcursor *cursor = glfwCreateCursor(image, 0, 0);
        if (cursor != NULL) { glfwSetCursor(window, cursor); }

        // OpenGL Info
        {
            int max_tex = 0, max_coords = 0, max_size = 0;
            glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &max_tex);
            glGetIntegerv(GL_MAX_TEXTURE_COORDS, &max_coords);
            glGetIntegerv(GL_MAX_TEXTURE_SIZE, &max_size);

            LOG_INFO("---- OpenGL Info\n%s\nmax_tex: %d\nmax_coords: "
                     "%d\nmax_size: %dpx\n----",
                     glGetString(GL_VERSION),
                     max_tex,
                     max_coords,
                     max_size);
        }

        return window;
    }

    // Vulkan
    else if (GSK_DEVICE_API_VULKAN)
    {
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

        GLFWwindow *window =
          glfwCreateWindow(win_width, win_height, "Title", NULL, NULL);

        glfwSetKeyCallback(window, _key_callback);
        glfwSetCursorPosCallback(window, _cursor_callback);
        glfwSetMouseButtonCallback(window, _mouse_callback);

        VulkanDeviceContext *vulkanDevice = vulkan_device_create();
        *vkd                              = vulkanDevice;

        if (glfwCreateWindowSurface(vulkanDevice->vulkanInstance,
                                    window,
                                    NULL,
                                    &vulkanDevice->surface) != VK_SUCCESS)
        {
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
