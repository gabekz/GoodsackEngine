/*
 * Copyright (c) 2023, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#ifndef __DEBUG_DRAW_SKELETON_H__
#define __DEBUG_DRAW_SKELETON_H__

#include "core/graphics/mesh/mesh.h"
#include "tools/debug/debug_context.h"

void
debug_draw_skeleton(DebugContext *debugContext, gsk_Skeleton *skeleton);

#endif // __DEBUG_DRAW_SKELETON_H__