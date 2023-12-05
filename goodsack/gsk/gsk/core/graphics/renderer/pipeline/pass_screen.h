/*
 * Copyright (c) 2022-2023, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#ifndef __PASS_SCREEN_H__
#define __PASS_SCREEN_H__

#include "util/sysdefs.h"
#include "core/graphics/renderer/renderer_props.inl"

void
postbuffer_init(ui32 width, ui32 height, gsk_RendererProps *properties);
void
postbuffer_bind(int enableMSAA);
void
postbuffer_draw(gsk_RendererProps *properties);
void
postbuffer_resize(ui32 winWidth, ui32 winHeight);
void
postbuffer_cleanup();

#endif // __PASS_SCREEN_H__