/*
 * Copyright (c) 2023, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#ifndef __DEBUG_DRAW_LINE_H__
#define __DEBUG_DRAW_LINE_H__

#include "tools/debug/debug_context.h"
#include "util/maths.h"

void
debug_draw_line(DebugContext *debugContext, vec3 start, vec3 end);

#endif // __DEBUG_DRAW_LINE_H__
