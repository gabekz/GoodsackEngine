#ifndef H_CONTEXT
#define H_CONTEXT

#include <util/gfx.h>
#include <util/sysdefs.h>

#include <vulkan/vulkan.h>

typedef struct _applicationProperties ApplicationProperties;

struct _applicationProperties {
    const char *title;
    const char *description;

    struct {
        int major: 1;
        int minor: 1;
    } version;

};

GLFWwindow* createWindow(int winWidth, int winHeight);
//void cleanup(ContextProperties *contextProperties);

#endif // H_CONTEXT
