/*
 * Copyright (c) 2022-2023, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#ifndef __OPENGL_DEBUG_H__
#define __OPENGL_DEBUG_H__

#include "util/gfx.h"

void APIENTRY
glDebugOutput(GLenum source,
              GLenum type,
              unsigned int id,
              GLenum severity,
              GLsizei length,
              const char *message,
              const void *userParam);

void
glDebugInit();
// ~~~
void
_error_callback_gl(int error, const char *description);

void
GLClearError();
void
GLCheckError();

#endif // __OPENGL_DEBUG_H__
