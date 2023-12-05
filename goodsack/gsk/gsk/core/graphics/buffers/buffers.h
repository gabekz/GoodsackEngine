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

struct gsk_GraphicsBuffer
{
    BufferType type;
    void *data;
    u32 length;

    // VkBuffer bufferId;
    // u32 bufferId;
};

gsk_GraphicsBuffer *
gsk_graphics_buffer_create(BufferType type, void *data, u32 size);

#endif // __BUFFERS_H__