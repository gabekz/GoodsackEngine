/*
 * Copyright (c) 2022-2023, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

// TODO: Implement

#ifndef __BUFFERS_H__
#define __BUFFERS_H__

#include "util/gfx.h"
#include "util/sysdefs.h"

enum BufferType = {VERTEX = 0, INDEX, UNIFORM};

struct GraphicsBuffer
{
    BufferType type;
    void *data;
    ui32 length;

    // VkBuffer bufferId;
    // ui32 bufferId;
};

GraphicsBuffer *
graphics_buffer_create(BufferType type, void *data, ui32 size);

#endif // __BUFFERS_H__