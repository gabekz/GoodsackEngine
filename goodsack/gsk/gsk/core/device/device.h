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

#define GSK_TIME_FIXED_DELTA_DEFAULT 1.0 / 100.0
#define GSK_TIME_SCALE_DEFAULT       1.0

typedef enum { GRAPHICS_API_OPENGL, GRAPHICS_API_VULKAN } gsk_GraphicsAPI;

// Settings

typedef struct gsk_GraphicsSettings
{
    int swapInterval; // VSync
} gsk_GraphicsSettings;

// Time

typedef struct gsk_Time
{
    f64 delta_time;       // variable delta_time time
    f64 fixed_delta_time; // fixed delta_time time interval
    f64 next_time_scale, time_scale;

    struct
    {
        f64 last_fps;
        f64 last_ms;
    } metrics;

} gsk_Time;

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

gsk_Time
gsk_device_getTime();

void
gsk_device_resetTime();

void
gsk_device_setTimescale(f32 timescale);

void
gsk_device_updateTime(double time);

// fixed delta-time clock handlers //

u8
_gsk_device_check_fixed_update(double time);

void
_gsk_device_reset_fixed_update(double time);

// input //

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
