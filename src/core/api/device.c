#include "device.h"

#include <util/sysdefs.h>

static volatile GraphicsAPI s_device = GRAPHICS_API_VULKAN;;
static volatile int s_initialized = 0; // false

GraphicsAPI device_getGraphics() {
    return s_device;
}

void device_setGraphics(GraphicsAPI api) {
    s_device = api;
}
