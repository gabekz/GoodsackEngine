#ifndef H_DEVICE_API
#define H_DEVICE_API

#include <util/gfx.h>
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

    double delta;
    double lastFrame;

    // ui32 currentDrawCalls;
} Analytics;

// typedef enum { CURSOR_LOCK_MODE_NONE, CURSOR_LOCK_MODE_LOCKED }
// CursorLockMode;

typedef struct Input
{
    // Mouse Cursor
    double cursor_position[2];
    double cursor_axis_raw[2];

    struct
    {
        int is_locked;
        int is_visible;
    } cursor_state;

} Input;

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

Input
device_getInput();
void
device_setInput(Input input);

void
device_setCursorState(int is_locked, int is_visible);

void
device_updateCursorState(GLFWwindow *window);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // H_DEVICE_API
