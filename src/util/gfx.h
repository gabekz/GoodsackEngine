#ifndef H_GFX
#define H_GFX

#include <util/sysdefs.h> // TODO: Move def to build-system

// Only include this once as well
#ifndef GLFW_INCLUDE_NONE
 #define GLFW_INCLUDE_NONE
#endif

#if defined(SYS_API_VULKAN)
 #define GLFW_INCLUDE_VULKAN
 #include <vulkan/vulkan.h>
 #include <vulkan/vk_sdk_platform.h>
 //#include <vulkan/vulkan_xcb.h>
#endif

#include<glad/gl.h> // TODO: Move to #defined
#include<GLFW/glfw3.h>
#include<GLFW/glfw3native.h>

static inline void clearGLState() {
    glUseProgram(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
}

#endif // H_GFX
