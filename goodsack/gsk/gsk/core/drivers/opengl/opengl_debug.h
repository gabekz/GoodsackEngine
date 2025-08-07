/*
 * Copyright (c) 2022-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#ifndef __OPENGL_DEBUG_H__
#define __OPENGL_DEBUG_H__

#include "util/gfx.h"

void APIENTRY
_gsk_gl_debug_output(GLenum source,
                     GLenum type,
                     unsigned int id,
                     GLenum severity,
                     GLsizei length,
                     const char *message,
                     const void *userParam);

void
_gsk_gl_debug_init();

void
_gsk_gl_error_callback(int error, const char *description);

void
_gsk_gl_error_clear();

void
_gsk_gl_error_check();

#endif // __OPENGL_DEBUG_H__
