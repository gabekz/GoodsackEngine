#ifndef H_DEVICE_API
#define H_DEVICE_API

#include <util/sysdefs.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

// API

#define DEVICE_API_OPENGL device_getGraphics() == GRAPHICS_API_OPENGL
#define DEVICE_API_VULKAN device_getGraphics() == GRAPHICS_API_VULKAN

typedef enum { GRAPHICS_API_OPENGL, GRAPHICS_API_VULKAN } GraphicsAPI;

// Settings

typedef struct GraphicsSettings
{
    int swapInterval; // VSync
} GraphicsSettings;

// Analytics

typedef struct Analytics
{
    double currentFps;
    double currentMs;

    float delta;
    float lastFrame;

    // ui32 currentDrawCalls;
} Analytics;

// Functions

GraphicsAPI
device_getGraphics();
void
device_setGraphics(GraphicsAPI api);

GraphicsSettings
device_getGraphicsSettings();
void
device_setGraphicsSettings(GraphicsSettings settings);

Analytics
device_getAnalytics();
void
device_resetAnalytics();
void
device_updateAnalytics(double time);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // H_DEVICE_API
