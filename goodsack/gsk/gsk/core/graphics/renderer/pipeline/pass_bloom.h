/*
 * Copyright (c) 2025-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#ifndef __PASS_BLOOM_H__
#define __PASS_BLOOM_H__

#include "core/graphics/renderer/renderer_props.inl"
#include "util/sysdefs.h"

void
pass_bloom_init();

void
pass_bloom_render(u32 tex_source_id, gsk_RendererProps *properties);

u32
pass_bloom_get_texture_id();

#endif // __PASS_BLOOM_H__