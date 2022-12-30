#ifndef H_CONTEXT
#define H_CONTEXT

#include <util/gfx.h>
#include <util/sysdefs.h>

#include <core/api/vulkan/vulkan_device.h>

typedef struct _applicationProperties ApplicationProperties;

struct _applicationProperties
{
    const char *title;
    const char *description;

    struct
    {
        int major : 1;
        int minor : 1;
    } version;
};

GLFWwindow *
createWindow(int winWidth, int winHeight, VulkanDeviceContext **vkd);
// void cleanup(ContextProperties *contextProperties);

#endif // H_CONTEXT
