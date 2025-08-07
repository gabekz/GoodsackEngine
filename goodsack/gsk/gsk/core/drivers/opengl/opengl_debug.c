/*
 * Copyright (c) 2022-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

// Debugging APIENTRY

#include "opengl_debug.h"

#include <stdio.h>
#include <stdlib.h>

#include "util/gfx.h"
#include "util/logger.h"
#include "util/sysdefs.h"

#define _BREAK_ON_MSG FALSE

void APIENTRY
_gsk_gl_debug_output(GLenum source,
                     GLenum type,
                     unsigned int id,
                     GLenum severity,
                     GLsizei length,
                     const char *message,
                     const void *userParam)
{
    // ignore non-significant error/warning codes
    if (id == 131169 || id == 131185 || id == 131218 || id == 131204) return;

    // ignore debug markers
    if (type == GL_DEBUG_TYPE_PUSH_GROUP || type == GL_DEBUG_TYPE_POP_GROUP)
        return;

    LOG_WARN("------------\nOpenGL message: %s", message);

    switch (source)
    {
    case GL_DEBUG_SOURCE_API: LOG_PRINT("Source: API"); break;
    case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
        LOG_PRINT("Source: Window System");
        break;
    case GL_DEBUG_SOURCE_SHADER_COMPILER:
        LOG_PRINT("Source: Shader Compiler");
        break;
    case GL_DEBUG_SOURCE_THIRD_PARTY: LOG_PRINT("Source: Third Party"); break;
    case GL_DEBUG_SOURCE_APPLICATION: LOG_PRINT("Source: Application"); break;
    case GL_DEBUG_SOURCE_OTHER: LOG_PRINT("Source: Other"); break;
    }

    switch (type)
    {
    case GL_DEBUG_TYPE_ERROR: LOG_PRINT("Type: Error"); break;
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
        LOG_PRINT("Type: Deprecated Behaviour");
        break;
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
        LOG_PRINT("Type: Undefined Behaviour");
        break;
    case GL_DEBUG_TYPE_PORTABILITY: LOG_PRINT("Type: Portability"); break;
    case GL_DEBUG_TYPE_PERFORMANCE: LOG_PRINT("Type: Performance"); break;
    case GL_DEBUG_TYPE_MARKER: LOG_PRINT("Type: Marker"); break;
    case GL_DEBUG_TYPE_PUSH_GROUP: LOG_PRINT("Type: Push Group"); break;
    case GL_DEBUG_TYPE_POP_GROUP: LOG_PRINT("Type: Pop Group"); break;
    case GL_DEBUG_TYPE_OTHER: LOG_PRINT("Type: Other"); break;
    }

    switch (severity)
    {
    case GL_DEBUG_SEVERITY_HIGH: LOG_PRINT("Severity: high"); break;
    case GL_DEBUG_SEVERITY_MEDIUM: LOG_PRINT("Severity: medium"); break;
    case GL_DEBUG_SEVERITY_LOW: LOG_PRINT("Severity: low"); break;
    case GL_DEBUG_SEVERITY_NOTIFICATION:
        LOG_PRINT("Severity: notification");
        break;
    }

    LOG_PRINT("\n");

#if _BREAK_ON_MSG
    _BRK();
#endif // _BREAK_ON_MSG
}

void
_gsk_gl_debug_init()
{
    int flags;
    glGetIntegerv(GL_CONTEXT_FLAGS, &flags);

    if (flags & GL_CONTEXT_FLAG_DEBUG_BIT)
    {
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(_gsk_gl_debug_output, NULL);
        glDebugMessageControl(
          GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_TRUE);
        glDebugMessageControl(
          GL_DONT_CARE, GL_DEBUG_TYPE_ERROR, GL_DONT_CARE, 0, NULL, GL_TRUE);
    }
}
// ~~~
void
_gsk_gl_error_callback(int error, const char *description)
{
    LOG_ERROR("%s", description);
    _gsk_gl_error_clear();
}

void
_gsk_gl_error_clear()
{
    while (glGetError() != GL_NO_ERROR)
        ;
}

void
_gsk_gl_error_check()
{
    GLenum error = glGetError();
    while (error)
    {
        LOG_ERROR("|OpenGL Error| (%s)", error);
    }
}
