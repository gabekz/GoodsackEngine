#include "device.h"

#include <util/sysdefs.h>

static volatile GraphicsAPI s_device = GRAPHICS_API_VULKAN;
;
static volatile int s_initialized = 0; // false

static struct
{
    ui32 counter;
    double prevTime, timeDiff;
    Analytics analytics;
} s_ald; // Analytic Data

GraphicsAPI
device_getGraphics()
{
    return s_device;
}

void
device_setGraphics(GraphicsAPI api)
{
    s_device = api;
}

// Analytics

Analytics
device_getAnalytics()
{
    return s_ald.analytics;
}

void
device_resetAnalytics()
{
    s_ald.counter   = 0;
    s_ald.prevTime  = 0;
    s_ald.timeDiff  = 0;
    s_ald.analytics = (Analytics) {0, 0};
}

void
device_updateAnalytics(double time)
{
    s_ald.timeDiff = time - s_ald.prevTime;
    s_ald.counter++;

    if (s_ald.timeDiff >= 1.0 / 30.0) {
        s_ald.analytics =
          (Analytics) {.currentFps = (1.0 / s_ald.timeDiff) * s_ald.counter,
                       .currentMs  = (s_ald.timeDiff / s_ald.counter) * 1000};
        s_ald.prevTime = time;
        s_ald.counter  = 0;
    }
}
