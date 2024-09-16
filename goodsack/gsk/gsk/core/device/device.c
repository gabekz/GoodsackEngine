/*
 * Copyright (c) 2022-2023, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#include "device.h"

#include "util/gfx.h"
#include "util/logger.h"
#include "util/sysdefs.h"

static volatile gsk_GraphicsAPI s_graphics_api = GRAPHICS_API_OPENGL;
static volatile gsk_GraphicsSettings s_device_settings;
static volatile gsk_Input s_input;
;
static volatile int s_initialized = 0; // false

static struct
{
    u32 counter;
    f64 prevTime;
    f64 clock_metrics, clock_fixed_delta;
    f64 time_elapsed;
    gsk_Time time;
} s_device;

// API

gsk_GraphicsAPI
gsk_device_getGraphics()
{
    return s_graphics_api;
}

void
gsk_device_setGraphics(gsk_GraphicsAPI api)
{
    s_graphics_api = api;
}

// Settings

gsk_GraphicsSettings
gsk_device_getGraphicsSettings()
{
    return s_device_settings;
}

void
gsk_device_setGraphicsSettings(gsk_GraphicsSettings settings)
{
    s_device_settings = settings;
}

// Time

gsk_Time
gsk_device_getTime()
{
    return s_device.time;
}

void
gsk_device_resetTime()
{
    s_device.counter  = 0;
    s_device.prevTime = 0;

    s_device.clock_metrics     = 0;
    s_device.clock_fixed_delta = 0;

    // Track performance metrics
    s_device.time.metrics.last_fps = 0;
    s_device.time.metrics.last_ms  = 0;

    // Shared timing
    s_device.time.delta_time       = 0;
    s_device.time.fixed_delta_time = GSK_TIME_FIXED_DELTA_DEFAULT;
    s_device.time.time_scale       = GSK_TIME_SCALE_DEFAULT;
    s_device.time.next_time_scale  = GSK_TIME_SCALE_DEFAULT;
}

void
gsk_device_setTimescale(f32 timescale)
{
    if (timescale < 0)
    {
        LOG_ERROR("Cannot set timescale lower than 0. Requsted: %f", timescale);
    }
    LOG_DEBUG("Time scale set to %f", timescale);
    s_device.time.next_time_scale = timescale;
}

void
gsk_device_updateTime(double time)
{
    // update timescale
    s_device.time.time_scale = s_device.time.next_time_scale;

    // time *= s_device.time.time_scale;
    const f64 analytics_interval_ms = 1000;

    // Delta Time
    s_device.time.delta_time = time - s_device.time_elapsed;
    s_device.time_elapsed    = time;

    // Analytical Time
    s_device.clock_metrics = time - s_device.prevTime;
    s_device.counter++; // total frames since last interval

    // update metrics based on interval
    if (s_device.clock_metrics >= analytics_interval_ms / 1000)
    {

        s_device.time.metrics.last_fps =
          (1.0 / s_device.clock_metrics) * s_device.counter;

        s_device.time.metrics.last_ms =
          (s_device.clock_metrics / s_device.counter) * 1000;

        // Reset Timer
        s_device.prevTime = time;
        s_device.counter  = 0;
    }
}

u8
_gsk_device_check_fixed_update(double time)
{
    f64 clock_check = time - s_device.clock_fixed_delta;
    if (clock_check >= s_device.time.fixed_delta_time) { return 1; }
    return 0;
}

void
_gsk_device_reset_fixed_update(double time)
{
    s_device.clock_fixed_delta = time;
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
