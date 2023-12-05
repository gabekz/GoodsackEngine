/*
 * Copyright (c) 2022-2023, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#ifndef __CAMERA_H__
#define __CAMERA_H__

#include <util/gfx.h>
#include <util/maths.h>
#include <util/sysdefs.h>

#include <entity/v1/ecs.h>

#if !(USING_GENERATED_COMPONENTS)
struct ComponentCamera
{
    vec3 axisUp, center,
      front; // TODO: Currently used for Audio Listener orientation.
    float fov;
    float speed, sensitivity;

    float lastX, lastY;
    float yaw, pitch;

    si16 firstMouse; // bool

    // OpenGL
    ui32 uboId;
    //

    // Vulkan
    VkBuffer *uniformBuffer;
    VkDeviceMemory *uniformBufferMemory;
    void **uniformBufferMapped;
    //

    struct
    {
        float nearZ, farZ;
    } clipping;
    struct
    {
        int width, height;
    } screen;

    struct
    {
        // vec3 position; -- TODO: Should be part of UNIFORM structure
        mat4 model, view, proj;
    } uniform;
};
#endif

void
s_camera_init(gsk_ECS *ecs);

#endif // __CAMERA_H__