#include "device_api.h"

#include <util/sysdefs.h>

static volatile GraphicsAPI s_deviceapi = GRAPHICS_API_VULKAN;;
static volatile int s_initialized = 0; // false

GraphicsAPI device_api_getGraphics() {
    return s_deviceapi;
}

void device_api_setGraphics(GraphicsAPI api) {
    s_deviceapi = api;
}
