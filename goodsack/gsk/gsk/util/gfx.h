/*
 * Copyright (c) 2022-2023, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#ifndef __GFX_H__
#define __GFX_H__

// Only include this once as well
#ifndef GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_NONE
#endif

// #include <vulkan/vk_sdk_platform.h>
#include <vulkan/vulkan.h>
#define GLFW_INCLUDE_VULKAN

#include <GLFW/glfw3.h>
#include <glad/gl.h>

// Default Graphics API
#if defined(SYS_API_OPENGL) || defined(SYS_API_VULKAN)
#else
// #define SYS_API_OPENGL
#define SYS_API_VULKAN
#endif

// Default Window Resolution
#define DEFAULT_WINDOW_WIDTH  1280
#define DEFAULT_WINDOW_HEIGHT 720

// Default Render Resolution
#define DEFAULT_RENDER_WIDTH  DEFAULT_WINDOW_WIDTH
#define DEFAULT_RENDER_HEIGHT DEFAULT_WINDOW_HEIGHT

#define DRAWING_MODE GL_TRIANGLES // TODO: Compatibility

#define DEFAULT_CLEAR_COLOR    \
    {                          \
        0.1f, 0.1f, 0.8f, 1.0f \
    }

// Drawing modes
#define GPU_DRAW_MODE_ARRAYS   0xD000
#define GPU_DRAW_MODE_ELEMENTS 0xD001

#define SHADOW_WIDTH  4096
#define SHADOW_HEIGHT 4096

static inline void
clearGLState()
{
    glUseProgram(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
}

#endif // __GFX_H__
