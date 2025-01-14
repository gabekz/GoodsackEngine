/*
 * Copyright (c) 2022-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#ifndef __PASS_SCREEN_H__
#define __PASS_SCREEN_H__

#include "core/graphics/renderer/renderer_props.inl"
#include "util/sysdefs.h"

void
postbuffer_init(u32 width, u32 height, gsk_RendererProps *properties);
void
postbuffer_bind(int enableMSAA);
void
postbuffer_draw(gsk_RendererProps *properties, u32 bloom_texture_id);
void
postbuffer_resize(u32 winWidth, u32 winHeight);
void
postbuffer_cleanup();
u32
postbuffer_get_id();

#endif // __PASS_SCREEN_H__