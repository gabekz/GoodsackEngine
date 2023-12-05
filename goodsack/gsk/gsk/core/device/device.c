/*
 * Copyright (c) 2022-2023, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#include "device.h"

#include "util/gfx.h"
#include "util/sysdefs.h"

static volatile gsk_GraphicsAPI s_device = GRAPHICS_API_OPENGL;
static volatile gsk_GraphicsSettings s_deviceSettings;
static volatile gsk_Input s_input;
;
static volatile int s_initialized = 0; // false

static struct
{
    u32 counter;
    double prevTime, timeDiff;
    gsk_Analytics analytics;
} s_ald; // Analytic Data

// API

gsk_GraphicsAPI
gsk_device_getGraphics()
{
    return s_device;
}

void
gsk_device_setGraphics(gsk_GraphicsAPI api)
{
    s_device = api;
}

// Settings

gsk_GraphicsSettings
gsk_device_getGraphicsSettings()
{
    return s_deviceSettings;
}

void
gsk_device_setGraphicsSettings(gsk_GraphicsSettings settings)
{
    s_deviceSettings = settings;
}

// Analytics

gsk_Analytics
gsk_device_getAnalytics()
{
    return s_ald.analytics;
}

void
gsk_device_resetAnalytics()
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
gsk_device_updateAnalytics(double time)
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

gsk_Input
gsk_device_getInput()
{
    return s_input;
}

void
device_setCursorState(int is_locked, int is_visible)
{
    s_input.cursor_state.is_locked  = is_locked;
    s_input.cursor_state.is_visible = is_visible;
}

void
device_updateCursorState(GLFWwindow *window)
{
    glfwSetInputMode(window,
                     GLFW_CURSOR,
                     (s_input.cursor_state.is_visible) ? GLFW_CURSOR_NORMAL
                                                       : GLFW_CURSOR_DISABLED);
}

void
gsk_device_setInput(gsk_Input input)
{
    double lastX = s_input.cursor_position[0];
    double lastY = s_input.cursor_position[1];

    double crntX = input.cursor_position[0];
    double crntY = input.cursor_position[1];

    if (lastX > crntX)
        s_input.cursor_axis_raw[0] = -1;
    else if (lastX < crntX)
        s_input.cursor_axis_raw[0] = 1;
    else
        s_input.cursor_axis_raw[0] = 0;

    if (lastY > crntY)
        s_input.cursor_axis_raw[1] = 1;
    else if (lastY < crntY)
        s_input.cursor_axis_raw[1] = -1;
    else
        s_input.cursor_axis_raw[1] = 0;

    // store results
    s_input.cursor_position[0] = crntX;
    s_input.cursor_position[1] = crntY;

    // s_input.holding_right_button = input.holding_right_button;
    //_update_cursor_state();
}
