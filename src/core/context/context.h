#ifndef H_CONTEXT
#define H_CONTEXT

#include <util/gfx.h>
#include <util/sysdefs.h>

#include <vulkan/vulkan.h>

typedef struct _contextProperties{
   VkInstance *vulkanInstance; 
} ContextProperties;

GLFWwindow* createWindow(int winWidth, int winHeight);
void cleanup(ContextProperties *contextProperties);

#endif // H_CONTEXT
