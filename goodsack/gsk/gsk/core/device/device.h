/*
 * Copyright (c) 2022-2023, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#ifndef __DEVICE_H__
#define __DEVICE_H__

#include "util/gfx.h"
#include "util/sysdefs.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

// API

#define GSK_DEVICE_API_OPENGL gsk_device_getGraphics() == GRAPHICS_API_OPENGL
#define GSK_DEVICE_API_VULKAN gsk_device_getGraphics() == GRAPHICS_API_VULKAN

typedef enum { GRAPHICS_API_OPENGL, GRAPHICS_API_VULKAN } gsk_GraphicsAPI;

// Settings

typedef struct gsk_GraphicsSettings
{
    int swapInterval; // VSync
} gsk_GraphicsSettings;

// Analytics

typedef struct gsk_Analytics
{
    double currentFps;
    double currentMs;

    double delta;
    double lastFrame;

    // u32 currentDrawCalls;
} gsk_Analytics;

// typedef enum { CURSOR_LOCK_MODE_NONE, CURSOR_LOCK_MODE_LOCKED }
// CursorLockMode;

typedef struct gsk_Input
{
    // Mouse Cursor
    double cursor_position[2];
    double cursor_axis_raw[2];

    struct
    {
        int is_locked;
        int is_visible;
    } cursor_state;

} gsk_Input;

// Functions

gsk_GraphicsAPI
gsk_device_getGraphics();
void
gsk_device_setGraphics(gsk_GraphicsAPI api);

gsk_GraphicsSettings
gsk_device_getGraphicsSettings();

void
gsk_device_setGraphicsSettings(gsk_GraphicsSettings settings);

gsk_Analytics
gsk_device_getAnalytics();

void
gsk_device_resetAnalytics();

void
gsk_device_updateAnalytics(double time);

gsk_Input
gsk_device_getInput();

void
gsk_device_setInput(gsk_Input input);

void
device_setCursorState(int is_locked, int is_visible);

void
device_updateCursorState(GLFWwindow *window);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __DEVICE_H__
