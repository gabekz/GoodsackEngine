#include "device.h"

#include <util/sysdefs.h>

static volatile GraphicsAPI s_device = GRAPHICS_API_VULKAN;
static volatile GraphicsSettings s_deviceSettings;
;
static volatile int s_initialized = 0; // false

static struct
{
    ui32 counter;
    double prevTime, timeDiff;
    Analytics analytics;
} s_ald; // Analytic Data

// API

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

// Settings

GraphicsSettings
device_getGraphicsSettings()
{
    return s_deviceSettings;
}

void
device_setGraphicsSettings(GraphicsSettings settings)
{
    s_deviceSettings = settings;
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
    s_ald.counter  = 0;
    s_ald.prevTime = 0;
    s_ald.timeDiff = 0;

    s_ald.analytics.currentFps = 0;
    s_ald.analytics.currentMs  = 0;
    s_ald.analytics.delta      = 0;
    s_ald.analytics.lastFrame  = 0;
}

void
device_updateAnalytics(double time)
{
    // Delta Time
    s_ald.analytics.delta     = time - s_ald.analytics.lastFrame;
    s_ald.analytics.lastFrame = time;

    // Analytical Time
    s_ald.timeDiff = time - s_ald.prevTime;
    s_ald.counter++;

    if (s_ald.timeDiff >= 1.0 / 30.0) {

        s_ald.analytics.currentFps = (1.0 / s_ald.timeDiff) * s_ald.counter;
        s_ald.analytics.currentMs  = (s_ald.timeDiff / s_ald.counter) * 1000;

        // Reset
        s_ald.prevTime = time;
        s_ald.counter  = 0;
    }
}
