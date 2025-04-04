/*
 * Copyright (c) 2023-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#ifndef __DEBUG_DRAW_SKELETON_H__
#define __DEBUG_DRAW_SKELETON_H__

#include "core/graphics/mesh/mesh.h"
#include "tools/debug/debug_context.h"

#include "util/maths.h"

void
gsk_debug_draw_skeleton(gsk_DebugContext *debugContext,
                        gsk_Skeleton *skeleton,
                        mat4 model_matrix);

#endif // __DEBUG_DRAW_SKELETON_H__