#include "context.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <util/gfx.h>
#include <util/debug.h>
#include <util/logging.h>

#include <util/sysdefs.h>

#include <core/api/vulkan/vulkan_device.h>


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

GLFWwindow* createWindow(int winWidth, int winHeight) {

   glfwSetErrorCallback(_error_callback);

   if(!glfwInit()) { // Initialization failed
        printf("Failed to initialize glfw");
   }

   /*
   ApplicationProperties props = {
       .title = "Test",
       .description = "Test",
       .version = {
           .major = 0,
           .minor = 3,
       },
   };
   */

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

   VulkanDeviceContext *vulkanDevice = vulkan_device_create();

if (glfwCreateWindowSurface(*vulkanDevice->vulkanInstance, window, NULL, &vulkanDevice->surface) != VK_SUCCESS) {
        LOG_ERROR("failed to create window surface!");
    }


#endif /* USING_VULKAN */

   return window;

}

